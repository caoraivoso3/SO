#include "ServerH.h"

/*Variável Global para terminar as threads dos BOTS
* As extern in ServerH.h
*/
int morre = 0, stopjogo=0;

int main(int argc, char *envp[])
{
    char filename[MAXFILENAME];
    FILE * f=NULL;

    if(!access(SERVER_FIFO,F_OK))
    {
        printf("Está outro servidor a Correr!\n\n");
        exit(1);
    }

    /*Create server Named Pipe*/
    mkfifo(SERVER_FIFO,0600);
    signal(SIGUSR1,answerSignal);
    signal(SIGINT,answerSignal);
    signal(SIGHUP,answerSignal);
    signal(SIGALRM,acabajogo);
    do
    {
        printf("Insira o Nome do Ficheiro de Texto Com IDs e Passwords:\n> ");
        fgets(filename,MAXFILENAME,stdin);
        if(morre!=0)
        {
            if(!unlink(SERVER_FIFO))
        printf("\n\tDesligou o Servidor com sucesso.\n");
    else
        printf("\n\tNão apagou o Named Pipe %s com sucesso\n",SERVER_FIFO);
 //   exit(2);
    exit(0);
        }
        filename[strcspn(filename,"\n")]='\0';
        f=fopen(filename,"r+");
    }

    while(f==NULL);

    int start = 0, pid = 0, nbrOnCli = 0,cmdReturn = -2;


    MaxCliRun clientsOn[MAXCLIENTS];

    int clisize=0;//nº de clientes ligados
     //cur[21];
    char cmd[MAXCMDLEN];

    pthread_t task; // thread inicial, le o pipe do servidor
    pthread_t encalhados;
    pthread_t bots[MAXJOGADOR]; // thread do bot
    pthread_t matajogo;
    int killthread[MAXJOGADOR]; // kill das threads do bot cada um tem a sua kill thread
    int killjogo=0;
    int botsize=0;// nºbots ligados
    jogador jogad[MAXJOGADOR];
    bola b; //bola que é importante

    srand((unsigned int)time(NULL));

    pthread_t timer[2]; //0 o de 3*n 1 4*n
    jogada arraydejogadas[MAXJOGADOR];
    int jogcont=0;


    int i;
    b.dele=NULL;
    b.posx=0;
    b.posy=0;
    b.dest=NULL;
    b.isremate=0;
    b.tipo=-1;
    b.destposx=0;
    b.destposy=0;

    for(i=0;i<MAXCLIENTS;i++)
    {
        clientsOn[i].Filecli=0;
        snprintf(clientsOn[i].cliFifo,sizeof(clientsOn[i].cliFifo),"NULL");
        snprintf(clientsOn[i].nome,sizeof(clientsOn[i].nome),"NULL");
        clientsOn[i].meu=NULL;
        clientsOn[i].equipaprimaria=-1;
    }

    for(i=0;i<MAXJOGADOR;i++)
    {
        arraydejogadas[i].equipa=jogad[i].equipa=-1;
        arraydejogadas[i].jogador=jogad[i].jogador=0;
        arraydejogadas[i].jogada=-1;
        arraydejogadas[i].passe=0;
        jogad[i].posx=0;
        jogad[i].posy=0;
        jogad[i].cliente=NULL;
        snprintf(arraydejogadas[i].cliFifo,sizeof(arraydejogadas[i].cliFifo),"NULL");
        killthread[i]=0;
    }

    jogo jog;
    jog.comeco=0;
    jog.resequipa0=0;
    jog.resequipa1=0;
    jog.temporizador=0;
    threadlog args;
    args.f=f;
    args.clisize=&clisize;
    args.cli=clientsOn;
    args.inic=&jog;
    args.jog=jogad;
    args.b=&b;
    args.arrayjogadas=arraydejogadas;
    args.bots=bots;
    args.killthread=killthread;
    pthread_create(&task,NULL,(void*)&fieldThread,(threadlog *)&args);
    pthread_create(&encalhados,NULL,(void*)&semclient,NULL);
    //printf("%d main\n",(clisize));
    do
    {
         if(morre!=0) break;
         printf(">");
         scanf(" %21[^\n]",cmd); // cmd -> adm input

         if(morre!=0) break;
         cmdReturn=cmdReturn = readCommand(cmd,&botsize,clientsOn,nbrOnCli,bots,jogad,&jogcont,&jog,f,&b,timer,arraydejogadas,killthread);
    }
     while(morre==0);

    for(i=0;i<MAXJOGADOR;i++)
    {
        if(jogad[i].equipa == -1 || jogad[i].cliente!=NULL)  continue;
        // printf("aaa\n");
        killthread[i]=1;
        pthread_join(bots[i], NULL);
    }

    pthread_join(timer[0], NULL);
    pthread_join(timer[1], NULL);
    union sigval val;
    val.sival_int=123;
    // printf("aqui\n");

    for(i=0;i<MAXCLIENTS;i++)
    {
        if(clientsOn[i].Filecli== 0) continue;
        sscanf(clientsOn[i].cliFifo,"CF%d",&pid);
        sigqueue(pid,SIGUSR1,val);
        //printf("pid:%d\n",pid);
    }
    for(i=0;i<MAXCLIENTS;i++)
    {
        if(clientsOn[i].Filecli== 0) continue;
        close(clientsOn[i].Filecli);
    }
    fclose(f);
    // printf("magia\n");
    if(!unlink(SERVER_FIFO))
        printf("\n\tDesligou o Servidor com sucesso.\n");
    else
        printf("\n\tNão apagou o Named Pipe %s com sucesso\n",SERVER_FIFO);
 //   exit(2);
    exit(0);
}




void shutDownServer()
{
    morre=20;
    stopjogo=1;

}
void answerSignal()
{
    shutDownServer();
    //printf("Insira um Caracter e prima ENTER para Sair\n");
    // Falta o restooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
    // Apenas para rapide Ctrl + z
printf("Escreva alguma coisa e prima enter para sair do servidor\n");
  //  exit(2);
}

