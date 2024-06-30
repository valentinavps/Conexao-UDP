#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024
#define NUM_QUOTES 5  // Defina o número total de citações

char *quotes1[NUM_QUOTES] = {
    "Um anel para a todos governar",
    "Na terra de Mordor onde as sombras se deitam",
    "Não é o que temos, mas o que fazemos com o que temos",
    "Não há mal que sempre dure",
    "O mundo está mudando, senhor Frodo"
};

char *quotes2[NUM_QUOTES] = {
    "Vou fazer uma oferta que ele não pode recusar",
    "Mantenha seus amigos por perto e seus inimigos mais perto ainda",
    "É melhor ser temido que amado",
    "A vingança é um prato que se come frio",
    "Nunca deixe que ninguém saiba o que você está pensando"
};

char *quotes3[NUM_QUOTES] = {
    "Primeira regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "Segunda regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "O que você possui acabará possuindo você",
    "É apenas depois de perder tudo que somos livres para fazer qualquer coisa",
    "Escolha suas lutas com sabedoria"
};

struct client_info {
    int sockfd;
    struct sockaddr_storage caddr;
    socklen_t caddrlen;
};

void usage(int argc, char **argv){
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void send_quotes(int s, struct sockaddr *caddr, socklen_t caddrlen, char *quotes[]) {
    for (int i = 0; i < NUM_QUOTES; i++) {
        ssize_t count = sendto(s, quotes[i], strlen(quotes[i]) + 1, 0, caddr, caddrlen);
        if (count == -1) {
            logexit("sendto");
        }
        sleep(3); // Delay de 3 segundos entre as mensagens
    }
}

int main(int argc, char **argv){
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
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

        if (strcmp(buf, "1\n") == 0) {
            send_quotes(s, caddr, caddrlen, quotes1);
        } else if (strcmp(buf, "2\n") == 0) {
            send_quotes(s, caddr, caddrlen, quotes2);
        } else if (strcmp(buf, "3\n") == 0) {
            send_quotes(s, caddr, caddrlen, quotes3);
        } else {
            sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
            count = sendto(s, buf, strlen(buf) + 1, 0, caddr, caddrlen); // Use sendto for UDP
            if (count == -1) {
                logexit("sendto");
            }
        }
    }

    close(s);
    exit(EXIT_SUCCESS);
}