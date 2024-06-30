#include <inttypes.h> // Biblioteca para definições de tipos de dados com largura específica.
#include <stdio.h> // Biblioteca padrão para entrada e saída.
#include <stdlib.h> // Biblioteca padrão para funções de alocação de memória, controle de processos, etc.
#include <string.h> // Biblioteca para manipulação de strings.
#include <arpa/inet.h> // Biblioteca para manipulação de endereços de rede.

void logexit(const char *msg) {
    // Função para registrar uma mensagem de erro e sair do programa.
    perror(msg); // Imprime a mensagem de erro associada à última chamada de sistema que falhou.
    exit(EXIT_FAILURE); // Termina a execução do programa com falha.
}

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage) {
    // Função para analisar a string de endereço e porta e preencher a estrutura sockaddr_storage.
    if (addrstr == NULL || portstr == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // Converte a string da porta para um número inteiro sem sinal.
    if (port == 0) {
        return -1;
    }

    port = htons(port); // Converte o número da porta de ordem de bytes do host para ordem de bytes de rede.

    struct in_addr inaddr4; // Estrutura para armazenar um endereço IPv4 de 32 bits.

    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        // Tenta converter o endereço IPv4 da string para formato binário.
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // Estrutura para armazenar um endereço IPv6 de 128 bits.

    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        // Tenta converter o endereço IPv6 da string para formato binário.
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6)); // Copia o endereço IPv6.
        return 0;
    }

    return -1; // Retorna erro se não conseguiu converter o endereço.
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    // Função para converter um endereço de rede em uma string legível.
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = ""; // Buffer para armazenar o endereço como string.
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        // Trata um endereço IPv4.
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // Converte o número da porta de ordem de bytes de rede para ordem de bytes do host.
    } else if (addr->sa_family == AF_INET6) {
        // Trata um endereço IPv6.
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // Converte o número da porta de ordem de bytes de rede para ordem de bytes do host.
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port); // Formata a string com o endereço e a porta.
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    // Função para inicializar a estrutura sockaddr_storage para um servidor.
    uint16_t port = (uint16_t)atoi(portstr); // Converte a string da porta para um número inteiro sem sinal.
    if (port == 0) {
        return -1;
    }
    port = htons(port); // Converte o número da porta de ordem de bytes do host para ordem de bytes de rede.

    memset(storage, 0, sizeof(*storage)); // Inicializa a estrutura de armazenamento com zeros.
    if (0 == strcmp(proto, "v4")) {
        // Inicializa para um endereço IPv4.
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY; // Define o endereço para aceitar conexões de qualquer endereço.
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        // Inicializa para um endereço IPv6.
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any; // Define o endereço para aceitar conexões de qualquer endereço.
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1; // Retorna erro se o protocolo não for reconhecido.
    }
}
