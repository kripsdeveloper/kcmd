/*
    kcmd.inc - Ultra Fast Command Processor
    Build: 3.2.1-stable
    Developer: KralCMD
    
    Zero setup required. Just use it.
    
    KCMD("heal") { 
        SetPlayerHealth(playerid, 100.0); 
        SendClientMessage(playerid, -1, "HP restored!"); 
    }
*/

#if defined _inc_kcmd
    #endinput
#endif
#define _inc_kcmd

#tryinclude <a_samp>

#if !defined KCMD_MAX_CMDS
    #define KCMD_MAX_CMDS 768
#endif

#if !defined KCMD_MAX_ALIAS
    #define KCMD_MAX_ALIAS 384
#endif

#if !defined KCMD_PARAM_LEN
    #define KCMD_PARAM_LEN 320
#endif

#if !defined KCMD_HASH_SIZE
    #define KCMD_HASH_SIZE 256
#endif

#if !defined KCMD_SECURITY_MODE
    #define KCMD_SECURITY_MODE 1
#endif

#if !defined KCMD_BENCHMARK_MODE
    #define KCMD_BENCHMARK_MODE 0
#endif

#if !defined KCMD_CACHE_OPTIMIZATION
    #define KCMD_CACHE_OPTIMIZATION 1
#endif

enum _:E_CMD_REGISTRY {
    cmd_identifier[48],
    cmd_callback[72],
    cmd_hash_value,
    cmd_exec_count,
    cmd_total_duration,
    cmd_access_flags,
    bool:cmd_enabled,
    cmd_min_args,
    cmd_max_args,
    cmd_permission_level,
    cmd_last_execution,
    Float:cmd_avg_time
}

enum _:E_ALIAS_REGISTRY {
    alias_name[48],
    alias_target[48],
    alias_hash_key,
    bool:alias_active,
    alias_usage_counter
}

enum _:E_SECURITY_PROFILE {
    sec_player_uid,
    sec_previous_cmd[48],
    sec_cmd_frequency,
    sec_timestamp,
    sec_violation_count,
    bool:sec_temporary_block,
    sec_warning_level,
    sec_total_commands
}

enum _:E_PERFORMANCE_METRICS {
    perf_global_executions,
    perf_cumulative_time,
    Float:perf_mean_duration,
    perf_last_benchmark,
    perf_peak_usage,
    perf_cache_hits,
    perf_cache_misses
}

static g_CommandRegistry[KCMD_MAX_CMDS][E_CMD_REGISTRY];
static g_AliasRegistry[KCMD_MAX_ALIAS][E_ALIAS_REGISTRY];
static g_SecurityProfiles[MAX_PLAYERS][E_SECURITY_PROFILE];
static g_PerformanceData[E_PERFORMANCE_METRICS];
static g_HashLookupTable[KCMD_HASH_SIZE][24];
static g_CommandIndex = 0;
static g_AliasIndex = 0;
static bool:g_SystemReady = false;
static g_MemoryBuffer[2048];
static g_BufferPointer = 0;

#define HASH_PRIME_A 16777619
#define HASH_PRIME_B 2166136261
#define HASH_MASK 0x7FFFFFFF

stock __KCMD_ComputeHash(const input[]) {
    new hash = HASH_PRIME_B;
    new length = strlen(input);
    for(new idx = 0; idx < length; idx++) {
        hash ^= tolower(input[idx]);
        hash *= HASH_PRIME_A;
    }
    return hash & HASH_MASK;
}

static g_TempBuffer[512];
static g_WorkingSet[128];

stock __KCMD_AllocateBuffer(required_size) {
    if(g_BufferPointer + required_size >= sizeof(g_MemoryBuffer)) {
        g_BufferPointer = 0;
    }
    new allocation = g_BufferPointer;
    g_BufferPointer += required_size;
    return allocation;
}

