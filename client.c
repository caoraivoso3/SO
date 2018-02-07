#include "useful.h"

typedef struct
{
    client *cli;
    jogador *jog;
    int * continuaciclo;
    int *meuboneco;
    bola *b;
    int *mata;
    jogo *jo;
}fieldthread;


void * footballField(void* pdata);
void fexarclient(int s,siginfo_t *info, void *context);
int login(WINDOW *cmdUI,client *cli);
void cfexar();

int saircliente;
WINDOW * cmdUI;

int main() {
	int res;
	char cmd[15];
	logi cur;
    client cli;
    jogador jog[MAXJOGADOR];
    int meuboneco=-1; // posicao no array do boneco do jgador
    int i;
    bola b;
    jogo j;

    //Client PID to server is able to identify client named pipe
    signal(SIGINT,cfexar);
    signal(SIGHUP,cfexar);
    struct sigaction act;
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = fexarclient;
	sigaction(SIGUSR1, &act, NULL);
    snprintf(cur.cliFifo,sizeof(cur.cliFifo),"CF%d",getpid());
    mkfifo(cur.cliFifo,0600);

	// -- inicializa��o do terminal ncurses ANTES das threads
	initscr();
	start_color();
	clear();
	cbreak();
	// noecho(); // teclas escritas n�o apareciam
	curs_set(0); // cursor n�o visivel

	init_pair(1, COLOR_BLUE, COLOR_GREEN);
	init_pair(2, COLOR_RED, COLOR_GREEN);
	init_pair(3,COLOR_YELLOW,COLOR_BLUE);
    init_pair(4, COLOR_BLUE, COLOR_WHITE);
    init_pair(5, COLOR_RED, COLOR_WHITE);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);

	cmdUI = newwin(10,25,1,fieldX + 2);
    keypad(cmdUI,TRUE);
	wattrset(cmdUI, COLOR_PAIR(3));
	scrollok(cmdUI, TRUE); // "turn on" "standard" window scroll

	wbkgd(cmdUI, COLOR_PAIR(3));
	wclear(cmdUI);
	wrefresh(cmdUI);


	cli.contin = 1;
	cli.gameWindow = newwin(fieldY, fieldX, 1, 1);
    cli.golosmostra = newwin(10,30,1+fieldY,1);
    //	wattrset(cli.gameWindow, COLOR_PAIR(1));
    wattrset(cli.gameWindow, COLOR_PAIR(4));
    //      wattrset(cli.gameWindow, COLOR_PAIR(2));
    wbkgd(cli.gameWindow, COLOR_PAIR(1));
    wbkgd(cli.golosmostra, COLOR_PAIR(6));
    wclear(cli.gameWindow);
    wclear(cli.golosmostra);
    j.resequipa0=0;
    j.resequipa1=0;
    for(i=0;i<9;i++)
    {
         wattron(cli.gameWindow,COLOR_PAIR(4));
         mvwprintw(cli.gameWindow,i+6,0," ");
         mvwprintw(cli.gameWindow,i+6,50," ");
    }

    wrefresh(cli.gameWindow);
    wrefresh(cli.golosmostra);
    cli.cmdui=cmdUI;
    for(i=0;i<MAXJOGADOR;i++)
    {
        jog[i].equipa=-1;
        jog[i].jogador=0;
        jog[i].posx=0;
        jog[i].posy=0;
        jog[i].cliente=NULL;
    }
    login(cmdUI,&cli);

    int continuaciclo=0;
    int breakjogo=0;
    cli.breakjogo=&breakjogo;
    cli.continuajogo=&continuaciclo;
    fieldthread enviar;

    enviar.cli=&cli;
    enviar.jog=jog;
    enviar.continuaciclo=&continuaciclo;
    enviar.meuboneco=&meuboneco;
    enviar.b=&b;
    enviar.mata=&saircliente;
    enviar.jo=&j;

	if (pthread_create(&cli.task,NULL,&footballField,(fieldthread *)&enviar))
	{
		endwin();
		perror("\nError: Thread Creation without success\n");
		exit(2);
	}


	cli.contin = 0;
    // int serveropen=open(SERVER_FIFO,O_WRONLY);
    char comando[6],equip,njogador;
    tipo a;
    do
    {
        if(saircliente!=0) break;
        do
        {
            echo();
            continuaciclo=0;
            breakjogo=0;
            saircliente=0;
            wprintw(cmdUI,"Escolhe Equipa e Jogador\npara Jogar.\n Ex: jogar 0 2\n> ");
            wrefresh(cmdUI);
            wscanw(cmdUI," %s %d %d",comando,&equip,&njogador);
            //wprintw(cmdUI,"comando %s %d %d\n",comando,equip,njogador);
            wrefresh(cmdUI);
            if(continuaciclo !=0 || saircliente != 0) break;
            if( !strcmp(comando,"jogar") && equip < 2 && equip >=0 && njogador > 0 && njogador <= MAXJOGADOR/2)
            {
                a.tipo=3;
                snprintf(a.data.jogadajog.cliFifo,sizeof(a.data.jogadajog.cliFifo),"%s",cur.cliFifo);
                a.data.jogadajog.equipa=equip;
                a.data.jogadajog.jogador=njogador;
                int sssa=write(cli.sfcli,&a,sizeof(a));
                wrefresh(cmdUI);
            }
            else if(!strcmp(comando,"sair") || saircliente!=0)
            {
                // enviar para o server sair e fechar threads e o programa
            //    printf("sair");
                continuaciclo=1;
                breakjogo=1;
                saircliente=2;
             //   a.tipo=7;
               // a.data.clientquit.quit=1;
                //sprintf(a.data.clientquit.clififo,"%s",cur.cliFifo);
                //write(serveropen,&a,sizeof(a));
            }
        } while(continuaciclo==0);
    if(saircliente != 0) break;

    int tecla;
    int pass;
    noecho();

    do
    {
        tecla=wgetch(cmdUI);
        //wprintw(cmdUI,"%d\n",saircliente);
        if(saircliente==2)
        {
            continuaciclo=1;
            breakjogo=1;
            //saircliente=2;
            break;
        }
        wrefresh(cmdUI);
        if((tecla==87 || tecla==119|| tecla==83 || tecla==115 || tecla==65 || tecla==97 ||
            tecla==68 || tecla==100 || tecla==81 || tecla==113 || tecla==101 || tecla==69 ||
            tecla==90 || tecla==122 || tecla==88 || tecla==120  ||tecla ==32 || tecla == 75 ||
            tecla ==107))
        {
            a.tipo=5;
            a.data.jogada.passe=0;
            a.data.jogada.jogada=-1;
            a.data.jogada.equipa=jog[meuboneco].equipa;
            a.data.jogada.jogador=jog[meuboneco].jogador;
           sprintf(a.data.jogada.cliFifo,"%s",cur.cliFifo);
            if(tecla == 87 || tecla == 119) // W
                a.data.jogada.jogada=1;
            else if(tecla==83 || tecla==115) // S
                a.data.jogada.jogada=0;
            else if(tecla==65 || tecla==97 ) //a
                a.data.jogada.jogada=2;
            else if(tecla==68 || tecla==100 ) // d
                a.data.jogada.jogada=3;
            else if(tecla==81 || tecla==113) //q
                a.data.jogada.jogada=4;
            else if(tecla==101|| tecla==69) // e
                a.data.jogada.jogada=5;
            else if(tecla==90 || tecla==122) // z
                a.data.jogada.jogada=6;
            else if(tecla==88 || tecla==120) // x
                a.data.jogada.jogada=7;
            else if(tecla==32)
            {
                a.data.jogada.jogada=8;
                pass=wgetch(cli.gameWindow);
                if(pass >=48 || pass <=57)
                {
                    pass-=48;
                   //wprintw(cmdUI,"%djogada passe%d\n ",a.data.jogada.jogada,pass);
                   //wrefresh(cmdUI);
                    a.data.jogada.passe=pass;
                }
            }
            else if(tecla == 75 || tecla ==107)
            {
                breakjogo=1;
                a.tipo=6;
                sprintf(a.data.sairjogo.clififo,"%s",cur.cliFifo);
                a.data.sairjogo.quitjogo=1;
            }
            //wprintw(cmdUI,"%d %dmeuboneco\n",a.data.jogada.jogada,meuboneco);
            //wrefresh(cmdUI);
            write(cli.sfcli,&a,sizeof(a));
        }
    } while(breakjogo==0);
    }
    while(saircliente==0);
        //wprintw(cmdUI,"aaa\n");
        //wrefresh(cmdUI);
    pthread_join(cli.task, &cli.retval);
    if(saircliente!=123)
    {
        a.tipo=7;
        a.data.clientquit.quit=1;
        sprintf(a.data.clientquit.clififo,"%s",cur.cliFifo);
        unlink(cur.cliFifo);
          wclear(cmdUI);
           wclear(cli.gameWindow);
    wclear(cli.golosmostra);

    wrefresh(cmdUI);
    wrefresh(cli.gameWindow);
    wrefresh(cli.golosmostra);

    endwin(); // never forget it
        write(cli.sfcli,&a,sizeof(a));
    close(cli.sfcli);
    close(cli.fdcli);
     exit(0);
    }
    else
    {
    unlink(cur.cliFifo);
    fflush(stdin);
    sleep(3);
    wrefresh(cmdUI);
    wrefresh(cli.gameWindow);
    wrefresh(cli.golosmostra);
    close(cli.sfcli);
    close(cli.fdcli);
    endwin(); // never forget it 
    exit(0);
    }
   


}

