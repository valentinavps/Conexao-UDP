#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
    printf("usage: %s <server IP> < server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) { 
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_DGRAM, 0); // Changed to SOCK_DGRAM for UDP

    if (s == -1) {
        logexit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    printf("ready to send to %s\n", addrstr); // Updated message to reflect UDP context

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("mensagem> ");
    fgets(buf, BUFSZ-1, stdin);

    size_t count = sendto(s, buf, strlen(buf)+1, 0, addr, sizeof(storage)); // Use sendto for UDP
    if (count != strlen(buf)+1) {
        logexit("sendto");
    }

    memset(buf, 0, BUFSZ);
    struct sockaddr_storage from;
    socklen_t fromlen = sizeof(from);
    count = recvfrom(s, buf, BUFSZ, 0, (struct sockaddr *)(&from), &fromlen); // Use recvfrom for UDP
    if (count == -1) {
        logexit("recvfrom");
    }

    printf("received %zu bytes\n", count);
    puts(buf);

    close(s);
    exit(EXIT_SUCCESS);
}
