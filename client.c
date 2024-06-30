#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

// Função para exibir o uso correto do programa
void usage(int argc, char **argv) {
    printf("usage: %s <ipv4|ipv6> <server IP> <server port>\n", argv[0]);
    printf("example: %s ipv4 127.0.0.1 51514\n", argv[0]);
    exit(EXIT_FAILURE);
}

// Função para exibir o menu de opções
void display_menu() {
    printf("\n=== MENU ===\n");
    printf("1. Senhor dos Anéis\n");
    printf("2. O Poderoso Chefão\n");
    printf("3. Clube da Luta\n");
    printf("0. Sair\n");
    printf("=============\n");
}

int main(int argc, char **argv) {
    // Verifica se o número de argumentos é suficiente
    if (argc < 4) {
        usage(argc, argv);
    }
    
    while (1) { // Loop infinito para manter o programa em execução
        struct sockaddr_storage storage;
        // Parseia os argumentos e inicializa a estrutura de endereço
        if (0 != addrparse(argv[1], argv[2], argv[3], &storage)) {
            usage(argc, argv);
        }

        struct sockaddr *addr = (struct sockaddr *)(&storage);
        socklen_t addrlen = sizeof(storage);

        // Cria o socket
        int s = socket(storage.ss_family, SOCK_DGRAM, 0);
        if (s == -1) {
            logexit("socket");
        } 
        
        // Exibe o menu de seleção
        display_menu();

        int escolha;
        printf("Escolha uma opção: ");
        scanf("%d", &escolha);
        getchar();  // Consome o newline deixado pelo scanf

        switch (escolha) {
            case 0:
                // Fechar o socket e encerrar o programa
                close(s);
                exit(EXIT_SUCCESS);

            default: {
                // Verifica se a escolha é válida
                if (escolha < 0 || escolha >= 4) {
                    printf("Opção inválida.\n");
                    close(s);
                    continue;
                }

                char buf[BUFSZ];
                memset(buf, 0, BUFSZ);

                // Converte a escolha para string
                snprintf(buf, BUFSZ, "%d", escolha);
                // Envia a escolha para o servidor
                size_t count = sendto(s, buf, strlen(buf) + 1, 0, addr, addrlen);
                if (count == -1) {
                    perror("sendto");
                    logexit("sendto");
                }

                memset(buf, 0, BUFSZ);
                // Recebe a resposta do servidor
                count = recvfrom(s, buf, BUFSZ, 0, addr, &addrlen);
                if (count == -1) {
                    perror("recvfrom");
                    logexit("recvfrom");
                }

                // Divide a resposta em frases
                char** frases = str_split(buf, ';');
                if (frases) {
                    for (int i = 0; frases[i]; i++) {
                        printf("%s\n", frases[i]);
                        usleep(3000000); // 3 segundos de pausa entre frases
                        free(frases[i]);  // Libera a memória alocada para a frase
                    }
                    free(frases);  // Libera a memória alocada para o array de ponteiros
                }
                close(s); // Fecha o socket após a comunicação
                break;
            }
        }
    }

    return 0;
}