stock __KCMD_RegisterCommand(const name[], const func[], access = 0, min_params = 0, max_params = -1, perm_level = 0) {
    if(g_CommandIndex >= KCMD_MAX_CMDS) {
        return 0;
    }
    
    if(!g_SystemReady) {
        __KCMD_InitializeCore();
    }
    
    new hash_val = __KCMD_ComputeHash(name);
    
    strpack(g_CommandRegistry[g_CommandIndex][cmd_identifier], name, 48);
    strpack(g_CommandRegistry[g_CommandIndex][cmd_callback], func, 72);
    g_CommandRegistry[g_CommandIndex][cmd_hash_value] = hash_val;
    g_CommandRegistry[g_CommandIndex][cmd_exec_count] = 0;
    g_CommandRegistry[g_CommandIndex][cmd_total_duration] = 0;
    g_CommandRegistry[g_CommandIndex][cmd_access_flags] = access;
    g_CommandRegistry[g_CommandIndex][cmd_enabled] = true;
    g_CommandRegistry[g_CommandIndex][cmd_min_args] = min_params;
    g_CommandRegistry[g_CommandIndex][cmd_max_args] = max_params;
    g_CommandRegistry[g_CommandIndex][cmd_permission_level] = perm_level;
    g_CommandRegistry[g_CommandIndex][cmd_last_execution] = 0;
    g_CommandRegistry[g_CommandIndex][cmd_avg_time] = 0.0;
    
    new table_idx = hash_val % KCMD_HASH_SIZE;
    new slot_idx = 0;
    while(slot_idx < 24 && g_HashLookupTable[table_idx][slot_idx] != 0) {
        slot_idx++;
    }
    if(slot_idx < 24) {
        g_HashLookupTable[table_idx][slot_idx] = g_CommandIndex + 1;
    }
    
    g_CommandIndex++;
    return 1;
}

stock __KCMD_InitializeCore() {
    if(g_SystemReady) return;
    
    for(new i = 0; i < KCMD_HASH_SIZE; i++) {
        for(new j = 0; j < 24; j++) {
            g_HashLookupTable[i][j] = 0;
        }
    }
    
    for(new playerid = 0; playerid < MAX_PLAYERS; playerid++) {
        g_SecurityProfiles[playerid][sec_player_uid] = playerid;
        g_SecurityProfiles[playerid][sec_previous_cmd][0] = 0;
        g_SecurityProfiles[playerid][sec_cmd_frequency] = 0;
        g_SecurityProfiles[playerid][sec_timestamp] = 0;
        g_SecurityProfiles[playerid][sec_violation_count] = 0;
        g_SecurityProfiles[playerid][sec_temporary_block] = false;
        g_SecurityProfiles[playerid][sec_warning_level] = 0;
        g_SecurityProfiles[playerid][sec_total_commands] = 0;
    }
    
    g_PerformanceData[perf_global_executions] = 0;
    g_PerformanceData[perf_cumulative_time] = 0;
    g_PerformanceData[perf_mean_duration] = 0.0;
    g_PerformanceData[perf_last_benchmark] = GetTickCount();
    g_PerformanceData[perf_peak_usage] = 0;
    g_PerformanceData[perf_cache_hits] = 0;
    g_PerformanceData[perf_cache_misses] = 0;
    
    g_SystemReady = true;
}

stock __KCMD_ProcessParameters(const source[], destination[], max_length = sizeof(destination)) {
    new src_len = strlen(source);
    new dest_pos = 0;
    new quote_mode = 0;
    new bracket_depth = 0;
    
    for(new i = 0; i < src_len && dest_pos < max_length - 1; i++) {
        switch(source[i]) {
            case '"', '\'': {
                quote_mode = !quote_mode;
                continue;
            }
            case '[': {
                if(!quote_mode) bracket_depth++;
            }
            case ']': {
                if(!quote_mode && bracket_depth > 0) bracket_depth--;
            }
            case ' ', '\t': {
                if(!quote_mode && bracket_depth == 0) {
                    if(dest_pos > 0 && destination[dest_pos - 1] != ' ') {
                        destination[dest_pos++] = ' ';
                    }
                    continue;
                }
            }
        }
        destination[dest_pos++] = source[i];
    }
    
    destination[dest_pos] = 0;
    
    while(dest_pos > 0 && destination[dest_pos - 1] == ' ') {
        destination[--dest_pos] = 0;
    }
    
    return dest_pos;
}