void *fieldThread(void *args)
{
    threadlog *arg=args;
    FILE *f;
    f=arg->f;

    int *clisize;
    clisize=arg->clisize;
    MaxCliRun *clientsOn=arg->cli;
    jogador *jog= arg->jog;
    jogo * j= arg->inic;
    bola * b = arg->b;
    jogada *arrayjogada=arg->arrayjogadas;
    pthread_t *bots=arg->bots;
    int *killthread=arg->killthread;
    //printf("%dthread\n",*clisize);
    int fs=open(SERVER_FIFO,O_RDONLY);
    //printf("%dfifo\n",fs);
    threadbot bot;
    
    bot.array=jog;
    bot.arrayjogadas=arrayjogada;
    bot.b=b;
    bot.killthread=killthread;
    if(fs==-1)  pthread_exit(0);
    tipo a;
    int breaka=0;
    int threadout;
    do
    {

    read(fs,&a,sizeof(a));
    int i,l=0; //index de circulos
   //printf("bytenum returnado %d   Bugou: %s\n",bitenum,strerror(errno));
    //printf("akula %d\n",a.tipo);
    if(a.tipo == 0)
    {
        rewind(f);
        int fdcli=open(a.data.log.cliFifo,O_WRONLY);
        char user[MAXUSER];
        char pass[MAXPASS];
        int scan=0;
        a.tipo=1;
    do
    {
        scan=fscanf(f,"%s",user);
        if(scan != 1)
        {
            // chegou ao fim do ficheiro criar usar e password.

            fprintf(f,"%s ",a.data.log.username);
            fprintf(f,"%s ",a.data.log.password);
            fflush(f);
            a.data.reslog.res=2;
            clientsOn[*clisize].Filecli=fdcli;
            snprintf(clientsOn[*clisize].nome,sizeof(clientsOn[*clisize].nome),"%s",a.data.log.username);
            snprintf(clientsOn[*clisize].cliFifo,sizeof(clientsOn[*clisize].cliFifo),"%s",a.data.log.cliFifo);
            //printf("%d passou memoria %s \n",clientsOn[*clisize].Filecli,clientsOn[*clisize].cliFifo);
            (*clisize)++;
            write(fdcli,&a,sizeof(tipo));
                if(j->comeco == 1)
                {
                    a.tipo=2;
                    for(i=0;i<MAXJOGADOR;i++)
                    {
                        a.data.atualizajogo.jogador[i]=jog[i];
                    }
                    if(b->dele != NULL) a.data.atualizajogo.databola.equipa=b->dele->equipa;
                    else a.data.atualizajogo.databola.equipa=b->equipa;

                    a.data.atualizajogo.databola.jogador=-1;
                    a.data.atualizajogo.databola.posx=b->posx;
                    a.data.atualizajogo.databola.posy=b->posy;

                    write(fdcli,&a,sizeof(tipo));

                    a.tipo=8;
                    a.data.atualizarres.resequip1=j->resequipa1;
                    a.data.atualizarres.resequip0=j->resequipa0;

                    write(fdcli,&a,sizeof(tipo));
                }
            break;

        }
        else
        {
            if(!strcmp(user,a.data.log.username))
            {
                for(i=0;i<MAXCLIENTS;i++)
                {
                    if(!strcmp(user,clientsOn[i].nome))
                    {
                        a.data.reslog.res=3;   // Já conectado
                        write(fdcli,&a,sizeof(tipo));
                        break;
                        breaka=1;
                    }
                    if(breaka==1) break;
                }
                fscanf(f,"%s",pass);
                if(!strcmp(pass,a.data.log.password))
                {
                    // Conectou-se
                    a.data.reslog.res=1;
                    clientsOn[*clisize].Filecli=fdcli;
                    snprintf(clientsOn[*clisize].nome,sizeof(clientsOn[*clisize].nome),"%s",a.data.log.username);
                    snprintf(clientsOn[*clisize].cliFifo,sizeof(clientsOn[*clisize].cliFifo),"%s",a.data.log.cliFifo);
                    (*clisize)++;
                    write(fdcli,&a,sizeof(tipo));

                    if(j->comeco == 1)
                    {
                        a.tipo=2;
                        for(i=0;i<MAXJOGADOR;i++) a.data.atualizajogo.jogador[i]=jog[i];

                        if(b->dele != NULL) a.data.atualizajogo.databola.equipa=b->dele->equipa;
                        else a.data.atualizajogo.databola.equipa=b->equipa;

                        a.data.atualizajogo.databola.jogador=-1;
                        a.data.atualizajogo.databola.posx=b->posx;
                        a.data.atualizajogo.databola.posy=b->posy;

                        write(fdcli,&a,sizeof(tipo));

                        a.tipo=8;
                        a.data.atualizarres.resequip1=j->resequipa1;
                        a.data.atualizarres.resequip0=j->resequipa0;

                        write(fdcli,&a,sizeof(tipo));

                    }
                    break;
                }
                else
                {
                    // Pass Errada
                    a.data.reslog.res=0;
                    if(write(fdcli,&a,sizeof(tipo))  == -1)
                    {
                        printf("Bugou\n");
                    }
                   // printf("errou\n");
                    break;
                }
            }
            else
            {
                fscanf(f,"%s",user);
                continue;
            }
        }
    }
    while(1);
    }
        else if(a.tipo == 3)
        {
            a.tipo=4;
            int res=0;
            int equipa,jogador;
            if(j->comeco == 1)
            {
                for(i=0;i<MAXJOGADOR;i++)
                {
                    if(jog[i].cliente !=NULL)  continue;
                    if(a.data.jogadajog.equipa==jog[i].equipa && a.data.jogadajog.jogador == jog[i].jogador)
                    {
                        for(l=0;l<MAXCLIENTS;l++)
                        {
                           // printf("%d l=%d jog%d\n",clientsOn[l].equipaprimaria,l,jog[i].equipa);
                             if(strcmp(a.data.jogadajog.cliFifo,clientsOn[l].cliFifo)!=0 || clientsOn[l].meu!=NULL)
                                continue;
                             if(clientsOn[l].equipaprimaria == -1 || clientsOn[l].equipaprimaria == jog[i].equipa)
                             {
                                jog[i].cliente=&clientsOn[l];
                                killthread[i]=1;
                                clientsOn[l].meu=&jog[i];
                                clientsOn[l].equipaprimaria=jog[i].equipa;
                                res=1;
                                break;
                            }
                        }
                        equipa=jog[i].equipa;
                        jogador=jog[i].jogador;
                        break;
                    }
                }
            }
            a.data.resjogadas.res=res;
            a.data.resjogadas.equipa=equipa;
            a.data.resjogadas.jogador=jogador;
            //printf("%d\n",res);
            write(clientsOn[l].Filecli,&a,sizeof(a));
        }
        else if(a.tipo == 5)
        {
            int posdaarray=0,nposx=0,nposy=0,breaka;
            int bnposx=0,bnposy=0;
            for(i=0;i<MAXCLIENTS;i++)
            {
                breaka=0;
                if(clientsOn[i].meu == NULL) continue;
                if(strcmp(clientsOn[i].cliFifo,a.data.jogada.cliFifo)==0 && clientsOn[i].meu->equipa == a.data.jogada.equipa
                 && clientsOn[i].meu->jogador==a.data.jogada.jogador)
                {
                    if(a.data.jogada.jogada >=0 && a.data.jogada.jogada<=7)
                    {
                        getcoords(&nposx,&nposy,clientsOn[i].meu->posx,clientsOn[i].meu->posy,a.data.jogada.jogada);
                        if(clientsOn[i].meu == b->dele) getcoords(&bnposx,&bnposy,nposx,nposy,a.data.jogada.jogada);  //fixar a a bola nao sair limites
                        //if() ver se funciona como é claro
                        if(clientsOn[i].meu->tipo==0 && ((clientsOn[i].meu->equipa == 0 && nposx > 6) || (clientsOn[i].meu->equipa == 1 && nposx < 44)) || nposx <0 || nposy<0 || nposx>fieldX-1 ||nposy >fieldY-1)
                        {
                            breaka=1;
                            continue;
                        }
                        for(l=0;l<MAXJOGADOR;l++)
                        {
                            if((jog[l].posx == nposx &&  jog[l].posy == nposy) || nposx <0 || nposy<0 || nposx>fieldX-1 ||nposy >fieldY-1) breaka=1;
                            if(((bnposx != 0 || bnposy !=0) &&(jog[l].posx == bnposx &&  jog[l].posy == bnposy) )|| bnposx<0 || bnposy <0 || bnposx >fieldX-1 || bnposy>fieldY-1 ) breaka=1;
                            if(clientsOn[i].meu == &jog[l]) posdaarray=l;
                        }
                    }

                 else
                 {
                    if(a.data.jogada.passe < 0 || a.data.jogada.passe > 9 ||  (clientsOn[i].meu != b->dele)) breaka=1;
                 }
                    if(breaka==0)
                    {
                        for(l=0; l<MAXJOGADOR;l++)
                        {
                            if(jog[l].equipa==a.data.jogada.equipa && jog[l].jogador==a.data.jogada.jogador) break;
                        }
                        arrayjogada[l].jogada=a.data.jogada.jogada;
                        arrayjogada[l].passe=a.data.jogada.passe;
                        arrayjogada[l].equipa=a.data.jogada.equipa;
                        arrayjogada[l].jogador=a.data.jogada.jogador;
                        //printf("jogada %d arrayjogada %d",arrayjogada[l].jogada,arrayjogada[l].jogador);
                        for(i=0;i<MAXJOGADOR;i++)
                        {
                            if(jog[i].equipa==-1) continue;
                        }
                    }
                }
            }
        }
        else if(a.tipo==7)
        {
            if(a.data.clientquit.quit== 1)
            {
                for(i=0;i<MAXCLIENTS;i++)
                {
                    if(!strcmp(a.data.clientquit.clififo,clientsOn[i].cliFifo))
                    {
                        printf("apagatudo\n");
                    close(clientsOn[i].Filecli);
                    clientsOn[i].Filecli=0;
                    sprintf(clientsOn[i].cliFifo,"NULL");
                    clientsOn[i].equipaprimaria=-1;
                    sprintf(clientsOn[i].nome,"NULL");
                    (*clisize)--;
                        if(clientsOn[i].meu!=NULL)
                        {
                            clientsOn[i].meu->cliente=NULL;
                            for(l=0;l<MAXJOGADOR;l++)
                            {
                               if(clientsOn[i].meu == &jog[l]) break;
                            }
                            bot.minhapos=l;
                            clientsOn[i].meu=NULL;
                            if(jog[i].tipo==0) threadout=pthread_create(&bots[i],NULL,(void*)&guardaredes,(threadbot*)&bot);
                            else if(jog[i].tipo==1) threadout=pthread_create(&bots[l],NULL,(void*)&defesa,(threadbot*)&bot);
                            else if(jog[i].tipo==2) threadout=pthread_create(&bots[l],NULL,(void*)&avancado,(threadbot*)&bot);
                            if(threadout != 0)printf("Esta operacao deu erro!\n");
                        }
                        else
                        {
                        }
                    }
                }
            }
        }
        else if(a.tipo==6)
        {
           if(a.data.sairjogo.quitjogo== 1)
            {
                for(i=0;i<MAXCLIENTS;i++)
                {
                    if(!strcmp(a.data.sairjogo.clififo,clientsOn[i].cliFifo))
                    {
                        if(clientsOn[i].meu!=NULL)
                        {
                            clientsOn[i].meu->cliente=NULL;
                            for(l=0;l<MAXJOGADOR;l++)
                            {
                               if(clientsOn[i].meu == &jog[l]) break;
                            }
                            bot.minhapos=l;
                            clientsOn[i].meu=NULL;
                            // Cria threads para cada BOT ... 0 Guarda-Redes, 1 Defesa, 2 Avançado
                            if(jog[l].tipo==0)  threadout=pthread_create(&bots[l],NULL,(void*)&guardaredes,(threadbot*)&bot);
                            else if(jog[l].tipo==1) threadout=pthread_create(&bots[l],NULL,(void*)&defesa,(threadbot*)&bot);
                            else if(jog[l].tipo==2) threadout=pthread_create(&bots[l],NULL,(void*)&avancado,(threadbot*)&bot);
                            if(threadout != 0)printf("Esta operacao deu erro!\n");
                        }
                    }
                }
            }
        }
    //printf("morrenoddddfim%d\n",morre);
    } while(morre==0);

    //printf("morrenofim%d\n",morre);
    close(fs);
    pthread_exit(0);
}

