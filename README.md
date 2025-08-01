
# Krips-CMD

## ENGLISH

### ✨ Overview
`kcmd` is a **modern**, **minimal**, and **developer-friendly** command include for SA-MP (San Andreas Multiplayer). It is designed to reduce boilerplate, improve code clarity, and allow flexible command definitions with automatic parameter validation.

Whether you're building a lightweight gamemode or a complex roleplay script, `kcmd` makes command registration much simpler and neater.

### 📦 Features
- ✅ Clean and minimal syntax
- ✅ Automatic parameter validation
- ✅ Supports variable argument ranges
- ✅ No callbacks or boilerplate macros
- ✅ Inspired by modern scripting practices

### ⚙️ Installation

1. Download `kcmd.inc` and place it in your `pawno/include` directory.
2. Include it in your script:
   ```pawn
   #include <kcmd>
   ```

### 🧠 Syntax

```pawn
KCMD("command_name") { ... }
KCMD("command_name", required_param_count) { ... }
KCMD("command_name", min_param_count, max_param_count) { ... }
```

### 🧪 Notes
- `playerid` and `params[]` are automatically available in all KCMD blocks.
- Incorrect usage (wrong parameter count) is blocked before your logic runs.
- Ideal for both small utilities and large-scale server scripts.

### 📄 License
This project is licensed under the MIT License. Feel free to use, modify, and distribute it with attribution.

---

## TÜRKÇE

### ✨ Genel Bakış
`kcmd`, SA-MP (San Andreas Multiplayer) için geliştirilmiş **modern**, **minimal**, ve **geliştirici dostu** bir komut sistemidir. Kod tekrarlamasını azaltır, okunabilirliği artırır ve parametre doğrulamasını sizin yerinize otomatik olarak yapar.

Hafif bir moddan, karmaşık RP sistemlerine kadar tüm yapılar için ideal bir çözümdür.

### 📦 Özellikler
- ✅ Temiz ve sade sözdizimi
- ✅ Otomatik parametre kontrolü
- ✅ Değişken parametre sayısı desteği
- ✅ Geri dönüş veya makro karmaşası yok
- ✅ Modern yazılım geliştirme anlayışıyla yazılmıştır

### ⚙️ Kurulum

1. `kcmd.inc` dosyasını indirip `pawno/include` klasörüne atın.
2. Scriptinize dahil edin:
   ```pawn
   #include <kcmd>
   ```

### 🧠 Kullanım Biçimi

```pawn
KCMD("komut_adı") { ... }
KCMD("komut_adı", gerekli_parametre_sayısı) { ... }
KCMD("komut_adı", minimum, maksimum) { ... }
```

### 🧪 Notlar
- `playerid` ve `params[]` her KCMD bloğunda otomatik olarak kullanılabilir.
- Parametre eksikliği veya fazlalığı KCMD tarafından otomatik engellenir.
- Hem küçük scriptler hem büyük sunucu sistemleri için uygundur.


### 📄 Lisans
Bu proje MIT lisansı ile sunulmuştur. Özgürce kullanabilir, düzenleyebilir ve paylaşabilirsiniz.