void *footballField(void * pdata)
{
    fieldthread *arg=pdata;
    jogador *jog=arg->jog;
    client *cli = arg->cli;

    int *ciclomain;
    int *meuboneco;

    meuboneco=arg->meuboneco;
    ciclomain=arg->continuaciclo;

    bola *b=arg->b;
    char fifo[20];

    snprintf(fifo,sizeof(fifo),"CF%d",getpid());
    tipo a;

    int i;
    int clientF;

    clientF=cli->fdcli;
    int *mata;
    jogo *j=arg->jo;
    mata=arg->mata;

    do
    {
        read(clientF,&a,sizeof(tipo));
        if(a.tipo == 2)
        {
            //wclear(cli->gameWindo w);
            if((b->posx == 0 || b->posx==50) && (b->posy > 5 && b->posy<15) &&b->equipa==0)
                wattron(cli->gameWindow,COLOR_PAIR(4));
            else wattron(cli->gameWindow,COLOR_PAIR(1));

            mvwprintw(cli->gameWindow, b->posy, b->posx," ");

            for(i=0;i<MAXJOGADOR;i++)
            {
                if((jog[i].posx == 0 || jog[i].posx==50  )&& jog[i].posy > 5 && jog[i].posy <15)    wattron(cli->gameWindow,COLOR_PAIR(4));
                else wattron(cli->gameWindow,COLOR_PAIR(2));
                mvwprintw(cli->gameWindow,jog[i].posy,jog[i].posx," ");
                jog[i]=a.data.atualizajogo.jogador[i];
                if(a.data.atualizajogo.jogador[i].equipa == -1) continue;
                else if((jog[i].posx == 0 || jog[i].posx == 50 ) && jog[i].posy > 5 && jog[i].posy<15)
                {
                    if(a.data.atualizajogo.jogador[i].equipa == 0)      wattron(cli->gameWindow,COLOR_PAIR(4));
                    else  wattron(cli->gameWindow,COLOR_PAIR(5));
                }
                else
                {
                    if(a.data.atualizajogo.jogador[i].equipa == 0)      wattron(cli->gameWindow,COLOR_PAIR(1));
                    else  wattron(cli->gameWindow,COLOR_PAIR(2));
                }
                mvwprintw(cli->gameWindow,a.data.atualizajogo.jogador[i].posy,a.data.atualizajogo.jogador[i].posx,"%d",a.data.atualizajogo.jogador[i].jogador);
            //wprintw(cli->gameWindow,"%dsdsdsdsds\n",a.data.jogador[i].jogador);
            }

            b->posx=a.data.atualizajogo.databola.posx;
            b->posy=a.data.atualizajogo.databola.posy;
            b->equipa=a.data.atualizajogo.databola.equipa;

            if((b->posx == 0 || b->posx== 50)  && b->posy > 5 && b->posy<15)
            {
                if((a.data.atualizajogo.databola.equipa) == 0)
                    wattron(cli->gameWindow,COLOR_PAIR(4));
                else  wattron(cli->gameWindow,COLOR_PAIR(5));
            }
            else
            {
                if((a.data.atualizajogo.databola.equipa) == 0)
                    wattron(cli->gameWindow,COLOR_PAIR(1));
                else  wattron(cli->gameWindow,COLOR_PAIR(2));
            }
            if(a.data.atualizajogo.databola.equipa != -1)mvwprintw(cli->gameWindow,a.data.atualizajogo.databola.posy,a.data.atualizajogo.databola.posx,"o");
            else
            {
                wprintw(cli->cmdui,"Jogo acabou,enter para continuar!\n");
                wrefresh(cli->cmdui);
                *(cli->breakjogo)=1;
                 *(cli->continuajogo)=1;
            }
            wrefresh(cli->gameWindow);
        }
        if(a.tipo==4)
        {
            if(a.data.resjogadas.res == 1)
            {
                *ciclomain=1;
                wprintw(cli->cmdui,"Conseguiste pegar num Boneco\n");
                wprintw(cli->cmdui,"Prima ENTER para Começar o Jogo\n");
                wprintw(cli->cmdui,"Clice 'K' para Sair do Jogo, Espaço para Rematar\nE wsad e qezx para controlar o boneco\n");
                // meter variavel de matar ciclo de espera
                for(i=0;i<MAXJOGADOR;i++)
                {
                    if(jog[i].equipa == a.data.resjogadas.equipa && jog[i].jogador == a.data.resjogadas.jogador) (*meuboneco)=i;
                }

            }
            else wprintw(cli->cmdui,"Não conseguiste pegar num boneco, tenta novamente\n");
            wrefresh(cli->cmdui);
        }

        if(a.tipo==8)
        {
            wclear(cli->golosmostra);
            j->resequipa0=a.data.atualizarres.resequip0;
            j->resequipa1=a.data.atualizarres.resequip1;
            wprintw(cli->golosmostra,"      Equipa 0    Equipa 1\nResultado    %d:%d",j->resequipa0,j->resequipa1);
            wrefresh(cli->golosmostra);
        }
        if(a.tipo == 9)
        {
            if(a.data.sairjogo.quitjogo == 2)
            {
            *(cli->breakjogo)=1; 
            wprintw(cli->cmdui,"Clica enter para sair do jogo, levou cartao vermelho!\n");
            wrefresh(cli->cmdui);
            }
        }

    } while(*mata==0);
     //Check Client account
  /*
    while(cli->contin == 1);
    {

        if(read(clientF,&act,sizeof(act)) == -1)
        {
            printf("Error receiving info from Server.\n");
            exit(1);
        }

        //cursor - variavel declarada pelo ncurses
		//getyx(stdscr, al,ac) //macro n�o � necess�rio &variables

		mvwprintw(cur->gameWindow, 2, 5, "%s", cur->contin);
		wrefresh(cur->gameWindow); // manda actualizar ecran/janela. � mesmo necess�rio
		usleep(300000);
        //move(cur->Y,cur->X);
        //wrefresh(cur->gameWindow);
    }
         * */
    pthread_exit(0);
}

