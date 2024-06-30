#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024 // Define o tamanho do buffer usado para comunicação

// Função para mostrar o uso correto do programa e sair com código de falha
void usage(int argc, char **argv){
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    // Verifica se os argumentos necessários foram fornecidos
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    // Inicializa a estrutura de endereço do servidor com base na versão IP e porta
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }
    
    int s;
    s = socket(storage.ss_family, SOCK_DGRAM, 0); // Changed to SOCK_DGRAM for UDP
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting for messages\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);

        ssize_t count = recvfrom(s, buf, BUFSZ - 1, 0, caddr, &caddrlen); // Use recvfrom for UDP
        if (count == -1) {
            logexit("recvfrom");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        count = sendto(s, buf, strlen(buf) + 1, 0, caddr, caddrlen); // Use sendto for UDP
        if (count == -1) {
            logexit("sendto");
        }
    }

    close(s);
    exit(EXIT_SUCCESS);
}