void semclient()
{
    int writeserver=open(SERVER_FIFO,O_WRONLY); // para dar block quando fica sem clients
    printf("\n\tNão há clientes conectados\n");
    pthread_exit(0);
}
/*@parameters checktipo 0 -> 3 em 3
                        1 -> 4 em 4
*/
void checkfunc(void *args, int checktipo)
{
    int i,j,l;
    threadcheck *arg=args;
    MaxCliRun *clientsOn=arg->cli;
    jogador *jog= arg->array;
    jogada *jogadaarray=arg->arrayjogada;
    bola *b = arg->b;
    int bposx=0,bposy=0;
    int *botsize;
    botsize=arg->botsize;
    jogo * jg=arg->jg;
    int *killthread=arg->killthread;
    int r;
    tipo a;
    int nposx=0,nposy=0,avanca=0;
     //printf("%d idclicheck1\n",clientsOn[0].Filecli);
    do
    {
        for(i=0;i<MAXJOGADOR;i++)
        {
             if(jog[i].equipa== -1) continue;   
            if(checktipo == 0 && jog[i].tipo == 1) continue;
            if(checktipo == 1 && (jog[i].tipo == 0 ||jog[i].tipo == 2 )) continue;
            if(jogadaarray[i].jogada < 0 || jogadaarray[i].jogada > 8) continue;
            //printf("%djogada passe%d i=%d\n ",jogadaarray[i].jogada,jogadaarray[i].passe,i);
            if(jogadaarray[i].jogada < 8)
            {
            getcoords(&nposx,&nposy,jog[i].posx,jog[i].posy,jogadaarray[i].jogada);
            for(l=0;l<MAXJOGADOR;l++)
            {
            if(jog[l].equipa== -1) continue;
            if((nposx==jog[l].posx && nposy == jog[l].posy) || (nposx==b->posx && nposy == b->posy) )
            {
                avanca=1;
                break;
            }
            if(avanca==1)
            {
                avanca=0;
                jogadaarray[i].jogada=-1;
                continue;
            }
            }
            getcoords(&jog[i].posx,&jog[i].posy,jog[i].posx,jog[i].posy,jogadaarray[i].jogada);
            jog[i].ultimajogada=jogadaarray[i].jogada;
               if(&jog[i]== b->dele &&b->isremate==0)
               {
                   getcoords(&b->posx,&b->posy,jog[i].posx,jog[i].posy,jog[i].ultimajogada);
                        if((b->posx == 0 || b->posx==50) &&( b->posy > 5 && b->posy<15))
                        {
                        b->isremate=0;
                        b->dest=NULL;
                        b->destposx=0;
                        b->destposy=0;
                        b->equipa=0;
                        b->isremate=0;
                        b->tipo=0;
                            if(b->posx==0)
                            {
                                jg->resequipa1++;
                                b->dele=&jog[0];
                                b->equipa=b->dele->equipa;
                                getcoords(&b->posx,&b->posy,jog[0].posx,jog[0].posy,jog[0].ultimajogada);

                            }
                            else
                            {
                                jg->resequipa0++;
                                for(i=0;i<MAXJOGADOR;i++)
                                {
                                    if(jog[i].jogador==1 && jog[i].equipa==1)   break;
                                }
                                b->dele=&jog[i];
                                b->equipa=b->dele->equipa;
                                getcoords(&b->posx,&b->posy,jog[i].posx,jog[i].posy,jog[i].ultimajogada);
                            }
                               a.tipo=8;
                               a.data.atualizarres.resequip1=jg->resequipa1;
                               a.data.atualizarres.resequip0=jg->resequipa0;
                               for(i=0;i<MAXCLIENTS;i++)
                               {
                               if(clientsOn[i].Filecli==0) continue;
                               write(clientsOn[i].Filecli,&a,sizeof(tipo));
                               }
                        }
                   for(j=0;j<MAXJOGADOR;j++)
                   {
                       if(jog[j].posx==b->posx && jog[j].posy==b->posy)
                       {
                           b->dele=&jog[j];
                           b->equipa=b->dele->equipa;
                           getcoords(&b->posx,&b->posy,jog[j].posx,jog[j].posy,jog[j].ultimajogada);
                       break;
                       }

                   }
               }
               else
               {
                   if(!isbolaperto(b,jog,i)) continue;
                    b->dele=&jog[i];
                    b->equipa=b->dele->equipa;
                   getcoords(&b->posx,&b->posy,jog[i].posx,jog[i].posy,jog[i].ultimajogada);
                   b->isremate=0;
                   b->dest=NULL;
                   b->destposx=0;
                   b->destposy=0;
                   b->dele=&jog[i];
                   b->equipa=b->dele->equipa;
               }
            }
            else
            {
                r=rand()%100;
                b->isremate=1;
                b->tipo=b->dele->tipo;
                b->equipa=b->dele->equipa;
                b->dele=NULL;
                b->dest=NULL;
                //printf("%d\n",jogadaarray[i].passe);
                if(jogadaarray[i].passe==0)
                {
                    bposx=0;
                    bposy=6 + rand()%9;
                    if(jog[i].equipa == 0)
                    {
                    bposx=50;
                    }
                    else
                    {
                    bposx=0;
                    }

                        if((jog[i].tipo == 0 && r < 25)||(jog[i].tipo == 2 && r < 60) || (jog[i].tipo == 1 && r<80))
                        {
                            b->destposy=bposy;
                            b->destposx=bposx;
                        }
                        else
                        {
                           // printf("inicio\n");
                           b->destposx=rand()%fieldX;
                           b->destposy=rand()%fieldY;
                        }
                }
                else
                {

                    b->destposx=0;
                    b->destposy=0;
                    if(jog[i].tipo == 0||jog[i].tipo == 2 || jog[i].tipo == 1)
                    {
                    for(j=0;j<MAXJOGADOR;j++)
                    {
                        //printf("adhufehfewiheeiuh\n");
                        if(jog[j].jogador == jogadaarray[i].passe && jogadaarray[i].equipa== jog[j].equipa)
                        {
                            //printf("aquitoeu\n");
                            b->dest=&jog[j];
                            break;
                        }

                    }
                    }
                    else
                     {
                        //printf("ouaki %d i=%d\n",jog[i].tipo,i);
                           b->destposx=rand()%fieldX;
                           b->destposy=rand()%fieldY;
                     }
                }

            }
        }

        if(b->dele == NULL)
        {
            if(b->isremate == 1 && (b->tipo == 0 || b->tipo==2) && checktipo == 0 ) bolaanda(b,jog,checktipo,jg,clientsOn);
            if(b->isremate == 1 && (b->tipo == 1) && checktipo == 1 ) bolaanda(b,jog,checktipo,jg,clientsOn);
        }
        else
        {
          if(b->isremate == 1 && (b->dele->tipo == 0 || b->dele->posy==2) && checktipo == 0 )  bolaanda(b,jog,checktipo,jg,clientsOn);
          if(b->isremate == 1 &&  b->dele->posy==1 && checktipo == 1 )  bolaanda(b,jog,checktipo,jg,clientsOn);
        }
        a.tipo=2;
        for(i=0;i<MAXJOGADOR;i++)
        {
            jogadaarray[i].jogada=-1;
            jogadaarray[i].passe=-1;
            a.data.atualizajogo.jogador[i]=jog[i];
        }
      if(b->dele == NULL)  a.data.atualizajogo.databola.equipa=b->equipa;
      else a.data.atualizajogo.databola.equipa=b->dele->equipa;
        a.data.atualizajogo.databola.posx=b->posx;
        a.data.atualizajogo.databola.posy=b->posy;
        for(i=0;i<MAXCLIENTS;i++)
        {
            if(clientsOn[i].Filecli==0) continue;
        write(clientsOn[i].Filecli,&a,sizeof(a));
        }

    if(checktipo == 0) usleep(3*TEMPOJOGAR*10000);
    if(checktipo == 1) usleep(4*TEMPOJOGAR*10000);
    }
    while(morre == 0 && stopjogo == 0);
    if(stopjogo == 1 && checktipo == 0)
    {
        for(i=0;i<MAXJOGADOR;i++)
        {
            if(jog[i].cliente !=NULL) continue;
            killthread[i]=1;
        }
        printf("Jogo foi terminado com sucesso!\nResultado Equipa0: %d     Equipa1: %d\n",jg->resequipa0,jg->resequipa1);
        jg->comeco=0;
        jg->resequipa0=0;
        jg->resequipa1=0;
        jg->temporizador=0;
        jg->tamequipa=0;
        a.tipo=2;
        (*botsize)=0;
            for(i=0;i<MAXJOGADOR;i++)
            {
            jogadaarray[i].equipa=jog[i].equipa=-1;
            jogadaarray[i].jogador=jog[i].jogador=0;
            jogadaarray[i].jogada=-1;
            jogadaarray[i].passe=0;
            jog[i].posx=0;
            jog[i].posy=0;
            jog[i].cliente=NULL;
            snprintf(jogadaarray[i].cliFifo,sizeof(jogadaarray[i].cliFifo),"NULL");
             a.data.atualizajogo.jogador[i]=jog[i];
            }
            for(i=0;i<MAXCLIENTS;i++)
            {
                clientsOn[i].equipaprimaria=-1;
                clientsOn[i].meu=NULL;
            }
            b->dele=NULL;
            b->dest=NULL;
            b->destposx=0;
            b->destposy=0;
            b->equipa=-1;
            b->isremate=0;
            b->posx=0;
            b->posy=0;
            b->tipo=0;
            a.data.atualizajogo.databola.posx=b->posx;
            a.data.atualizajogo.databola.posy=b->posy;
            a.data.atualizajogo.databola.equipa=-1;
             for(i=0;i<MAXCLIENTS;i++)
            {
                if(clientsOn[i].Filecli==0) continue;
            write(clientsOn[i].Filecli,&a,sizeof(a));
            }
            
        stopjogo=0;
    }
    pthread_exit(0);
}

