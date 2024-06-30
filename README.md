# Aplicativo Simples de Streaming - README

## Objetivo do Trabalho

Neste trabalho prático, os alunos serão desafiados a desenvolver uma aplicação console que simule o funcionamento básico de aplicativos de streaming, com um servidor capaz de lidar com múltiplas conexões de clientes utilizando multithreading. Além disso, a comunicação entre o cliente e o servidor será realizada por meio de uma conexão UDP. O servidor enviará frases aleatórias para cada cliente conectado a cada 3 segundos, que serão exibidas no console do cliente.

## Instruções para Utilização do Programa

1. **Compilação do Projeto:**
   - Compile o projeto utilizando o comando `make`.

2. **Inicialização do Servidor:**
   - Utilize o seguinte comando para iniciar o servidor:
     ```
     ./server-mt <ipv4|ipv6> <server port>
     ```
     Exemplo:
     ```
     ./server-mt ipv4 51511
     ```

3. **Inicialização da Conexão via Cliente:**
   - Utilize o seguinte comando para conectar o cliente ao servidor:
     ```
     ./client <ipv4|ipv6> <server IP> <server port>
     ```
     Exemplo:
     ```
     ./client ipv4 127.0.0.1 51514
     ```



# TP2-REDESofc
