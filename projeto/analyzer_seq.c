#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

/*
- Andre Doerner Duarte - 10427938
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

int main(int argc, char** argv) {
    // Valida se o usuário passou o arquivo de log como argumento na linha de comando
    // Exemplo de uso: ./programa arquivo_de_log.txt
    if (argc < 2) {
        return printf("Uso: %s <log_file>\n", argv[0]), 1;
    }

    // Cria a tabela hash com um tamanho primo grande (131071) para reduzir colisões
    HashTable* ht = ht_create(131071);
    
    // Abre o arquivo "manifest.txt" para leitura
    FILE* f_man = fopen("manifest.txt", "r");
    if (!f_man) {
        printf("Erro ao abrir manifest.txt\n");
        ht_destroy(ht);
        return 1;
    }

    char line[1024];
    // Lê o manifesto linha por linha
    while (fgets(line, sizeof(line), f_man)) {
        // Remove a quebra de linha ('\n') e substitui por '\0'
        line[strcspn(line, "\n")] = 0;
        
        // Insere a URL limpa na tabela hash
        ht_put(ht, line);
    }
    fclose(f_man);
    
    // Abre o arquivo de log passado pelo usuário via linha de comando (argv[1])
    FILE* f_log = fopen(argv[1], "r");
    if (!f_log) {
        printf("Erro ao abrir o arquivo de log: %s\n", argv[1]);
        ht_destroy(ht);
        return 1;
    }

    char url[512];
    // Lê o arquivo de log linha por linha
    while (fgets(line, sizeof(line), f_log)) {
        // Busca o padrão "GET " na linha do log
        char *start = strstr(line, "GET ");
        if (start) {
            start += 4; // Avança o ponteiro 4 caracteres para apontar para o início da URL
            
            // Busca o padrão " HTTP" após o início da URL
            char *end = strstr(start, " HTTP");
            if (end) {
                // Calcula o tamanho exato da string da URL
                size_t len = end - start;
                
                // Copia a URL extraída para a variável 'url' com segurança baseada no tamanho
                strncpy(url, start, len);
                url[len] = '\0'; // Garante o caractere nulo terminador da string

                // Busca a URL extraída dentro da tabela hash
                CacheNode* node = ht_get(ht, url);
                if (node) {
                    // Se a URL existia no manifesto, incrementa o contador de acessos (hits)
                    node->hit_count++;
                }
            }
        }
    }
    fclose(f_log);

    // Grava as estatísticas coletadas no arquivo "results.csv"
    ht_save_results(ht, "results.csv");
    
    // Libera a memória alocada para a tabela hash
    ht_destroy(ht);
    
    return 0;
}
