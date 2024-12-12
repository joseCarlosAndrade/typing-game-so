# Jogo de digitação competitivo - Projeto final de Sistemas operacionais

Realizado por:
-Felipe Carneiro Machado - 14569373
-José Carlos Andrade do Nascimento - 12549450
-Shogo Shima - 12675145
-Thiago Zero Araujo - 11814183

Este projeto é o trabalho final da disciplina de Sistemas Operacionais, ICMC-USP, onde implementamos um jogo de digitação competitivo, onde múltiplos jogadores tentam digitar rapidamente uma frase, acompanhando em tempo real o seu progresso e o de outros jogadores. Este projeto tem como objetivo implementar conceitos de programação concurrente, como threads e semáforos.


## Gameplay   

Jogadores buscam digitar uma frase de forma correta o mais rápido possível, podendo acompanhar sua pontuação e a dos outros jogadores em tempo real.

## Detalhes de implementação  

### Cliente
-3 threads, 1 para renderização e game loop principal, 1 para enviar informação ao servidor e 1 para receber informações do servidor
-1 semáforo no modelo produtor/consumidor: thread principal determina pontuação a ser enviada para o servidor, adiciona a uma fila (região crítica protegida por mutex), thread de envio ao servidor
lê essa fila e envia, caso esteja vazia, thread dorme.

### Servidor
-2 + 2 x num_jogadores threads, 1 para receber comandos via stdin, 1 para verficar se todos os jogadores já finalizaram, dormindo quando a condição é falsa, visando evitar 
busy waiting, e 2 threads por client, 1 recebe informação e a outra envia.
-Semáforo do estado do jogo: mutex utilizado pela primeira thread para alterar o estado do servidor, o qual é representado por valores de uma enum, e cuja mudança também implica em ações de envio de mensagens
-Semáforo de finalização dos jogadores: mutex utilizado pelas threads de recebimento de mensagens e de verificação de finalização para alterar informações desse aspecto.
-Semáforo do ranking: mutex para alterar e ler o ranking das pontuações dos jogadores, utilizado por todas as threads.


## Executando o jogo

Para rodar o jogo, é necessãrio executar primmeiro o servidor, e depois, quantos clientes desejar. Os clientes são os jogadores que vão digitar e competir. Segue abaixo o passo a passo.

### Executando o servidor

Para executar o servidor, digite em seu terminal (estando na pasta deste projeto):

```bash
cd server
```

```bash
make all
make run
```

Após iniciá-lo e havendo interfaces conectadas, pelo terminal envie os comandos "phrase" para enviar a frase para os clientes, e "start" para iniciar o jogo.

### Executando a interface 

É necessário instalar a SDL2 na sua máquina. O código utiliza `#include<SDL2/SDL.h>`, o que é compatível com sistemas linux, e provavelmente MacOS, porém provavelmente levará a erros em sistemas windows.


Dependências (outros package managers também possuem essas bibliotecas, busque na documentação deles pelo pacote correto caso seja necessário):

```bash
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-2.0
sudo apt install libsdl2-ttf-dev
```

Para executar, entre na pasta deste repositório e execute:

```bash
cd interface
```

```bash
make all
make run NAME=your_name
```

A janela se abrirá e aguardará intruções do servidor.