stock bool:__KCMD_ValidateSecurity(playerid, const command_name[]) {
    #if KCMD_SECURITY_MODE == 0
        return true;
    #endif
    
    new current_tick = GetTickCount();
    
    if(g_SecurityProfiles[playerid][sec_temporary_block]) {
        if(current_tick - g_SecurityProfiles[playerid][sec_timestamp] > 15000) {
            g_SecurityProfiles[playerid][sec_temporary_block] = false;
            g_SecurityProfiles[playerid][sec_violation_count] = 0;
            g_SecurityProfiles[playerid][sec_warning_level] = 0;
        } else {
            return false;
        }
    }
    
    new previous_cmd[48];
    strunpack(previous_cmd, g_SecurityProfiles[playerid][sec_previous_cmd]);
    
    if(!strcmp(previous_cmd, command_name, true)) {
        if(current_tick - g_SecurityProfiles[playerid][sec_timestamp] < 300) {
            g_SecurityProfiles[playerid][sec_violation_count]++;
            if(g_SecurityProfiles[playerid][sec_violation_count] >= 8) {
                g_SecurityProfiles[playerid][sec_temporary_block] = true;
                g_SecurityProfiles[playerid][sec_warning_level] = 3;
                SendClientMessage(playerid, 0xFF3333FF, "Anti-spam triggered. Cooldown: 15 seconds.");
                return false;
            } else if(g_SecurityProfiles[playerid][sec_violation_count] >= 4) {
                g_SecurityProfiles[playerid][sec_warning_level] = 2;
                SendClientMessage(playerid, 0xFFAA33FF, "Slow down with commands!");
            }
        }
    } else {
        if(current_tick - g_SecurityProfiles[playerid][sec_timestamp] > 1000) {
            g_SecurityProfiles[playerid][sec_violation_count] = 0;
            g_SecurityProfiles[playerid][sec_warning_level] = 0;
        }
    }
    
    strpack(g_SecurityProfiles[playerid][sec_previous_cmd], command_name, 48);
    g_SecurityProfiles[playerid][sec_timestamp] = current_tick;
    g_SecurityProfiles[playerid][sec_total_commands]++;
    
    return true;
}

stock __KCMD_LookupCommand(const name[]) {
    new hash_val = __KCMD_ComputeHash(name);
    new table_idx = hash_val % KCMD_HASH_SIZE;
    
    for(new slot = 0; slot < 24; slot++) {
        new cmd_idx = g_HashLookupTable[table_idx][slot] - 1;
        if(cmd_idx < 0) break;
        
        if(g_CommandRegistry[cmd_idx][cmd_hash_value] == hash_val && g_CommandRegistry[cmd_idx][cmd_enabled]) {
            new stored_name[48];
            strunpack(stored_name, g_CommandRegistry[cmd_idx][cmd_identifier]);
            if(!strcmp(stored_name, name, true)) {
                #if KCMD_CACHE_OPTIMIZATION
                    g_PerformanceData[perf_cache_hits]++;
                #endif
                return cmd_idx;
            }
        }
    }
    
    #if KCMD_CACHE_OPTIMIZATION
        g_PerformanceData[perf_cache_misses]++;
    #endif
    return -1;
}

stock __KCMD_CreateAlias(const alias[], const target[]) {
    if(g_AliasIndex >= KCMD_MAX_ALIAS) return 0;
    
    strpack(g_AliasRegistry[g_AliasIndex][alias_name], alias, 48);
    strpack(g_AliasRegistry[g_AliasIndex][alias_target], target, 48);
    g_AliasRegistry[g_AliasIndex][alias_hash_key] = __KCMD_ComputeHash(alias);
    g_AliasRegistry[g_AliasIndex][alias_active] = true;
    g_AliasRegistry[g_AliasIndex][alias_usage_counter] = 0;
    
    g_AliasIndex++;
    return 1;
}