void *checkguardaavan(void *args) // 3em3
{
    checkfunc(args,0);
    pthread_exit(0);
}
void *checkdefesa(void *args) // 4em4
{
    checkfunc(args,1);
    pthread_exit(0);
}

void acabajogo()
{
    ///printf("Pois acaba\n");
    stopjogo=1;

}






void bolaanda(bola *b, jogador * jog, int checktipo,jogo *j,MaxCliRun *clientsOn)
{
    int arraybolax[8],arraybolay[8],i;
    float dist= fieldX*fieldY,dtemp,iteradormin=0;
    tipo a;
    getcoords8(arraybolax,arraybolay,b->posx,b->posy);
    if(b->dest != NULL)
    {
        for(i=0;i<8;i++)
        {
            dtemp=dist2Pontos(b->dest->posx,b->dest->posy,arraybolax[i],arraybolay[i]);
            if( dtemp < dist)
            {
                dist=dtemp;
                iteradormin=i;
            }
        }

    }
    else
    {
        for(i=0;i<8;i++)
        {
            dtemp=dist2Pontos(b->destposx,b->destposy,arraybolax[i],arraybolay[i]);
            if( dtemp < dist)
            {
                dist=dtemp;
                iteradormin=i;
            }
        }
    }
    getcoords(&b->posx,&b->posy,b->posx,b->posy,iteradormin);
    for(i=0;i<MAXJOGADOR;i++)
    {
        if(b->posx == jog[i].posx && b->posy == jog[i].posy)
        {
            b->dele=&jog[i];
            b->dest=NULL;
            b->destposx=0;
            b->destposy=0;
            b->equipa=0;
            b->isremate=0;
            b->tipo=0;
            getcoords(&b->posx,&b->posy,b->posx,b->posy,jog[i].ultimajogada);
            break;
        }
    }
    if( b->dele==NULL &(b->posx == b->destposx && b->posy == b->destposy))
    {
         b->isremate=0;
         b->dest=NULL;
         b->destposx=0;
         b->destposy=0;
         b->equipa=0;
         b->isremate=0;
         b->tipo=0;
         if((b->posx == 0 || b->posx==50) && b->posy > 5 && b->posy<15)
         {
             if(b->posx==0)
             {
                 j->resequipa1++;
                 b->dele=&jog[0];
                 getcoords(&b->posx,&b->posy,jog[0].posx,jog[0].posy,jog[0].ultimajogada);

             }
             else
             {
                 j->resequipa0++;
                 for(i=0;i<MAXJOGADOR;i++)
                 {
                     if(jog[i].jogador==1 && jog[i].equipa==1)   break;
                 }
                 b->dele=&jog[i];
                 getcoords(&b->posx,&b->posy,jog[i].posx,jog[i].posy,jog[i].ultimajogada);
             }
             if(b->posx <0 || b->posy <0 || b->posx>=fieldX || b->posy >=fieldY)
             {
                 b->posx=25;
                 b->posy=10;
             }
            a.tipo=8;
            a.data.atualizarres.resequip1=j->resequipa1;
            a.data.atualizarres.resequip0=j->resequipa0;
            for(i=0;i<MAXCLIENTS;i++)
            {
                if(clientsOn[i].Filecli==0) continue;
                write(clientsOn[i].Filecli,&a,sizeof(tipo));
            }
         }
    }
}

