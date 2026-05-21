#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define MAX_LOG_LINES 10000000
#define BUCKET_SIZE 1024

/*
- Andre Doerner Duarte - 10427938
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

int main(int argc, char** argv){
    // Valida se o arquivo de log foi passado como argumento na linha de comando
    if (argc < 2) return printf("Uso: %s <log_file>\n", argv[0]), 1;

    // Aloca a tabela hash com um tamanho primo para reduzir colisões
    HashTable* ht = ht_create(131071);
    
    // Abre o arquivo que contém as URLs que devem ser monitoradas
    FILE* f_man = fopen("manifest.txt", "r");

    // Garante que a tabela hash foi alocada com sucesso
    if(!ht){
        fprintf(stderr, "Erro na criacao do Hash Table\n");
        return 1;
    }

    // Garante que o arquivo de manifesto existe e foi aberto
    if(!f_man){
        perror("Erro no acesso -> 'manifest.txt'\n");
        ht_destroy(ht); // Libera a tabela hash para evitar vazamento de memória
        return 1;
    }

    char line[1024];
    // Lê o manifesto linha por linha (processo sequencial)
    while (fgets(line, sizeof(line), f_man)) {
        // Remove o caractere de nova linha ('\n') trocando-o pelo terminador '\0'
        line[strcspn(line, "\n")] = 0;
        
        // Guarda a URL lida na tabela hash
        ht_put(ht, line);
    }
    fclose(f_man);

    // Aloca um vetor de ponteiros (char*) capaz de guardar até 10 milhões de strings
    char** log_entries = malloc(MAX_LOG_LINES * sizeof(char*));
    FILE* f_log = fopen(argv[1], "r");
    int total_lines = 0;

    // Lê o arquivo de log sequencialmente até o fim ou até atingir o limite máximo definido
    while (fgets(line, sizeof(line), f_log) && total_lines < MAX_LOG_LINES) {
        // strdup aloca o espaço exato na memória e copia a linha atual para o vetor
        log_entries[total_lines++] = strdup(line); 
    }
    fclose(f_log);

    // Diretiva OpenMP: Divide as iterações do loop 'for' entre os núcleos da CPU
    #pragma omp parallel for
    for(int x = 0; x < total_lines; x++){
        // Cada thread isola a linha que ficou responsável por processar
        char *current_line = log_entries[x];
        
        // Procura a palavra "GET" na linha do log
        char *start = strstr(current_line, "GET");

        if(start){
            start += 4; // Avança o ponteiro para o início da URL (considerando o espaço após o GET)
            
            // Procura o final da URL delimitado pelo protocolo " HTTP"
            char * end = strstr(start, " HTTP");
            if(end){
                char url[512];

                // Calcula o comprimento da string da URL e faz a cópia
                size_t len = end - start;
                strncpy(url, start, len);
                url[len] = '\0'; // Garante a terminação correta da string

                // Faz a busca da URL extraída dentro da tabela hash
                CacheNode* node = ht_get(ht, url);

                #pragma omp atomic update
                node -> hit_count++;
            }
        }
        // Liberação imediata da memória da linha limpa o cache conforme o loop avança
        free(log_entries[x]);
    }

    // Libera o vetor de ponteiros principal após todas as linhas serem apagadas
    free(log_entries);

    // Grava os resultados consolidados no arquivo CSV
    ht_save_results(ht, "results.csv"); 
    
    // Desaloca completamente a estrutura da tabela hash da memória
    ht_destroy(ht);

    return 0;
}
