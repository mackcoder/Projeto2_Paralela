#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define MAX_LOGS_LINES 10000000
#define BUCKET_SIZE 1024

/*
- Andre Doerner Duarte - 10427938
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

int main(int argc, char** argv){
    if (argc < 2) return printf("Uso: %s <log_file>\n", argv[0]), 1;

    // Construção a partir do manifest 
    HashTable* ht = ht_create(131071);
    FILE* f_man = fopen("manifest.txt", "r");

    if(!ht){
        fprintf(stderr, "Erro na criacao do Hash Table\n");
        return 1;
    }

    if(!f_man){
        perror(stderr, "Erro no acesso -> 'manifest.txt'\n");
        ht_destroy(ht);
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), f_man)) {
        line[strcspn(line, "\n")] = 0;
        ht_put(ht, line);
    }
    fclose(f_man);
    // Carregar log em memória para permitir parallel for
    char** log_entries = malloc(MAX_LOGS_LINES * sizeof(char*));
    FILE* f_log = fopen(argv[1], "r");
    int total_lines = 0;

    while (fgets(line, sizeof(line), f_log) && total_lines < MAX_LOGS_LINES) {
        log_entries[total_lines++] = strdup(line); // Duplica a string na memória
    }
    fclose(f_log);

    // Paralelizando Acesso aos buckets

    omp_lock_t* locks = malloc(sizeof(omp_lock_t) * ht -> size);
    for(size_t s = 0; s < ht -> size; s++){
        omp_set_lock(&locks[s]);
    }

    #pragma omp parallel for
    for(int x = 0; x < total_lines; x++){
        char *current_line = log_entries[x];
        char *start = strstr(current_line, "GET");

        if(start){
            start += 4;
            char * end = strstr(start, " HTTP");
            if(end){
                char url[512];

                size_t len = end - start;
                strcpy(url, start, len);
                url[len] = '\0';

                CacheNode* node = ht_get(ht, url);

                omp_lock_t* locks = malloc(sizeof(omp_lock_t) * ht -> size);
                size_t bucket = hash_function(url) % ht-> size;

                omp_set_lock(&locks[bucket]);
                node -> hit_count++;
                omp_unset_lock(&locks[bucket]);
            }
        }
        free(log_entries[x]);
    }

    for(int destroyer = 0; destroyer < ht -> size; destroyer++){
        omp_destroy_lcok(&locks[destroyer]);
    }
    
    free(locks);
    free(log_entries);

    ht_save_results(ht, "results.csv"); 
    ht_destroy(ht);

    return 0;

}
