#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

void display_menu() {
    printf("\n=== MENU ===\n");
    printf("1. Senhor dos Anéis\n");
    printf("2. O Poderoso Chefão\n");
    printf("3. Clube da Luta\n");
    printf("0. Sair\n");
    printf("=============\n");
    printf("Escolha uma opção: ");
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (addrparse(argv[1], argv[2], &storage) != 0) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_DGRAM, 0);

    if (s == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    printf("ready to send to %s\n", addrstr);

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    display_menu();

    while (1) {
        fgets(buf, BUFSZ - 1, stdin);

        size_t count = sendto(s, buf, strlen(buf) + 1, 0, addr, sizeof(storage));
        if (count != strlen(buf) + 1) {
            perror("sendto");
            close(s);
            exit(EXIT_FAILURE);
        }

        memset(buf, 0, BUFSZ);
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        count = recvfrom(s, buf, BUFSZ, 0, (struct sockaddr *)(&from), &fromlen);
        if (count == -1) {
            perror("recvfrom");
        } else {
            printf("received %zu bytes\n", count);
            puts(buf);

            // Check if received "fim" to terminate the loop
            if (strcmp(buf, "fim\n") == 0) {
                break;
            }
        }
    }

    close(s);
    exit(EXIT_SUCCESS);
}
