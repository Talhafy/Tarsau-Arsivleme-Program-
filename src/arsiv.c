#include "tarsau.h"

/* ── Yardımcı: dosyanın metin (ASCII) dosyası olup olmadığını kontrol et ── */
static int is_text_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    int c;
    while ((c = fgetc(f)) != EOF) {
        /* ASCII metin: 0-127 arası, kontrol karakterleri hariç
         * (tab, newline, carriage return izinli) */
        if (c > 127) { fclose(f); return 0; }
        if (c < 32 && c != '\t' && c != '\n' && c != '\r') {
            fclose(f); return 0;
        }
    }
    fclose(f);
    return 1;
}

/* ────────────────────────────────────────────────────────────────────────
 * -b modu: arşiv oluştur
 * ──────────────────────────────────────────────────────────────────────── */
int build_archive(int file_count, char *files[], const char *archive_name) {

    if (file_count == 0) {
        fprintf(stderr, "Hata: en az bir giriş dosyası belirtilmelidir.\n");
        return 1;
    }
    if (file_count > MAX_FILES) {
        fprintf(stderr, "Hata: giriş dosyası sayısı en fazla %d olabilir.\n", MAX_FILES);
        return 1;
    }

    /* Dosyaları doğrula */
    long long total = 0;
    for (int i = 0; i < file_count; i++) {
        if (!is_text_file(files[i])) {
            fprintf(stderr, "%s giriş dosyasının formatı uyumsuzdur!\n", files[i]);
            return 1;
        }

        struct stat st;
        if (stat(files[i], &st) != 0) {
            fprintf(stderr, "Hata: %s dosyası okunamadı.\n", files[i]);
            return 1;
        }
        
        total += (long long)st.st_size;
        if (total > MAX_TOTAL_BYTES) {
            fprintf(stderr, "Hata: giriş dosyalarının toplam boyutu 200 MB'ı geçemez.\n");
            return 1;
        }

        const char *base = strrchr(files[i], '/');
        base = base ? base + 1 : files[i];
        if (strlen(base) >= NAME_FIELD_SIZE) {
            fprintf(stderr, "Hata: '%s' dosya adı çok uzun (max %d karakter).\n",
                    base, NAME_FIELD_SIZE - 1);
            return 1;
        }
    }

    /* Arşivi yaz */
    FILE *out = fopen(archive_name, "wb");
    if (!out) {
        perror(archive_name);
        return 1;
    }

    /* Başlık: dosya sayısı */
    fprintf(out, "%*d", COUNT_FIELD_SIZE, file_count);

    for (int i = 0; i < file_count; i++) {
        const char *base = strrchr(files[i], '/');
        base = base ? base + 1 : files[i];

        struct stat st;
        stat(files[i], &st);
        long long sz = (long long)st.st_size;

        /* Dosya adı alanı */
        char name_buf[NAME_FIELD_SIZE];
        memset(name_buf, 0, NAME_FIELD_SIZE);
        strncpy(name_buf, base, NAME_FIELD_SIZE - 1);
        fwrite(name_buf, 1, NAME_FIELD_SIZE, out);

        /* Boyut alanı: SIZE_FIELD_SIZE bayt ASCII */
        fprintf(out, "%*lld", SIZE_FIELD_SIZE, sz);
        
        /* İzin alanı (Mode): 8 bayt octal format */
        fprintf(out, "%0*o", MODE_FIELD_SIZE, st.st_mode & 07777);

        /* Dosya içeriği */
        FILE *in = fopen(files[i], "rb");
        if (!in) {
            perror(files[i]);
            fclose(out);
            remove(archive_name);
            return 1;
        }
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, n, out);
        fclose(in);
    }

    fclose(out);
    printf("Dosyalar birleştirildi.\n");
    return 0;
}