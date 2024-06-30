#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>

// Função para exibir mensagens de erro e encerrar o programa
void logexit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Função para analisar o endereço e preencher a estrutura sockaddr_storage
int addrparse(const char *proto, const char *addrstr, const char *portstr, struct sockaddr_storage *storage) {
    if (addrstr == NULL || portstr == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // Converte a string de porta para unsigned short de 16 bits
    if (port == 0) {
        return -1;
    }
    port = htons(port); // Converte o número da porta para big-endian

    struct in_addr inaddr4; // Endereço IP de 32 bits
    if (strcmp(proto, "ipv4") == 0 && inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // Endereço IP de 128 bits
    if (strcmp(proto, "ipv6") == 0 && inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6)); // Copia o endereço IPv6
        return 0;
    }
    return -1;
}

// Função para converter um endereço sockaddr em uma string
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) { // Se o endereço for IPv4
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // Converte a porta para host byte order
    } else if (addr->sa_family == AF_INET6) { // Se o endereço for IPv6
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // Converte a porta para host byte order
    } else {
        logexit("unknown protocol family.");
    }

    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port); // Formata a string de saída
    }
}

// Função para inicializar a estrutura sockaddr_storage para um servidor
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr); // Converte a string de porta para unsigned short de 16 bits
    if (port == 0) {
        return -1;
    }
    port = htons(port); // Converte o número da porta para big-endian

    memset(storage, 0, sizeof(*storage)); // Inicializa a estrutura com zeros
    if (0 == strcmp(proto, "ipv4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY; // Aceita conexões de qualquer endereço
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(proto, "ipv6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any; // Aceita conexões de qualquer endereço
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}

// Função para dividir uma string em substrings usando um delimitador
char** str_split(char* a_str, const char a_delim) {
    char** result = 0;
    size_t count = 0;
    char* tmp = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
