#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define BUFSZ 1024

// Frases a serem enviadas para os clientes
char* frases1[] = {
    "Um anel para a todos governar",
    "Na terra de Mordor onde as sombras se deitam",
    "Não é o que temos, mas o que fazemos com o que temos",
    "Não há mal que sempre dure",
    "O mundo está mudando, senhor Frodo"
};

char* frases2[] = {
    "Vou fazer uma oferta que ele não pode recusar",
    "Mantenha seus amigos por perto e seus inimigos mais perto ainda",
    "É melhor ser temido que amado",
    "A vingança é um prato que se come frio",
    "Nunca deixe que ninguém saiba o que você está pensando"
};

char* frases3[] = {
    "Primeira regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "Segunda regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "O que você possui acabará possuindo você",
    "É apenas depois de perder tudo que somos livres para fazer qualquer coisa",
    "Escolha suas lutas com sabedoria"
};

// Estrutura para armazenar os dados do cliente
struct client_data {
    int csock;
    int escolha;
    struct sockaddr_in client_addr;
};

// Variáveis para contagem de threads ativas
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int active_threads = 0;

// Função executada pela thread do cliente
void *client_thread(void *data) {
    pthread_mutex_lock(&mutex);
    active_threads++;
    pthread_mutex_unlock(&mutex);

    struct client_data *cdata = (struct client_data *)data;

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Seleciona as frases a serem enviadas com base na escolha do cliente
    switch(cdata->escolha) {
        case 1:
            snprintf(buf, BUFSZ, "%s;%s;%s;%s;%s", frases1[0], frases1[1], frases1[2], frases1[3], frases1[4]);
            break;
        case 2:
            snprintf(buf, BUFSZ, "%s;%s;%s;%s;%s", frases2[0], frases2[1], frases2[2], frases2[3], frases2[4]);
            break;
        case 3:
            snprintf(buf, BUFSZ, "%s;%s;%s;%s;%s", frases3[0], frases3[1], frases3[2], frases3[3], frases3[4]);
            break;
        default:
            snprintf(buf, BUFSZ, "Escolha inválida.");
            break;
    }

    // Envia as frases para o cliente
    size_t count = sendto(cdata->csock, buf, strlen(buf), 0, (struct sockaddr*)&cdata->client_addr, sizeof(cdata->client_addr));
    if (count == -1) {
        perror("sendto");
    }
    usleep(15000000); // Pausa de 15 segundos
    close(cdata->csock);
    free(cdata);

    pthread_mutex_lock(&mutex);
    active_threads--;
    pthread_mutex_unlock(&mutex);

    pthread_exit(EXIT_SUCCESS);
}

// Função executada pela thread de monitoramento
void *monitor_thread(void *arg) {
    while (1) {
        
        sleep(3); // Espera 3 segundos

        pthread_mutex_lock(&mutex);
        printf("Clientes conectados: %d\n", active_threads); // Imprime o número de clientes conectados
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Função para exibir o uso correto do programa
void usage(int argc, char **argv) {
    printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
    printf("example: %s ipv4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];
    socklen_t addr_size;

    // Verifica se o número de argumentos é suficiente
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    // Inicializa a estrutura de endereço do servidor
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int sockfd;
    // Cria o socket do servidor
    sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        logexit("socket");
    }

    int enable = 1;
    // Configura o socket para reutilização de endereço
    if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    // Associa o socket ao endereço e porta especificados
    if (0 != bind(sockfd, addr, sizeof(storage))) {
        logexit("bind");
    }

    // Criação da thread de monitoramento
    pthread_t monitor_tid;
    pthread_create(&monitor_tid, NULL, monitor_thread, NULL);
    pthread_detach(monitor_tid);

    while (1) {
        
        printf("Aguardando mensagens...\n");
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        bzero(buffer, 1024);
        addr_size = sizeof(client_addr);
        // Recebe dados do cliente
        recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&client_addr, &addr_size);

        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata) {
            perror("malloc");
            exit(1);
        }

        cdata->csock = socket(AF_INET, SOCK_DGRAM, 0);
        if (cdata->csock < 0) {
            perror("[-]socket error");
            free(cdata);
            continue;
        }
        cdata->client_addr = client_addr;
        // Converte a escolha do cliente para inteiro
        cdata->escolha = atoi(buffer);

        pthread_t tid;
        // Cria uma thread para lidar com o cliente
        pthread_create(&tid, NULL, client_thread, cdata);
        // Torna a thread destacada
        pthread_detach(tid);
    }

    // Fecha o socket do servidor
    close(sockfd);
    return 0;
}
