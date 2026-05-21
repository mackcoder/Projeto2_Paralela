#include <omp.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

/*
- Andre Doerner Duarte - 10427938
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

#define MAX_LOG_LINES 10000000 // Limite máximo de linhas para evitar estouro de memória

int main(int argc, char** argv) {
    if (argc < 2) return printf("Uso: %s <log_file>\n", argv[0]), 1;

    // Construção a partir do manifest (Sequencial)
    HashTable* ht = ht_create(131071);
    FILE* f_man = fopen("manifest.txt", "r");
    if (!f_man) { printf("Erro no manifest\n"); return 1; }
    
    char line[1024];
    while (fgets(line, sizeof(line), f_man)) {
        line[strcspn(line, "\n")] = 0;
        ht_put(ht, line);
    }
    fclose(f_man);

    // Aloca um array de ponteiros para strings (vetor de strings)
    char** log_entries = malloc(MAX_LOG_LINES * sizeof(char*));
    FILE* f_log = fopen(argv[1], "r");
    if (!f_log) { printf("Erro no log\n"); free(log_entries); return 1; }
    
    int total_lines = 0;
    // Lê o arquivo sequencialmente e armazena na memória
    while (fgets(line, sizeof(line), f_log) && total_lines < MAX_LOG_LINES) {
        log_entries[total_lines++] = strdup(line); // Aloca memória exata e copia a linha
    }
    fclose(f_log);
    
    // Divide o loop 'for' entre as threads disponíveis no sistema
    #pragma omp parallel for 
    for (int i = 0; i < total_lines; i++) {
        char url[512];
        char *current_line = log_entries[i]; // Cada thread pega sua respectiva linha
        
        char *start = strstr(current_line, "GET ");
        if (start) {
            start += 4;
            char *end = strstr(start, " HTTP");
            if (end) {
                size_t len = end - start;
                if (len > sizeof(url) - 1) len = sizeof(url) - 1; // Proteção contra overflow
                strncpy(url, start, len);
                url[len] = '\0';

                // Busca na tabela hash (Operação de leitura: Segura para múltiplas threads)
                CacheNode* node = ht_get(ht, url);
                if (node) {
                    // SEÇÃO CRÍTICA: Protege contra Condição de Corrida
                    // Garante que apenas UMA thread por vez incremente o 'hit_count' deste nó
                    #pragma omp critical
                    {
                        node->hit_count++;
                    }
                }
            }
        }
        free(log_entries[i]); // Libera a string da linha assim que ela termina de ser processada
    }
    // Libera o array de ponteiros principal
    free(log_entries);

    ht_save_results(ht, "results.csv");
    ht_destroy(ht);
    return 0;
}