/*BOTS Threads*/
void *guardaredes(void *args)
{
    threadbot *arg=args;
    int pos=arg->minhapos;
    jogador * jog=arg->array;
    jogada * arrayjogadas=arg->arrayjogadas;
    bola *b=arg->b;
    int *killthread=arg->killthread;
    int myteam = jog[pos].equipa;
    int vizCordsX[8],vizCordsY[8],i;
    int dist, nearestPlayer;
            
       do
    {
           
        dist = fieldX*fieldY;
        getcoords8(vizCordsX,vizCordsY,jog[pos].posx,jog[pos].posy);
        //printf("aaa\n");
        // If bot as the ball
        if(b->dele == &jog[pos])
        {
            arrayjogadas[pos].passe = jogadorMaisPerto(jog,1,pos);
            arrayjogadas[pos].jogada = 8;
        }
        // bot doesn't have the ball

        else
        {
            //Go to a certain x coord
            if((jog[pos].posx > 4 &&  myteam == 0)  || (jog[pos].posx > 48 && myteam == 1) )
               arrayjogadas[pos].jogada = isposboa(jog,pos,b,2);
               // arrayjogadas[pos].jogada = 2;

            else if((jog[pos].posx < 4 &&  myteam == 0) || (jog[pos].posx < 48 && myteam == 1))
               arrayjogadas[pos].jogada = isposboa(jog,pos,b,3);
              //  arrayjogadas[pos].jogada = 3;

            else if((b->posy > jog[pos].posy)  )
                arrayjogadas[pos].jogada = isposboa(jog,pos,b,0);
               // arrayjogadas[pos].jogada = 0;
            else if((b->posy < jog[pos].posy)  )
                arrayjogadas[pos].jogada = isposboa(jog,pos,b,1);
              // arrayjogadas[pos].jogada = 1;
        }
        usleep(3*TEMPOJOGAR*10000);
    }
    while(killthread[pos]==0);

    killthread[pos]=0;
    pthread_exit(0);

}

void *defesa(void *args)
{

    threadbot *arg=args;
    int pos=arg->minhapos;

    jogador * jog=arg->array;
    bola *b=arg->b;
    int *killthread=arg->killthread;
    jogada *arrayjogadas=arg->arrayjogadas;

    // const int pos = arg->minhapos;
    int vizCordsX[8],vizCordsY[8],i;
    int dist, nearestPlayer;
    int nposx,nposy;
    do
    {
       // printf("defpos%d equipa:%d jogador %d\n",pos,jog[pos].equipa,jog[pos].jogador);
       // printf("%dkillthreadpos\n",killthread[pos]);
        //arrayjogadas[pos].jogada=3;
        arrayjogadas[pos].equipa=jog[pos].equipa;
        arrayjogadas[pos].jogador=jog[pos].jogador;
        //printf("%d - myteam",myteam );
        dist = fieldX*fieldY;
        getcoords8(vizCordsX,vizCordsY,arg->array->posx,arg->array->posy);
        //If bot as the ball
        if(b->dele == &jog[pos])
        {
            //If back bot is in the middle of football field, pass the ball to striker
            if((jog[pos].posx >= 25 && jog[pos].equipa == 0) ||
               (jog[pos].posx <= 25 && jog[pos].equipa == 1) )
            {
                arrayjogadas[pos].passe = jogadorMaisPerto(jog,2,pos);
                arrayjogadas[pos].jogada = 8;
            }
            //Oponentes distance in range is lower than 5
            else if(distOponenteMaisPerto(arg->array,arg->minhapos) < 5)
            {
                arrayjogadas[pos].passe = jogadorMaisPerto(arg->array,1,pos);
                arrayjogadas[pos].jogada = 8;
            }
            //Otherwise walk forward
            else
            {
                //0 Equipa da Esquerda -> Vai para Direita
                if(jog[pos].equipa == 0)
                {
                    if(rand() % 3) // 2/3 prob go in front
                        arrayjogadas[pos].jogada = 3;
                    else // 1/3 prob
                        (rand() % 2) ? (arrayjogadas[pos].jogada = isposboa(jog,pos,b,4))
                            : (arrayjogadas[pos].jogada = isposboa(jog,pos,b,5));
                }
                //1 Equipa da Direita -> Vai para Esquerda
                else if(jog[pos].equipa == 1)
                {
                     if(rand() % 3) // 2/3 prob go in front
                        arrayjogadas[pos].jogada = isposboa(jog,pos,b,4);
                    else // 1/3 prob
                        (rand() % 2) ? (arrayjogadas[pos].jogada = isposboa(jog,pos,b,6))
                            : (arrayjogadas[pos].jogada = isposboa(jog,pos,b,7));
                }
            }
        }
        //If bot doesn't has the ball
        else
        {
            //if my team has't the ball
            if(b->equipa != jog[pos].equipa)
            {
                // 0 is left team _ 1 is right team
                if(jog[pos].equipa == 0 && jog[pos].posx <= 25
                   || jog[pos].equipa == 1 && jog[pos].posx >= 25 )
                    arrayjogadas[pos].jogada = followCoords(jog[pos].posx,jog[pos].posy,
                        b->posx,b->posy,jog,pos);

                else if(jog[pos].equipa == 0 && jog[pos].posx >= 25)
                {
                   arrayjogadas[pos].jogada= isposboa(jog,pos,b,2);
                }
                    
                    

                else if(jog[pos].equipa  == 1 && jog[pos].posx <= 25)
                  arrayjogadas[pos].jogada=isposboa(jog,pos,b,3);
            }
            // my team as the ball
            else
            {
                if(jog[pos].posx >= 32 && jog[pos].equipa==1)
                {
                     arrayjogadas[pos].jogada= isposboa(jog,pos,b,2);
                }
                else if(jog[pos].posx <= 18 && jog[pos].equipa==0)
                {
                    arrayjogadas[pos].jogada= isposboa(jog,pos,b,3);
                }
            }
        }
        usleep(4*TEMPOJOGAR*10000);
    }
    while(killthread[pos]==0);

    killthread[pos]=0;
    pthread_exit(0);

}
void *avancado(void *args)
{
    threadbot *arg = (threadbot *) args;
    int pos=arg->minhapos;
    jogador * jog = arg->array;
    bola *b=arg->b;
    int myteam = jog[pos].equipa;
    int *killthread=arg->killthread;
    int dist, vizCordsX[8], vizCordsY[8];
    //printf("\tAvançado Before\n");
    do
    {

       // printf("\tAvancado:%d  Killthread:%d  Equipa:%d  Pos:%d\n",morre,killthread[pos],jog[pos].equipa,pos);
        //printf("%d - Avançado\n",myteam );
        dist = fieldX*fieldY;
        getcoords8(vizCordsX,vizCordsY,arg->array->posx,arg->array->posy);

        //If bot as the ball
        if(b->dele == &jog[pos])
        {
            //If back bot is in the middle of football field, pass the ball to striker
            if((arg->array[pos].posx > 42 && myteam == 0) ||
               (arg->array[pos].posx < 8 && myteam == 1) )
            {
                // Remate
                arg->arrayjogadas[pos].passe = 0;
                arg->arrayjogadas[pos].jogada = 8;
            }
            //Oponentes distance in range is lower than 5
            else if(distOponenteMaisPerto(arg->array,pos) < 2)
            {
                arg->arrayjogadas[pos].passe = jogadorMaisPerto(arg->array,2,pos);
                arg->arrayjogadas[pos].jogada = 8;
            }
            //Otherwise walk forward
            else
            {
                //0 Equipa da Esquerda -> Vai para Direita
                if(arg->array[pos].equipa == 0)
                {
                    if(rand() % 3) // 2/3 prob go in front
                        arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,3);
                    else if(arg->array[pos].posx <= 5)
                        arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,7);
                    else if(arg->array[pos].posx >= 16)
                        arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,5);
                    else // 1/3 prob
                        (rand() % 2) ? (arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,4))
                            : (arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,6));
                }
                //1 Equipa da Direita -> Vai para Esquerda
                else if(arg->array[pos].equipa == 1)
                {
                     if(rand() % 3) // 2/3 prob go in front
                        arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,2);
                    else if(arg->array[pos].posx <= 5)
                        arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,6);
                    else if(arg->array[pos].posx >= 16)
                        arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,4);
                    else // 1/3 prob
                        (rand() % 2) ? (arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,5))
                            : (arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,7));
                }
            }
        }
        //If bot doesn't has the ball
        else
        {
            //if my team has't the ball
            if(arg->b->equipa != myteam)
            {
                // 0 is left team _ 1 is right team
                if((myteam == 0 && arg->array[pos].posx >= 25) || (myteam == 1 && arg->array[pos].posx <= 25) )
                    arg->arrayjogadas[pos].jogada = followCoords(arg->array[pos].posx,arg->array[pos].posy,
                        arg->b->posx,arg->b->posy,arg->array,pos);
                else
                    !myteam ? (arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,3)) :
                        (arg->arrayjogadas[pos].jogada = isposboa(jog,pos,b,2));
            }
            // my team as the ball
            else
            {
                 // 0 is left team _ 1 is right team
                if((myteam == 0 && arg->array[pos].posx >= 25)
                   || (myteam == 1 && arg->array[pos].posx <= 25) )
                    arg->arrayjogadas[pos].jogada = followCoords(arg->array[pos].posx,arg->array[pos].posy,
                        arg->b->posx,arg->b->posy,arg->array,pos);
            }
        }
        usleep(3*TEMPOJOGAR*10000);
    } while(killthread[pos]==0);

    //printf("\tAvançado After\n");

    killthread[pos]=0;
    pthread_exit(0);
}

