#ifndef TARSAU_H
#define TARSAU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_FILES     32
#define MAX_TOTAL_MB  200
#define MAX_TOTAL_BYTES ((long long)MAX_TOTAL_MB * 1024 * 1024)
#define DEFAULT_ARCHIVE "a.sau"
#define NAME_FIELD_SIZE  20
#define SIZE_FIELD_SIZE  10
#define COUNT_FIELD_SIZE  4
#define MODE_FIELD_SIZE   8

// Başka dosyalardan çağrılacak ana fonksiyon prototipleri
int build_archive(int file_count, char *files[], const char *archive_name);
int extract_archive(const char *archive_name, const char *dest_dir);

#endif