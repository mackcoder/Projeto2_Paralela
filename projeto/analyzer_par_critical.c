#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define MAX_LOG_LINES 10000000

int main(int argc, char** argv) {
    if (argc < 2) return printf("Uso: %s <log_file>\n", argv[0]), 1;

    // Construção a partir do manifest 
    HashTable* ht = ht_create(131071);
    FILE* f_man = fopen("manifest.txt", "r");
    char line[1024];
    while (fgets(line, sizeof(line), f_man)) {
        line[strcspn(line, "\n")] = 0;
        ht_put(ht, line);
    }
    fclose(f_man);

    // Carregar log em memória para permitir parallel for
    char** log_entries = malloc(MAX_LOG_LINES * sizeof(char*));
    FILE* f_log = fopen(argv[1], "r");
    int total_lines = 0;

    while (fgets(line, sizeof(line), f_log) && total_lines < MAX_LOG_LINES) {
        log_entries[total_lines++] = strdup(line); // Duplica a string na memória
    }
    fclose(f_log);

    // Processamento do Log (Paralelo)
    #pragma omp parallel for 
    for (int i = 0; i < total_lines; i++) {
        char url[512];
        char *current_line = log_entries[i];
        
        char *start = strstr(current_line, "GET ");
        if (start) {
            start += 4;
            char *end = strstr(start, " HTTP");
            if (end) {
                size_t len = end - start;
                strncpy(url, start, len);
                url[len] = '\0';

                CacheNode* node = ht_get(ht, url);
                if (node) {
                    #pragma omp critical
                    {
                        node->hit_count++;
                    }
                }
            }
        }
        free(log_entries[i]); // Libera a linha após processar
    }
    free(log_entries);

    ht_save_results(ht, "results.csv");
    ht_destroy(ht);
    return 0;
}