/*@return nearest team player*/
int jogadorMaisPerto(jogador *allJog, int type, int myPos)
{
    float nearestDist = fieldX *fieldY;
    int teamPlayer, playerID=0,i;
    // MAXJOGADOR -> deve ser o número de jogadores
            for(i=0;i<MAXJOGADOR;i++)
            {
            if(allJog[myPos].equipa == allJog[playerID].equipa && type == allJog[playerID].tipo && myPos != playerID
           && dist2Pontos(allJog[myPos].posx,allJog[myPos].posy,allJog[playerID].posx,allJog[playerID].posy) < nearestDist)
            nearestDist = dist2Pontos(allJog[myPos].posx,allJog[myPos].posy,allJog[playerID].posx,allJog[playerID].posy);
            playerID=i;
            }
    teamPlayer = allJog[playerID].jogador;

    return teamPlayer;
}
int isposboa(jogador *jog, int pos, bola *b, int dire) // retorna dir se pos for permitida caso contrario retorna -1
{
    int nposx,nposy,bposx,bposy,i;
    getcoords(&nposx,&nposy,jog[pos].posx,jog[pos].posy,dire);
    if(b->dele == &jog[pos])
    {
    getcoords(&bposx,&bposy,nposx,nposy,dire); 
    for(i=0;i<MAXJOGADOR;i++)
    {
        if(jog[i].equipa==-1) continue;
        if((jog[i].posx ==jog[pos].posx && jog[i].posy == jog[pos].posy) && i!=pos) return rand()%8;
        if((nposx ==jog[i].posx && nposy && jog[i].posy) || (bposx ==jog[i].posx && bposy && jog[i].posy) || bposy >fieldY-1 || bposx > fieldX-1 
                || bposx<0 || bposy<0 || nposx > fieldX-1 || nposy >fieldY-1 || nposx <0 || nposy <0 ) return -1;
    }
    }
    else
    {
     for(i=0;i<MAXJOGADOR;i++)
    {
        if(jog[i].equipa==-1) continue;
        if((jog[i].posx ==jog[pos].posx && jog[i].posy == jog[pos].posy) && i!=pos) return rand()%8;
        if(nposx ==jog[i].posx && nposy == jog[i].posy || nposx > fieldX-1 || nposy >fieldY-1 || nposx <0 || nposy <0) return -1;
    }   
    }
    return dire;
}
/*@return return jogada to follow ball*/
int followCoords(int posx, int posy, int targetX, int targetY, jogador *allJog, int myPos)
{
    int ret, vizCordsX[8],vizCordsY[8],i,flag = 0;

    if     (posx == targetX && targetY > posy)  ret = 0;
    else if(posx == targetX && targetY < posy)  ret = 1;
    else if(targetX < posx && targetY == posy)  ret = 2;
    else if(targetX > posx && targetY == posy)  ret = 3;
    else if(targetX < posx && targetY < posy)   ret = 4;
    else if(targetX > posx && targetY < posy)   ret = 5;
    else if(targetX < posx && targetY > posy)   ret = 6;
    else if(targetX > posx && targetY > posy)   ret = 7;

    //If coords go out of field, turn back
    getcoords8(vizCordsX,vizCordsY,allJog[myPos].posx,allJog[myPos].posy);
    if(vizCordsX[ret] >= fieldX) ret = 2;
    if(vizCordsY[ret] >= fieldY) ret = 1;
    if(vizCordsY[ret] <= 0) ret = 0;
    if(vizCordsX[ret] <= 0) ret = 3;


    // If theres a player in vizCoords or near out of field, dont move

    
    for(i = 0; i < MAXJOGADOR; i++)
        if((myPos != i) && (allJog[i].posx ==allJog[myPos].posx && allJog[i].posy == allJog[myPos].posy)) return rand()%8;
        if(myPos != i && ((allJog[i].posx == vizCordsX[ret] && allJog[i].posy == vizCordsY[ret])) )
           return -1;

    return ret;
}

