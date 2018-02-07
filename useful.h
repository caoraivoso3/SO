#ifndef USEFUL_H
#define USEFUL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <ncurses.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define SERVER_FIFO "sf"
#define MAXCLIENTS 20
#define MAX 20
#define MAXCMDLEN 50
#define MAXPASS 21
#define MAXUSER 21
#define MAXFILENAME 100
#define fieldY 21
#define fieldX 51
#define MAXJOGADOR 18 // Diferente do Cliente
#define TEMPOJOGAR 10 // Décimas de Segundo

typedef struct inicio logi;
typedef struct jogado jogador;
typedef struct jogadas jogada;
typedef struct bol
{
    jogador *dele;
    int posx;
    int posy;
    jogador * dest; // Se Null, vai em direção da baliza
    int isremate; // 1 Remata, 0 Não Remate
    int destposx; // Caso a bola falhe vai para um sitio random
    int destposy;
    int tipo; // Mesmo tipo que os jogadores
    int equipa;

}bola;

typedef struct {
    char cliFifo[20];
    int Filecli;
    char nome[MAXUSER];
    jogador * meu;
    int equipaprimaria; // O Jogador fica na Equipa que escolhe
}MaxCliRun;

typedef struct
{
        int contin;
        int pausa;
	int l, c;
	pthread_t task;
	void * retval;
	WINDOW * gameWindow;
        WINDOW *cmdui;
        WINDOW *golosmostra; // Mostra os Golos
        WINDOW * instrucao; // Mostra instruções do jogo, durante o jogo
        int fdcli;
        int sfcli;
        int * breakjogo;
        int * continuajogo;
}client;


typedef struct
{
    int comeco; // 0 nao começado 1 começado
    int resequipa0;
    int resequipa1;
    int temporizador;
    int tamequipa;
} jogo;

typedef struct
{
    // 3 Atualizar jogo o Cliente para apenas um jogador __ Já não é preciso
    // 4 Jogo enviado por bot para Server __ Já nao é preciso
    // 6 Struct da bola do Server para cliente __ Já nao é preciso


    int tipo;
        // 0 Struct de login Cliente para Servidor
        // 1 Struct que envia informaçao do login para o cliente
        // 2 Inicio do jogo po cliente
        // 3 Cliente pede ao servidor, determinado jogador
        // 4 Resposta do servidor ao pedido do jogador pelo cliente
        // 5 Cliente envia para o servidor as teclas do Utilizador
        // 6 Cliente envia para o servidor a info que vai sair do jogo
        // 7 Cliente envia para o servidor a dizer que vair sair do Cliente
        // 8 Server envia para cliente o Resultado actual do jogo(golos)
        // 9 cartao vermelho  do servidor para o cliente usa a estrutura do tipo 6 
    union
    {
        /*reslog _ tipo _ */
        // 0 Não Conectou (Pass errada), 1 Conectou, 2 Conta Inexistente
        struct
        {
            int res;
        }reslog;

        /*log _ tipo _ */
        struct inicio
        {
            char password[MAXPASS],username[MAXUSER];
            char cliFifo[20];
        } log;

        /*atualizaJogo _ tipo _ */
        // Info para Update da GUI do Cliente
        struct
        {
            /* *jogador _ Data sobre os jogadores*/
            struct jogado
           {
               int tipo; // 0 Guarda-Redes, 1 Defesa, 2 Avançado
               int equipa;
               int jogador;
               int posx;
               int posy;
               MaxCliRun *cliente;
               int ultimajogada;
           }jogador[MAXJOGADOR];

           /*dataBola _ Data sobre a bola*/
           struct
           {
            int posx;
            int posy;
            int equipa;
            int jogador;
           } databola;

        } atualizajogo;

        struct jogadas
        {
            char cliFifo[20];
            // 0 Baixo, 1 Cima, 2 Esquerda, 3 Direita, 4 Esquerda Cima
            // 5 Direita Cima,6 Esquerda Baixo, 7 Direita Baixo
            // -1 Fica Imóvel 8 Passa
            int jogada;
            // 0 Para ninguém, outros valores para os respectivos jogadores da equipa
            int passe;
            //0 Equipa da Esquerda, 1 da Direita, -1 Não estão no Jogo
            int equipa;
            // Número do Jogador na Equipa (de 1 a 9) depende do número de defesas e avançados
            int jogador;
            // Info
            //jogada = 8 + passe = 0 para rematar
            //jogada = 8 + passe = numero do jogador para passar
        } jogada;

        /*jogadaJog _ tipo _ */
        struct pedirjogarserver
        {
            int equipa;
            int jogador;
            char cliFifo[20];
        }jogadajog;

        /*resJogadas _ tipo _ */
        struct
        {
            int res; // 0 Falhar, 1 Acertar
            int equipa;
            int jogador;
        }resjogadas;

        /*clientQuit _ tipo 7 */
        //São do cliente e não apenas do jogo
        struct
        {
            int quit;  // 1 Quit
            char clififo[20];
        } clientquit;

        /*servQuit _ tipo _ */
        //O Servidor é desligado e envia para todos os Clientes Conectados
        struct
        {
            int servquit; // 1 Quit
        } servquit;

        /* sairJogo _ tipo 6 e 9*/
        // Sai do Jogo, mas continua conectado 
        struct
        {
          char clififo[20];
          int quitjogo; // 1 Sair do Jogo 2 sair do jogo por cartao vermelho do admin do servidor
        }sairjogo ;

        /* atualizarres _ tipo 8*/
        // Resultados das equipas
        struct
        {
            int resequip0;
            int resequip1;
        }atualizarres;

    } data; // Union

} tipo;

#endif