stock __KCMD_ResolveAlias(const input[], output[], max_len = sizeof(output)) {
    new input_hash = __KCMD_ComputeHash(input);
    
    for(new i = 0; i < g_AliasIndex; i++) {
        if(g_AliasRegistry[i][alias_hash_key] == input_hash && g_AliasRegistry[i][alias_active]) {
            new stored_alias[48];
            strunpack(stored_alias, g_AliasRegistry[i][alias_name]);
            if(!strcmp(stored_alias, input, true)) {
                strunpack(output, g_AliasRegistry[i][alias_target], max_len);
                g_AliasRegistry[i][alias_usage_counter]++;
                return 1;
            }
        }
    }
    
    format(output, max_len, "%s", input);
    return 0;
}

stock __KCMD_UpdateBenchmark(cmd_idx, exec_time) {
    #if !KCMD_BENCHMARK_MODE
        return;
    #endif
    
    g_CommandRegistry[cmd_idx][cmd_exec_count]++;
    g_CommandRegistry[cmd_idx][cmd_total_duration] += exec_time;
    g_CommandRegistry[cmd_idx][cmd_last_execution] = GetTickCount();
    
    if(g_CommandRegistry[cmd_idx][cmd_exec_count] > 0) {
        g_CommandRegistry[cmd_idx][cmd_avg_time] = 
            Float:g_CommandRegistry[cmd_idx][cmd_total_duration] / Float:g_CommandRegistry[cmd_idx][cmd_exec_count];
    }
    
    g_PerformanceData[perf_global_executions]++;
    g_PerformanceData[perf_cumulative_time] += exec_time;
    
    if(g_PerformanceData[perf_global_executions] > 0) {
        g_PerformanceData[perf_mean_duration] = 
            Float:g_PerformanceData[perf_cumulative_time] / Float:g_PerformanceData[perf_global_executions];
    }
    
    if(exec_time > g_PerformanceData[perf_peak_usage]) {
        g_PerformanceData[perf_peak_usage] = exec_time;
    }
}

public OnPlayerCommandText(playerid, cmdtext[]) {
    if(!g_SystemReady) {
        __KCMD_InitializeCore();
    }
    
    #if KCMD_BENCHMARK_MODE
        new benchmark_start = GetTickCount();
    #endif
    
    new command_name[48], parameters[KCMD_PARAM_LEN];
    new space_pos = strfind(cmdtext, " ");
    
    if(space_pos == -1) {
        strmid(command_name, cmdtext, 1, strlen(cmdtext));
        parameters[0] = 0;
    } else {
        strmid(command_name, cmdtext, 1, space_pos);
        strmid(parameters, cmdtext, space_pos + 1, strlen(cmdtext));
        __KCMD_ProcessParameters(parameters, parameters);
    }
    
    for(new i = 0; i < strlen(command_name); i++) {
        command_name[i] = tolower(command_name[i]);
    }
    
    new resolved_command[48];
    __KCMD_ResolveAlias(command_name, resolved_command);
    
    if(!__KCMD_ValidateSecurity(playerid, resolved_command)) {
        return 1;
    }
    
    new cmd_idx = __KCMD_LookupCommand(resolved_command);
    if(cmd_idx == -1) {
        #if defined KCMD_OnPlayerCommandText
            return KCMD_OnPlayerCommandText(playerid, cmdtext);
        #else
            return 0;
        #endif
    }
    
    if(g_CommandRegistry[cmd_idx][cmd_min_args] > 0) {
        new arg_count = 0;
        new temp_params[KCMD_PARAM_LEN];
        format(temp_params, sizeof(temp_params), "%s", parameters);
        
        if(strlen(temp_params) > 0) {
            arg_count = 1;
            for(new i = 0; i < strlen(temp_params); i++) {
                if(temp_params[i] == ' ') arg_count++;
            }
        }
        
        if(arg_count < g_CommandRegistry[cmd_idx][cmd_min_args]) {
            format(g_TempBuffer, sizeof(g_TempBuffer), "Minimum %d arguments required.", g_CommandRegistry[cmd_idx][cmd_min_args]);
            SendClientMessage(playerid, 0xFF6666FF, g_TempBuffer);
            return 1;
        }
        
        if(g_CommandRegistry[cmd_idx][cmd_max_args] > 0 && arg_count > g_CommandRegistry[cmd_idx][cmd_max_args]) {
            format(g_TempBuffer, sizeof(g_TempBuffer), "Maximum %d arguments allowed.", g_CommandRegistry[cmd_idx][cmd_max_args]);
            SendClientMessage(playerid, 0xFF6666FF, g_TempBuffer);
            return 1;
        }
    }
    
    new callback_name[72];
    strunpack(callback_name, g_CommandRegistry[cmd_idx][cmd_callback]);
    CallLocalFunction(callback_name, "is", playerid, parameters);
    
    #if KCMD_BENCHMARK_MODE
        new execution_duration = GetTickCount() - benchmark_start;
        __KCMD_UpdateBenchmark(cmd_idx, execution_duration);
    #endif
    
    return 1;
}