/*@return nearest opponent distance*/
float distOponenteMaisPerto(jogador *allJog, int myPos)
{
    float nearestDist = fieldX *fieldY;
    int playerID,i;
    for(i = 0; i < MAXJOGADOR; i++)
    {
        playerID=i;
         if(allJog[playerID].equipa != -1 && allJog[myPos].equipa != allJog[playerID].equipa && myPos != playerID
           && dist2Pontos(allJog[myPos].posx,allJog[myPos].posy,allJog[playerID].posx,allJog[playerID].posy) < nearestDist)
        nearestDist = dist2Pontos(allJog[myPos].posx,allJog[myPos].posy,allJog[playerID].posx,allJog[playerID].posy);
    }
return nearestDist;
}

void getcoords(int *nposx,int *nposy, int x, int y, int tipojogada)
{
     if(tipojogada==0)
            {
                *nposy=y + 1;
                *nposx=x;
            }
            else if(tipojogada==1)
            {
                *nposy=y - 1;
                *nposx=x;
            }
            else if(tipojogada==2)
            {
                *nposy=y;
                *nposx=x - 1;
            }
            else if(tipojogada==3)
            {
                *nposy=y;
                *nposx=x + 1;
            }
            else if(tipojogada==4)
            {
                *nposy=y-1;
                *nposx=x - 1;
            }
            else if(tipojogada==5)
            {
                *nposy= y -1;
                *nposx=x + 1;
            }
            else if(tipojogada==6)
            {
                *nposy= y +1;
                *nposx=x - 1;
            }
            else if(tipojogada==7)
            {
                *nposy= y +1;
                *nposx=x + 1;
            }

}
void getcoords8(int *nposx,int *nposy, int x, int y)
{
                nposy[0]=y + 1;
                nposx[0]=x;
                nposy[1]=y - 1;
                nposx[1]=x;
                nposy[2]=y;
                nposx[2]=x - 1;
                nposy[3]=y;
                nposx[3]=x + 1;
                nposy[4]=y-1;
                nposx[4]=x - 1;
                nposy[5]= y -1;
                nposx[5]=x + 1;
                nposy[6]= y +1;
                nposx[6]=x - 1;
                nposy[7]= y +1;
                nposx[7]=x + 1;

}

int isbolaperto(bola *b,jogador *jog, int pos)
{
    if((( jog[pos].posx - b->posx )*( jog[pos].posx - b->posx ) + (jog[pos].posy - b->posy)*(jog[pos].posy - b->posy)) <= 2   ) return 1;
    else return 0;
}

float dist2Pontos(int x1,int y1,int x2,int y2)
{
    return sqrt(pow((x1 - x2),2) + pow((y1 - y2),2) );
}

int readCommand(char *cmd, int * botsize, MaxCliRun * clientsOn, int nbrOnCli,pthread_t *bots,
    jogador * jogad, int *jogcont, jogo * jog,FILE *f,bola *b,pthread_t *timer,
    jogada * arrayjogada,int *killthread)
{
    int i, n,l;
    FILE *InfoFile;
    FILE *TempFile;
    tipo a;
    char *commands[] = {"start","stop","users","user","result",
    "red","shutdown"}, *rest;
    char * token,* token1;
    int threadout;
    
    char userInfo[47],fileInfo[47], username[20],password[20];
    threadbot bot[MAXJOGADOR];
    threadcheck checkargs;
    if(strlen(cmd) > MAXCMDLEN)
    {
        printf("\tMax command length is 46 characters\n");
        return -1;
    }
    token=strtok(cmd," ");
         //printf("%s\n",token);
    for(n = 0; n < TOTALCOMMANDS; n++)
    {
        if(morre!=0) return 3;
          if(!strcmp(commands[1],token) && n==1)
          {
          char *paver;
            paver=strtok(NULL," ");
            if(paver!=NULL )
            {
                printf("\tComando é users, sem argumentos.\n");
                return 1;
            }
            if(jog->comeco==0)
            {
                printf("\t Comando é stop, sem argumentos.\n");
                return 1;
            }
            else
            {
            alarm(0);
            stopjogo=1;
            }
            return 1;
          }
        if(n == 0)
        {
            if(!strcmp(commands[0],token))
            {
                token1=strtok(NULL," ");
                if(token1 == NULL) break;
                   // printf("%s token1\n",token1);
                if(jog->comeco == 1)
                {
                    printf("\tJogo já foi iniciado!\n");
                    break;
                }
                else jog->comeco=1;

                jog->temporizador = atoi(token1);
                printf("\tNovo jogo começa...\n");

                iniciarJogo(botsize,clientsOn,nbrOnCli, bots,
                    jogad,jogcont,jog,f,b,timer,arrayjogada,killthread,bot,&checkargs);
                return 0;
            }
        }
        if(n == 3 && !strncmp(token,commands[3],strlen(commands[3]) + 1) )
        {
            char *username,*pass;

            username=strtok(NULL," ");
            pass=strtok(NULL," ");

            if(username==NULL || pass == NULL || strlen(username) > MAXUSER || strlen(pass) >MAXPASS)
            {
                printf("\tComando é:\n\t user [username] [password]\n");
                return 3;
            }
            char user[MAXUSER];

            fseek(f,0,SEEK_SET);

            while(fscanf(f,"%s",user) == 1)
            {
                if(!strcmp(username,user))
                {
                    printf("\tID [%s] existente, conta não criada.\n",user);
                    return 2;
                }
                fscanf(f,"%s",user);
            }

            fprintf(f,"%s %s ",username,pass);
            fflush(f);
            return 2;
        }
        // Jogadores Conectados
        if(n == 2 && !strncmp(token,commands[2],strlen(commands[2])) )
        {
            char *paver;
            paver=strtok(NULL," ");
            if(paver!=NULL )
            {
                printf("\tComando é users, sem argumentos.\n");
                break;
            }
            for(i=0;i<MAXCLIENTS;i++)
            {
                if(clientsOn[i].Filecli > 0)
                    printf("\t%s\n",clientsOn[i].nome);
                else if(!i)
                {
                    printf("\tNão há jogadores Conectados.\n");
                    break;
                }
                else
                    break;
            }
            return 3;
        }
        if(n==6 &&!strncmp(token,commands[6],strlen(commands[6])))
        {
            char *paver;
            paver=strtok(NULL," ");
            if(paver!=NULL )
            {
                printf("\tComando é shutdown, sem argumentos.\n");
                break;
            }
            printf("O servidor vai fechar!\n");
            shutDownServer();
            return 6;
        }
        if(n==4 &&!strncmp(token,commands[4],strlen(commands[4])))
        {
            char *paver;
            paver=strtok(NULL," ");
            if(paver!=NULL )
            {
                printf("\tComando é result, sem argumentos.\n");
                break;
            }
            if(jog->comeco==1)printf("Equipa 0: %d\nEquipa 1: %d\n",jog->resequipa0,jog->resequipa1);
            else printf("De momento nao ha nenhum jogo ativo.\n");
            
            return 6;
        }
        if(n==5 &&!strncmp(token,commands[5],strlen(commands[5])))
        {
            if(jog->comeco == 0)
            {
                printf("Para mandar cartoes vermelhos temos de ter jogo iniciado!\n");
                return 5;
            }
            char *username;
            username=strtok(NULL," ");
            for(i=0;i<MAXCLIENTS;i++)
            {
                if(clientsOn[i].meu==NULL) continue;
                if(!strcmp(clientsOn[i].nome,username))
                {
                    threadbot bot;
                   clientsOn[i].meu->cliente=NULL;
                            for(l=0;l<MAXJOGADOR;l++)
                            {
                               if(clientsOn[i].meu == &jogad[l]) break;
                            }
                            bot.minhapos=l;
                            clientsOn[i].meu=NULL;
                            a.tipo=9;
                            a.data.sairjogo.quitjogo=2;
                            snprintf(a.data.sairjogo.clififo,sizeof(a.data.sairjogo.clififo),"%s",clientsOn[i].cliFifo);
                            write(clientsOn[i].Filecli,&a,sizeof(a));
                            // Cria threads para cada BOT ... 0 Guarda-Redes, 1 Defesa, 2 Avançado
                            if(jogad[l].tipo==0)  threadout=pthread_create(&bots[l],NULL,(void*)&guardaredes,(threadbot*)&bot);
                            else if(jogad[l].tipo==1) threadout=pthread_create(&bots[l],NULL,(void*)&defesa,(threadbot*)&bot);
                            else if(jogad[l].tipo==2) threadout=pthread_create(&bots[l],NULL,(void*)&avancado,(threadbot*)&bot); 
                            if(threadout !=0)printf("Houve um erro de criacao de threads!\n");
                            return 5;
                }
            }
            printf("Jogador nao encontrado!\n");
            return 5;
        }
    }
    printf("Invalid Parameters\n");
    return 0;
}


