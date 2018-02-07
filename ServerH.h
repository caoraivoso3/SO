#ifndef SERVER_H
#define SERVER_H

#define TOTALCOMMANDS 7
#define INFOFILE "UsernamesAndPasswords.txt"


#include "useful.h"



typedef struct logs
{
    FILE *f;
    MaxCliRun *cli;
    int *clisize;
    jogador * jog; // Ponteiro para array na Main()
    jogo *inic;
    bola *b;
    jogada * arrayjogadas;
    pthread_t *bots;
    int *killthread;
} threadlog;

typedef struct
{
    jogador *array;
    int minhapos;
    bola *b;
    jogada * arrayjogadas; //Array para fazer as jogadas
    int *killthread;
}threadbot;

typedef struct
{
    jogador *array;
    jogada *arrayjogada;
    MaxCliRun *cli;
    bola *b;
    jogo *jg;
    int *killthread;
    int *botsize;
}threadcheck;



/*  @readCommand check and validate commands sent by Administrator
 *  @param
 *      Clients' struct Logged In
 *      Banned user's PID
 *
 *  @return
 *      -1 -> Command not found or inaccurate
 *       0 -> Start new game (Command: start n)
 *       1 -> Stop game (Command: stop)
 *       2 -> Create new account (Command: user username password)
 *       3 -> Show users List (Command: users)
 *       4 -> Show current running game result (Command: result)
 *       5 -> Ban from game a certain player (Command: red username)
 *       6 -> Turn off Server (Command: shutdown)
 *
 */
int readCommand(char *cmd, int *botsize, MaxCliRun  *clientsOn, int nbrOnCli,
    pthread_t * bots,jogador * jogad,int *jogcont, jogo *jog,FILE *f,bola *b,
    pthread_t *timer,jogada * arrayjogada,int *killthread);
int iniciarJogo(int * botsize, MaxCliRun * clientsOn, int nbrOnCli,pthread_t *bots,
    jogador * jogad, int *jogcont, jogo * jog,FILE *f,bola *b,pthread_t *timer,
    jogada * arrayjogada,int *killthread,threadbot *bot,threadcheck * args);

void bolaanda(bola *b, jogador * jog, int checktipo,jogo *j,MaxCliRun *clientson);




/*Thread do Campo
* FILE * fuser, MaxCliRun  *clientsOn, int * clisize
*/
void *fieldThread(void *args);

/*Thread dos BOTS*/
void *guardaredes(void *args); // int equipa e jogador
void *defesa(void *args);
void *avancado(void *args);

/*Algoritmos*/
int jogadorMaisPerto(jogador *allJog, int type, int myPos);
float distOponenteMaisPerto(jogador *allJog, int myPos);
void getcoords(int *nposx,int *nposy, int x, int y, int tipojogada);
void getcoords8(int *nposx,int *nposy, int x, int y);
float dist2Pontos(int x1,int y1,int x2,int y2);
int followCoords(int posx, int posy, int targetX, int targetY, jogador *allJog, int myPos);
int isbolaperto(bola *b,jogador *jog, int pos);
int isposboa(jogador *jog, int pos, bola *b, int dire); // retorna dir se pos for permitida caso contrario retorna -1
/*@parameters Tem como argumento threadBot
* threadBot inclui : array de jogadas, clientes e ponteiro para bola
*/
void *checkguardaavan(void *args); // 3em3
void *checkdefesa(void *args); //4 em 4

void answerSignal();
void semclient();
void shutDownServer();
void acabajogo();

/*Variável Global para terminar as threads dos BOTS */
extern int morre;



#endif // SERVER