int login(WINDOW *cmdUI,client *cli)
{
    char msg[2];
    int serverF,clientF;
    tipo type;
    snprintf(type.data.log.cliFifo,sizeof(type.data.log.cliFifo),"CF%d",getpid());
    //Client PID to server is able to identify client named pipe
    int logou=0;
    char debug[100];

    wprintw(cmdUI,"Welcome\n\nInsira info a baixo para conectar ao Server.\n");

    do
    {
        if(saircliente != 0) return -1;
        type.tipo=0;
        wprintw(cmdUI,"ID: ");
        wscanw(cmdUI," %20[^\n]",type.data.log.username);
        if(saircliente != 0) return -1;
        wrefresh(cmdUI);
        wprintw(cmdUI,"Password: ");
         cli->sfcli=open(SERVER_FIFO,O_WRONLY);
        noecho();
        wscanw(cmdUI," %20[^\n]",type.data.log.password);
        if(saircliente != 0) return -1;
        wrefresh(cmdUI);
        echo();
        wprintw(cmdUI,"\nConnecting to Server...\n\n");
        wrefresh(cmdUI);
        serverF = cli->sfcli;
        //sprintf(debug,"%s pass %s\n",type.data.log.username,type.data.log.password);
        //wprintw(cmdUI,debug);
        wrefresh(cmdUI);
        if(write(serverF,&type,sizeof(tipo)) == -1)
        {
            wprintw(cmdUI,"\nO Servidor não está disponível.\nTente mais Tarte.\n");
            //strerror(errno)
            wrefresh(cmdUI);
            continue;
        }
        clientF = open(type.data.log.cliFifo,O_RDONLY);
        cli->fdcli=clientF;
        if(read(clientF,&type,sizeof(tipo)) == -1 )
        {
            wprintw(cmdUI,"\nErro a Receber Login Resposta do Servidor.\n\nTry Again...\n\n");
            wrefresh(cmdUI);
            continue;
        }
        sprintf(debug,"Type:%d Result: %d\n",type.tipo,type.data.reslog.res);
        wprintw(cmdUI,debug);
        wrefresh(cmdUI);
        if(type.tipo == 1) logou = type.data.reslog.res;
        if(logou == 3)
        {
        wprintw(cmdUI,"Jogador ja logado!\n Nao e permitido logar um jogador com esse username\nClique nalguma tecla para sair\n");   
         wrefresh(cmdUI);
         wgetch(cmdUI);
                close(clientF);
                close(serverF);
                unlink(type.data.log.cliFifo);
                endwin();
                exit(1);
        }
        if( logou == 0 || logou == 3)
        {
            wprintw(cmdUI,"ID ou Password Incorreto!\nQuer sair do jogo?"
                    "\n1- Sim\n2-Nao\n> ");
            wscanw(cmdUI," %[^\n]",msg);
            wrefresh(cmdUI);
            if(msg[0] == '1')
            {
                wprintw(cmdUI,"\n Saiu com sucesso");
                wrefresh(cmdUI);
                close(clientF);
                close(serverF);
                unlink(type.data.log.cliFifo);
                endwin();
                exit(1);
            }
            else if(msg[0] == 0)
            {
                logou=0;
                continue;
            }
        }
        else if(logou == 1)
        {
            wprintw(cmdUI,"ID e Password corretos\n");
            wrefresh(cmdUI);
                //validate
        }
        else if(logou == 2)
        {
            wprintw(cmdUI,"Conta Inexistente! Criada nova Conta com Sucesso.\n");
            wrefresh(cmdUI);
        //validate
        }
    } while(logou == 0);

    //close(cli->sfcli);
    wprintw(cmdUI,"\nLigação Estabelecida com Sucesso através da sua Conta\n");
    wrefresh(cmdUI);
    return 1;
}

void fexarclient(int s,siginfo_t *info,void *context)
{
    //saircliente=2;
    if(info->si_int== 123)
        saircliente=info->si_int;
        wprintw(cmdUI,"\n\nO Servidor foi desligado. Todos os clientes ao disconectados\n ");
    wrefresh(cmdUI);
    echo();
}

void cfexar()
{

    saircliente = 2;
    wprintw(cmdUI,"\nClique uma Tecla para Sair\n ");
    wrefresh(cmdUI);
    clear();
   // endwin();


    //const int size = 20;
    //char buffer[size];
    //snprintf(buffer,size,"CF%d",getpid());

    //if(unlink(buffer))
    //    printf("\tErro a Remover o Named Pipe %s\n",buffer);
    //else
      //  printf("\tSaiu do Jogo com Sucesso.\n");
    // Avisar o Servidor que vai sair
 //   exit(2);
}