/*start n*/
int iniciarJogo(int * botsize, MaxCliRun * clientsOn, int nbrOnCli,pthread_t *bots,
    jogador * jogad, int *jogcont, jogo * jog,FILE *f,bola *b,pthread_t *timer,
    jogada * arrayjogada,int *killthread,threadbot *bot,threadcheck *args)
{
    //printf("\nEntrou na função iniciarJogo.\n");
    int Ndefesa, Nataque, i;
    char * ndefesa, *nataque;
    int threadout;

    ndefesa=getenv("NDEFESAS");
    nataque=getenv("NAVANCADOS");

    if(ndefesa != NULL) Ndefesa = atoi(ndefesa);
    if(nataque != NULL) Nataque = atoi(nataque) ;

   // printf("%d\n",Ndefesa);
    int nequipa;

    //Set número total defesas e atacantes de cada equipa
    if(ndefesa == NULL || Ndefesa < 1 || (Ndefesa) > 4) Ndefesa = 2;
    if(nataque == NULL || Nataque < 1 || (Nataque) > 4) Nataque = 2;

    //printf("%d\n",Ndefesa);
    // Cria um array de Estruturas para lançar as threads dos BOTS
    //threadbot bot[MAXJOGADOR];
    // +1 é o Guarda-Redes
    jog->tamequipa=1+Ndefesa+Nataque;


    int equipa;
    int ultimajogada=0;

    //Inicializa as info sobre os jogadores a cada equipa
    for(equipa=0;equipa<2;equipa++)
    {
        nequipa = 1; // n dos jogadores
        if(equipa==0) jogad[*botsize].ultimajogada = 3;
        else  jogad[*botsize].ultimajogada=2;

        jogad[*botsize].jogador=nequipa;
        jogad[*botsize].equipa=equipa;
        jogad[*botsize].tipo=0;

        if(jogad[*botsize].equipa == 0)
        {
            jogad[*botsize].posx = 3;
            jogad[*botsize].posy = 10;
        }
        else
        {
            jogad[*botsize].posx = 48;
            jogad[*botsize].posy = 10;
        }
        //bot.minhapos=*botsize;
        (*botsize)++;
        nequipa++;
        for(i=0;i<(Ndefesa);i++)
        {
            if(equipa==0)jogad[*botsize].ultimajogada=3;
            else jogad[*botsize].ultimajogada=2;
            jogad[*botsize].equipa=equipa;
            jogad[*botsize].tipo=1;

            if( jogad[*botsize].equipa == 0)
            {
                jogad[*botsize].posx = 10;
                if(i == 0)  jogad[*botsize].posy = 3;
                else  jogad[*botsize].posy = jogad[(*botsize)-1].posy + 3;
            }
            else
            {
                 jogad[*botsize].posx = 41;
                 if(i == 0)  jogad[*botsize].posy = 3;
                 else  jogad[*botsize].posy = jogad[(*botsize)-1].posy + 3;
            }

            jogad[*botsize].jogador=nequipa;
//                    bot.minhapos=*botsize;
            (*botsize)++;
            nequipa++;
        }
        for(i=0;i<(Nataque);i++)
        {
            jogad[*botsize].equipa=equipa;
            jogad[*botsize].tipo=2;

            if(equipa==0)  jogad[*botsize].ultimajogada = 3;
            else  jogad[*botsize].ultimajogada = 2;

            if( jogad[*botsize].equipa == 0)
            {
                jogad[*botsize].posx=15;
                if(i == 0)  jogad[*botsize].posy = 3;
                else  jogad[*botsize].posy = jogad[(*botsize)-1].posy + 3;
            }
            else
            {
                 jogad[*botsize].posx=36;
                 if(i == 0)  jogad[*botsize].posy = 3;
                 else  jogad[*botsize].posy = jogad[(*botsize)-1].posy + 3;
            }

            jogad[*botsize].jogador=nequipa;
//                      bot.minhapos=*botsize;
            (*botsize)++;
            nequipa++;
        }
    }
    tipo a;
    a.tipo = 2;
    for(i = 0;i<MAXJOGADOR;i++)
    {
        if(jogad[i].equipa != -1)
        {
            bot[i].array=jogad;
            bot[i].b=b;
            bot[i].killthread=killthread;
            bot[i].arrayjogadas=arrayjogada;
            bot[i].minhapos=i;
           // printf("%d\n",arrayjogada[i].jogada);

            if(jogad[i].tipo == 0) threadout=pthread_create(&bots[i],NULL,(void*)&guardaredes,(threadbot*)&bot[i]);
            else if(jogad[i].tipo == 1) threadout=pthread_create(&bots[i],NULL,(void*)&defesa,(threadbot*)&bot[i]);
            else  if(jogad[i].tipo == 2) threadout=pthread_create(&bots[i],NULL,(void*)&avancado,(threadbot*)&bot[i]);
            if(threadout!=0) printf("Houve um problema com uma thread\n");
        }
        a.data.atualizajogo.jogador[i]=jogad[i];
    }
    int r=rand()%2;// atribui bola a uma equipa

    b->equipa=r;
    b->posx=25;
    b->posy=10;

    a.data.atualizajogo.databola.equipa=b->equipa;
    a.data.atualizajogo.databola.jogador=-1;
    a.data.atualizajogo.databola.posx=b->posx;
    a.data.atualizajogo.databola.posy=b->posy;

    for(i=0;i<MAXCLIENTS;i++)
    {
        //printf("%s\n",clientsOn[i].cliFifo);
        if(!strcmp(clientsOn[i].cliFifo,"NULL")) continue;
            if(!write(clientsOn[i].Filecli,&a,sizeof(tipo)));
                //printf("\nCliente %d Desconectou-se.\n",i);
    }

    //threadcheck args;
    args->array=jogad;
    args->arrayjogada=arrayjogada;
    args->cli=clientsOn;
    args->b=b;
    args->killthread=killthread;
    args->jg=jog;
    args->botsize=botsize;


    threadout=pthread_create(&timer[0],NULL,(void*)&checkguardaavan,(threadcheck*)args);
    threadout=pthread_create(&timer[1],NULL,(void*)&checkdefesa,(threadcheck *)args);
    if(threadout != 0) printf("Houve um problema com uma thread\n");

    alarm(jog->temporizador);

    a.tipo=8;
    a.data.atualizarres.resequip0=jog->resequipa0;
    a.data.atualizarres.resequip1=jog->resequipa1;

    for(i=0;i<MAXCLIENTS;i++)
    {
        if(!strcmp(clientsOn[i].cliFifo,"NULL")) continue;
            if(!write(clientsOn[i].Filecli,&a,sizeof(tipo)))
                printf("\nCliente %d Desconectou-se.\n",i);
    }
    return 0;
}


