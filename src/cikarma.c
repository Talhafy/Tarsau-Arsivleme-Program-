#include "tarsau.h"

/* ── Yardımcı: dizin oluştur (yoksa) ── */
static int mkdir_if_needed(const char *dir) {
    if (dir == NULL || strcmp(dir, ".") == 0 || strcmp(dir, "") == 0)
        return 0;
    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
        perror(dir);
        return -1;
    }
    return 0;
}

/* ────────────────────────────────────────────────────────────────────────
 * -a modu: arşivi aç
 * ──────────────────────────────────────────────────────────────────────── */
int extract_archive(const char *archive_name, const char *dest_dir) {

    /* KURAL: .sau uzantısı kontrolü */
    int len = strlen(archive_name);
    if (len < 4 || strcmp(archive_name + len - 4, ".sau") != 0) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }

    FILE *in = fopen(archive_name, "rb");
    if (!in) {
        /* KURAL: Dosya açılamazsa / bozuksa istenen hata mesajı */
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }

    /* Başlık: dosya sayısı */
    char count_buf[COUNT_FIELD_SIZE + 1];
    if (fread(count_buf, 1, COUNT_FIELD_SIZE, in) != COUNT_FIELD_SIZE) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(in);
        return 1;
    }
    count_buf[COUNT_FIELD_SIZE] = '\0';
    int file_count = atoi(count_buf);
    
    if (file_count <= 0 || file_count > MAX_FILES) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(in);
        return 1;
    }

    /* KURAL: Hedef dizini oluştur (Boşluk/klasör yoksa oluşturur) */
    if (dest_dir && strcmp(dest_dir, ".") != 0) {
        if (mkdir_if_needed(dest_dir) != 0) {
            fclose(in);
            return 1; /* mkdir_if_needed kendi hatasını basıyor */
        }
    }

    /* Her dosyayı çıkar */
    char extracted_names[MAX_FILES][NAME_FIELD_SIZE];

    for (int i = 0; i < file_count; i++) {
        /* Dosya adı */
        char name_buf[NAME_FIELD_SIZE];
        if (fread(name_buf, 1, NAME_FIELD_SIZE, in) != NAME_FIELD_SIZE) {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            fclose(in); return 1;
        }
        name_buf[NAME_FIELD_SIZE - 1] = '\0';

        /* Boyut */
        char size_buf[SIZE_FIELD_SIZE + 1];
        if (fread(size_buf, 1, SIZE_FIELD_SIZE, in) != SIZE_FIELD_SIZE) {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            fclose(in); return 1;
        }
        size_buf[SIZE_FIELD_SIZE] = '\0';
        long long sz = atoll(size_buf);

        /* İzin (Mode) Oku */
        char mode_buf[MODE_FIELD_SIZE + 1];
        if (fread(mode_buf, 1, MODE_FIELD_SIZE, in) != MODE_FIELD_SIZE) {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            fclose(in); return 1;
        }
        mode_buf[MODE_FIELD_SIZE] = '\0';
        mode_t file_mode = (mode_t)strtol(mode_buf, NULL, 8);

        /* Hedef yol (Mutlak veya göreceli desteklenir) */
        char out_path[512];
        if (dest_dir && strcmp(dest_dir, ".") != 0)
            snprintf(out_path, sizeof(out_path), "%s/%s", dest_dir, name_buf);
        else
            snprintf(out_path, sizeof(out_path), "%s", name_buf);

        /* KURAL: Program aniden çökmemeli, tüm çıkışlar sorunsuz olmalı */
        FILE *out_f = fopen(out_path, "wb");
        if (!out_f) {
            perror(out_path);
            fclose(in);
            return 1;
        }

        long long remaining = sz;
        char buf[4096];
        while (remaining > 0) {
            size_t to_read = (remaining < (long long)sizeof(buf))
                             ? (size_t)remaining : sizeof(buf);
            size_t n = fread(buf, 1, to_read, in);
            if (n == 0) {
                printf("Arşiv dosyası uygunsuz veya bozuk!\n");
                fclose(out_f);
                fclose(in);
                return 1;
            }
            fwrite(buf, 1, n, out_f);
            remaining -= (long long)n;
        }
        fclose(out_f);
        
        /* KURAL: Orijinal izinleri (okuma/yazma/çalıştırma) geri yükle */
        chmod(out_path, file_mode);

        strncpy(extracted_names[i], name_buf, NAME_FIELD_SIZE - 1);
        extracted_names[i][NAME_FIELD_SIZE - 1] = '\0';
    }
    fclose(in);

    /* KURAL: İstenen formattaki çıktı mesajı */
    const char *dir_label = (dest_dir && strcmp(dest_dir, ".") != 0) ? dest_dir : ".";
    printf("%s dizininde ", dir_label);
    for (int i = 0; i < file_count; i++) {
        printf("%s", extracted_names[i]);
        if (i < file_count - 2) printf(", ");
        else if (i == file_count - 2) printf(" ve ");
    }
    printf(" dosyaları açıldı.\n");
    return 0;
}