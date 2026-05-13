#include "tarsau.h"

/* ────────────────────────────────────────────────────────────────────────
 * main
 * ──────────────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Kullanım:\n");
        fprintf(stderr, "  %s -b dosya1 [dosya2 ...] [-o arşiv.sau]\n", argv[0]);
        fprintf(stderr, "  %s -a arşiv.sau [hedef_dizin]\n", argv[0]);
        return 1;
    }

    /* ── -b modu ── */
    if (strcmp(argv[1], "-b") == 0) {
        char *input_files[MAX_FILES];
        int   input_count = 0;
        const char *archive_name = DEFAULT_ARCHIVE;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Hata: -o seçeneğinden sonra arşiv adı belirtilmelidir.\n");
                    return 1;
                }
                archive_name = argv[++i];
            } else {
                if (input_count >= MAX_FILES) {
                    fprintf(stderr, "Hata: en fazla %d giriş dosyası belirtilebilir.\n", MAX_FILES);
                    return 1;
                }
                input_files[input_count++] = argv[i];
            }
        }
        return build_archive(input_count, input_files, archive_name);
    }

    /* ── -a modu ── */
    if (strcmp(argv[1], "-a") == 0) {
        /* KURAL: -a parametresinden sonra en fazla 2 parametre almalıdır */
        if (argc < 3 || argc > 4) {
            printf("Hata: -a parametresinden sonra 1 veya 2 parametre girilmelidir.\n");
            return 1;
        }
        
        const char *archive_name = argv[2];
        const char *dest_dir     = (argc == 4) ? argv[3] : ".";
        
        return extract_archive(archive_name, dest_dir);
    }

    fprintf(stderr, "Hata: geçersiz seçenek '%s'. -b veya -a kullanın.\n", argv[1]);
    return 1;
}