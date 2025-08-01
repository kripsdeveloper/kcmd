#include <kcmd>

KCMD("test") {
    SendClientMessage(playerid, -1, "Test Komutu");
}

KCMD("goto", 1) {
    new targetid;
    if (sscanf(params, "u", targetid)) return SendClientMessage(playerid, -1, "/goto [user/id]");
    if (!IsPlayerConnected(targetid) || targetid == playerid)
    return SendClientMessage(playerid, -1, "boyle biri yok");
    new Float:x, Float:y, Float:z;
    GetPlayerPos(targetid, x, y, z);
    SetPlayerPos(playerid, x + 1.0, y, z); 
    SendClientMessage(playerid, -1, "goto basarili");
}


KCMD("kick", 1, 2) {
    new targetid, reason[64];
    if (sscanf(params, "us[64]", targetid, reason)) {
        if (sscanf(params, "u", targetid)) return SendClientMessage(playerid, -1, "/kick [user/id] [sebep]");
        format(reason, sizeof(reason), "sebep belirtmeden devam ediyorsun (parametre 2 gecersiz)");
    }
    if (!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, -1, "boyle biri yok");

    new name[MAX_PLAYER_NAME];
    GetPlayerName(targetid, name, sizeof(name));
    new msg[128];
    format(msg, sizeof(msg), "%s (%d) sunucudan atıldı ve sebep belirtilmis, sebep: %s", name, targetid, reason);
    SendClientMessageToAll(-1, msg);
    Kick(targetid);
}