#if defined _ALS_OnPlayerCommandText
    #undef OnPlayerCommandText
#else
    #define _ALS_OnPlayerCommandText
#endif
#define OnPlayerCommandText KCMD_OnPlayerCommandText

#if defined KCMD_OnPlayerCommandText
    forward KCMD_OnPlayerCommandText(playerid, cmdtext[]);
#endif

public OnPlayerConnect(playerid) {
    g_SecurityProfiles[playerid][sec_player_uid] = playerid;
    g_SecurityProfiles[playerid][sec_previous_cmd][0] = 0;
    g_SecurityProfiles[playerid][sec_cmd_frequency] = 0;
    g_SecurityProfiles[playerid][sec_timestamp] = 0;
    g_SecurityProfiles[playerid][sec_violation_count] = 0;
    g_SecurityProfiles[playerid][sec_temporary_block] = false;
    g_SecurityProfiles[playerid][sec_warning_level] = 0;
    g_SecurityProfiles[playerid][sec_total_commands] = 0;
    
    #if defined KCMD_OnPlayerConnect
        return KCMD_OnPlayerConnect(playerid);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect KCMD_OnPlayerConnect

#if defined KCMD_OnPlayerConnect
    forward KCMD_OnPlayerConnect(playerid);
#endif

#define KCMD(%0,%1,%2) \
    __KCMD_VARIANT_%1_%2(%0,%1,%2)

#define __KCMD_VARIANT_(%0,%1,%2) \
    forward kcmd_%0(playerid, params[]); \
    public kcmd_%0(playerid, params[]) \
    __KCMD_RegisterCommand(#%0, "kcmd_"#%0)

#define __KCMD_VARIANT_1_(%0,%1,%2) \
    forward kcmd_%0(playerid, params[]); \
    public kcmd_%0(playerid, params[]) \
    __KCMD_RegisterCommand(#%0, "kcmd_"#%0, 0, %1, %1)

#define __KCMD_VARIANT_1_2(%0,%1,%2) \
    forward kcmd_%0(playerid, params[]); \
    public kcmd_%0(playerid, params[]) \
    __KCMD_RegisterCommand(#%0, "kcmd_"#%0, 0, %1, %2)

#define KCMD_PARAMS(%0,%1) \
    forward kcmd_%0(playerid, params[]); \
    public kcmd_%0(playerid, params[]) \
    __KCMD_RegisterCommand(#%0, "kcmd_"#%0, 0, %1, %1)

#define KCMD_PARAMS_RANGE(%0,%1,%2) \
    forward kcmd_%0(playerid, params[]); \
    public kcmd_%0(playerid, params[]) \
    __KCMD_RegisterCommand(#%0, "kcmd_"#%0, 0, %1, %2)

stock KCMD_GetCommandCount() {
    return g_CommandIndex;
}

stock KCMD_GetCommandName(idx, name[], max_len = sizeof(name)) {
    if(idx < 0 || idx >= g_CommandIndex) return 0;
    strunpack(name, g_CommandRegistry[idx][cmd_identifier], max_len);
    return 1;
}

stock KCMD_GetCommandStats(idx, &executions, &total_time) {
    if(idx < 0 || idx >= g_CommandIndex) return 0;
    executions = g_CommandRegistry[idx][cmd_exec_count];
    total_time = g_CommandRegistry[idx][cmd_total_duration];
    return 1;
}

stock KCMD_ToggleCommand(const name[], bool:enable) {
    new idx = __KCMD_LookupCommand(name);
    if(idx == -1) return 0;
    g_CommandRegistry[idx][cmd_enabled] = enable;
    return 1;
}

stock KCMD_CreateAlias(const alias[], const target[]) {
    return __KCMD_CreateAlias(alias, target);
}

#if KCMD_BENCHMARK_MODE
stock KCMD_ShowStats() {
    printf("=== KCMD Performance Report ===");
    printf("Total Executions: %d", g_PerformanceData[perf_global_executions]);
    printf("Total Time: %dms", g_PerformanceData[perf_cumulative_time]);
    printf("Average Time: %.2fms", g_PerformanceData[perf_mean_duration]);
    printf("Peak Usage: %dms", g_PerformanceData[perf_peak_usage]);
    printf("Commands Registered: %d", g_CommandIndex);
    printf("Active Aliases: %d", g_AliasIndex);
    printf("Cache Hits: %d", g_PerformanceData[perf_cache_hits]);
    printf("Cache Misses: %d", g_PerformanceData[perf_cache_misses]);
    
    new sorted_indices[KCMD_MAX_CMDS];
    for(new i = 0; i < g_CommandIndex; i++) {
        sorted_indices[i] = i;
    }
    
    for(new i = 0; i < g_CommandIndex - 1; i++) {
        for(new j = 0; j < g_CommandIndex - i - 1; j++) {
            if(g_CommandRegistry[sorted_indices[j]][cmd_exec_count] < g_CommandRegistry[sorted_indices[j+1]][cmd_exec_count]) {
                new temp = sorted_indices[j];
                sorted_indices[j] = sorted_indices[j+1];
                sorted_indices[j+1] = temp;
            }
        }
    }
    
    printf("\n=== Top Commands ===");
    new display_count = (g_CommandIndex > 15) ? 15 : g_CommandIndex;
    for(new i = 0; i < display_count; i++) {
        new idx = sorted_indices[i];
        if(g_CommandRegistry[idx][cmd_exec_count] > 0) {
            new cmd_name[48];
            strunpack(cmd_name, g_CommandRegistry[idx][cmd_identifier]);
            printf("%d. %s - Calls: %d, Avg: %.2fms", 
                   i+1, cmd_name, 
                   g_CommandRegistry[idx][cmd_exec_count], 
                   g_CommandRegistry[idx][cmd_avg_time]);
        }
    }
    printf("===============================");
}
#endif

stock KCMD_ResetStats() {
    for(new i = 0; i < g_CommandIndex; i++) {
        g_CommandRegistry[i][cmd_exec_count] = 0;
        g_CommandRegistry[i][cmd_total_duration] = 0;
        g_CommandRegistry[i][cmd_avg_time] = 0.0;
    }
    
    g_PerformanceData[perf_global_executions] = 0;
    g_PerformanceData[perf_cumulative_time] = 0;
    g_PerformanceData[perf_mean_duration] = 0.0;
    g_PerformanceData[perf_peak_usage] = 0;
    g_PerformanceData[perf_cache_hits] = 0;
    g_PerformanceData[perf_cache_misses] = 0;
}

stock KCMD_Cleanup() {
    g_CommandIndex = 0;
    g_AliasIndex = 0;
    g_BufferPointer = 0;
    g_SystemReady = false;
    
    for(new i = 0; i < KCMD_MAX_CMDS; i++) {
        g_CommandRegistry[i][cmd_identifier][0] = 0;
        g_CommandRegistry[i][cmd_callback][0] = 0;
        g_CommandRegistry[i][cmd_hash_value] = 0;
        g_CommandRegistry[i][cmd_enabled] = false;
    }
    
    for(new i = 0; i < KCMD_HASH_SIZE; i++) {
        for(new j = 0; j < 24; j++) {
            g_HashLookupTable[i][j] = 0;
        }
    }
}