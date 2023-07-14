
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

MYSQL *connectdb;
MYSQL_RES *raspuns;
MYSQL_RES *raspuns2;
MYSQL_ROW rowdb,rowdb2;
unsigned int dbport = 3306;
static char *unix_socket = NULL;
unsigned int flag = 0;
static char *host = "localhost";
static char *user = "root";
static char *pass = "password";
static char *dbname="topmusic";
//Variabile globale
int client_online=0;
char user_login[20];
int client_restricted=0;
//Functii pentru baza de date SQL
char * login_client(char * msg);
char * register_new_client(char *msg);
char * register_new_admin(char *msg);
char * add_song_to_top(char *msg);
char * vote_a_song(char *msg);
int show_top();
char * show_top_songs(int client);
int show_genre(char *msg);
char * show_top_genres(int client,char *msg);
char * comment_song(char *msg);
char * delete_song(char *msg);
char * restrict_an_user(char *msg);
char * download_song(char *msg);
char * download_video(char *msg);
char * play_song(char *msg);
int help();

char * help_center ="\nBun venit in Help Center! TopMusic va ofera urmatoarele posibilitati:\n login <login> <password> \n register_client <login> <password>\n register_admin <login> <password> \n add_song <name>@ <url>@ <description>@ <genres>\n vote_song <id>\n show_top \n show_genre_top <genre> \n comment_song <id> <comment> \n delete_song <id>\n restrict_client <login>\n download_song <id>\n download_video <id>\nplay_song\n exit\n Atentie!!!\n Inainte de a utiliza pentru prima data functiile download_song si download_video va trebui sa executati urmatoarele comenzi in TERMINAL:\n sudo wget https://yt-dl.org/downloads/latest/youtube-dl -O /usr/local/bin/youtube-dl \n sudo chmod a+rx /usr/local/bin/youtube-dl \n sudo apt-get install ffmpeg\n Inainte de a utiliza pentru prima data functia play_song va trebui sa executati urmatoarea comanda in TERMINAL: sudo apt install mplayer";

int main ()
{
    connectdb=mysql_init(NULL);
    if(!(mysql_real_connect(connectdb,host,user,pass,dbname,dbport,unix_socket,flag)))
    {
        fprintf(stderr,"\nError:%s[%d]\n",mysql_error(connectdb),mysql_errno(connectdb));
        exit(1);
    }
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    char msg[300];		//mesajul primit de la client
    char msgrasp[500]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }

    /* servim in mod concurent clientii... */
    while (1)
    {
    	int client;
    	int length = sizeof (from);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);

    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) {
    		close(client);
    		continue;
    	} else if (pid > 0) {
    		// parinte
    		close(client);
    		continue;
    	} else if (pid == 0) {
    		// copil
    		close(sd);
            while(1){
                /* s-a realizat conexiunea, se astepta mesajul */
                bzero (msg, 100);
                printf ("[server]Asteptam mesajul...\n");
                fflush (stdout);

                /* citirea mesajului */
                if (read (client, msg, 300) <= 0)
                {
                    perror ("[server]Eroare la read() de la client.\n");
                    close (client);	/* inchidem conexiunea cu clientul */
                    continue;		/* continuam sa ascultam */
                }

                printf ("[server]Mesajul a fost receptionat...%s\n", msg);

                /*pregatim mesajul de raspuns */
                bzero(msgrasp,900);
                if (strstr(msg,"help")!=NULL) strcpy(msgrasp,help_center); else 
                if (strstr(msg,"exit")!=NULL) break; else 
                if (strstr(msg,"login")!=NULL) strcpy(msgrasp,login_client(msg)); else
                if (strstr(msg,"register_client")!=NULL)strcpy(msgrasp,register_new_client(msg)); else 
                if (strstr(msg,"register_admin")!=NULL)strcpy(msgrasp,register_new_admin(msg)); else 
                if (strstr(msg,"vote_song")!=NULL)strcpy(msgrasp,vote_a_song(msg)); else 
                if (strstr(msg,"comment_song")!=NULL)strcpy(msgrasp,comment_song(msg)); else 
                if (strstr(msg,"delete_song")!=NULL)strcpy(msgrasp,delete_song(msg)); else 
                if (strstr(msg,"restrict_client")!=NULL)strcpy(msgrasp,restrict_an_user(msg)); else
                if (strstr(msg,"add_song")!=NULL)strcpy(msgrasp,add_song_to_top(msg)); else 
                if (strstr(msg,"download_song")!=NULL)strcpy(msgrasp,download_song(msg)); else 
                if (strstr(msg,"download_video")!=NULL)strcpy(msgrasp,download_video(msg)); else 
                if (strstr(msg,"play_song")!=NULL)strcpy(msgrasp,play_song(msg)); else 
                if (strstr(msg,"show_top")!=NULL){
                    int nr_songs=show_top();
                    if (client_online==0)nr_songs=0;
                    if (write (client, &nr_songs, 4) <= 0)
                    {
                        perror ("[server]Eroare la write() catre client.\n");
                        continue;		/* continuam sa ascultam */
                    }
                    else
                        printf ("[server]Mesajul a fost trasmis cu succes.\n");
                    strcpy(msgrasp,show_top_songs(client));
                } else
                if (strstr(msg,"show_genre_top")!=NULL){
                    int nr_songs=show_genre(msg);
                    if (client_online==0)nr_songs=0;
                    if (write (client, &nr_songs, 4) <= 0)
                    {
                        perror ("[server]Eroare la write() catre client.\n");
                        continue;		/* continuam sa ascultam */
                    }
                    else
                        printf ("[server]Mesajul a fost trasmis cu succes.\n");
                    strcpy(msgrasp,show_top_genres(client,msg));
                } else
                strcpy(msgrasp,"Comanda nu a fost gasita, incearca help pentru ajutor.");
                printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

                /* returnam mesajul clientului */
                if (write (client, msgrasp, 900) <= 0)
                {
                    perror ("[server]Eroare la write() catre client.\n");
                    continue;		/* continuam sa ascultam */
                }
                else
                    printf ("[server]Mesajul a fost trasmis cu succes.\n");
            }
    		/* am terminat cu acest client, inchidem conexiunea */
    		close (client);
    		exit(0);
    	}

    }				/* while */
}				/* main */

char * login_client(char *msg){
    if (client_online==0){
    int start_login=0;
    char login[20],password[20];
    while (msg[start_login]!=' ')start_login++;
    int start_password=++start_login;
    while (msg[start_password]!=' '){
       login[start_password-start_login]=msg[start_password];
       start_password++;
    }
    login[start_password-start_login]='\0';
    int end=++start_password;
    while (msg[end]!='\0'){
       password[end-start_password]=msg[end];
       end++;
    }
    password[strlen(password)-1]='\0';
    char sqlQuery[200] = "SELECT * FROM clients WHERE login = '";
    strcat(sqlQuery,login);
    strcat(sqlQuery,"'");
    mysql_query(connectdb,sqlQuery);
    raspuns=mysql_store_result(connectdb);
    rowdb=mysql_fetch_row(raspuns);
    if (rowdb == NULL) return "Login incorect"; else 
    {
        rowdb[1]+='\0';
        if (strcmp(rowdb[1],password)==0){
            if (strcmp(rowdb[2],"1")==0)
            client_online = 2; else client_online=1;
            if (strcmp(rowdb[3],"1")==0)client_restricted=1;
            strcpy(user_login,login);
            return "Te-ai logat cu succes!";
        } else return "Parola incorecta";
    }
    } else return "Sunteti deja conectat!";
}
char * register_new_client(char *msg){
    if (client_online==0){
    int start_login=0;
    msg+='\0';
    char login[20],password[20];
    while (msg[start_login]!=' ')start_login++;
    int start_password=++start_login;
    while (msg[start_password]!=' '){
       login[start_password-start_login]=msg[start_password];
       start_password++;
    }
    login[start_password-start_login]='\0';
    int end=++start_password;
    while (msg[end]!='\0'){
       password[end-start_password]=msg[end];
       end++;
    }
    password[end-start_password-1]='\0';
    printf("login *%s* *%s* \n",login,password);
    
    char sqlQuery[200] = "SELECT login,password FROM clients WHERE login = '";
    strcat(sqlQuery,login);
    strcat(sqlQuery,"'");
    mysql_query(connectdb,sqlQuery);
    raspuns=mysql_store_result(connectdb);
    rowdb=mysql_fetch_row(raspuns);
    if (rowdb == NULL) {
        char insert[200]= "INSERT INTO clients(login,password,admin) VALUES ('";
        strcat(insert,login);
        strcat(insert,"','");
        strcat(insert,password);
        strcat(insert,"',0)");
        printf("%s\n",insert);
        mysql_query(connectdb,insert);
        client_online=1;
        strcpy(user_login,login);
        return "Utilizator inregistrat cu succes";
    } else 
    return "Login deja folosit!";
    } else return "Sunteti deja conectat";
    
}
char * register_new_admin(char *msg){
    if (client_online==0){
    int start_login=0;
    msg+='\0';
    char login[20],password[20];
    while (msg[start_login]!=' ')start_login++;
    int start_password=++start_login;
    while (msg[start_password]!=' '){
       login[start_password-start_login]=msg[start_password];
       start_password++;
    }
    login[start_password-start_login]='\0';
    int end=++start_password;
    while (msg[end]!='\0'){
       password[end-start_password]=msg[end];
       end++;
    }
    password[end-start_password-1]='\0';
    printf("login *%s* *%s* \n",login,password);
    
    char sqlQuery[200] = "SELECT login,password FROM clients WHERE login = '";
    strcat(sqlQuery,login);
    strcat(sqlQuery,"'");
    mysql_query(connectdb,sqlQuery);
    raspuns=mysql_store_result(connectdb);
    rowdb=mysql_fetch_row(raspuns);
    if (rowdb == NULL) {
        char insert[200]= "INSERT INTO clients(login,password,admin) VALUES ('";
        strcat(insert,login);
        strcat(insert,"','");
        strcat(insert,password);
        strcat(insert,"',1)");
        printf("%s\n",insert);
        mysql_query(connectdb,insert);
        client_online=2;
        return "Utilizator admin inregistrat cu succes";
    } else 
    return "Login deja folosit!";
    } else return "Sunteti deja conectat";
    
}
char * comment_song(char * msg){
    if (client_online!=0){
        int start_id=0;
        char id_song[5],comment[200];
        while (msg[start_id]!=' ')start_id++;
        int start_comment=++start_id;
        while (msg[start_comment]!=' '){
        id_song[start_comment-start_id]=msg[start_comment];
        start_comment++;
        }
        id_song[start_comment-start_id]='\0';
        int end=++start_comment;
        while (msg[end]!='\0'){
        comment[end-start_comment]=msg[end];
        end++;
        }
        comment[end-start_comment]='\0';
        printf ("2: *%s* *%s*\n",id_song,comment);
        char sqlQuery[200] = "SELECT * FROM songs WHERE song_id = ";
        strcat(sqlQuery,id_song);
        mysql_query(connectdb,sqlQuery);
        raspuns=mysql_store_result(connectdb);
        rowdb=mysql_fetch_row(raspuns);
        if (rowdb != NULL) {
            char sqlQuery[200] = "INSERT INTO comments VALUES (' ";
            strcat(sqlQuery,id_song);
            strcat (sqlQuery,"','");
            strcat(sqlQuery,comment);
            strcat(sqlQuery,"','");
            strcat(sqlQuery,user_login);
            strcat(sqlQuery,"')");
            mysql_query(connectdb,sqlQuery);
            return "Comentariul a fost adaugat!";
        } else return "Nu exista asa id!";
    } else return "Trebuie sa fii logat!";
}
char * vote_a_song(char * msg){
    if (client_online!=0){
        if (client_restricted==0){
            int start_id=0;
            char id_song[5];
            while (msg[start_id]!=' ')start_id++;
            strcpy(id_song,msg+start_id+1);
            id_song[strlen(id_song)]='\0';
            char sqlQuery[200] = "SELECT * FROM songs WHERE song_id = ";
            strcat(sqlQuery,id_song);
            mysql_query(connectdb,sqlQuery);
            raspuns=mysql_store_result(connectdb);
            rowdb=mysql_fetch_row(raspuns);
            if (rowdb != NULL) {
                char sqlQuery[200] = "UPDATE songs SET votes=votes+1 WHERE song_id = ";
                strcat(sqlQuery,id_song);
                mysql_query(connectdb,sqlQuery);
                return "Votat!";
            } else return "Nu exista asa id!";
        } else return "Ai fost blocat de catre admin!";
    } else return "Trebuie sa fii logat!";
}
char * download_song(char * msg){
    if (client_online!=0){
        if (client_restricted==0){
            char url[100]="youtube-dl -x --audio-format mp3 ";
            char song[50]=" ";
            int start_id=0;
            char id_song[5];
            while (msg[start_id]!=' ')start_id++;
            strcpy(id_song,msg+start_id+1);
            id_song[strlen(id_song)]='\0';
            char sqlQuery[200] = "SELECT url FROM songs WHERE song_id = ";
            strcat(sqlQuery,id_song);
            mysql_query(connectdb,sqlQuery);
            raspuns=mysql_store_result(connectdb);
            rowdb=mysql_fetch_row(raspuns);
            if (rowdb != NULL) {
                strcat(url,rowdb[0]);
                if(system(url)<= 0)
                return "Melodia s-a descarcat in folderul home!";
                else return "Melodia nu s-a putut descarca!";
            } else return "Nu exista asa id!";
        } else return "Ai fost blocat de catre admin!";
    } else return "Trebuie sa fii logat!";
}
char * download_video(char * msg){
    if (client_online!=0){
        if (client_restricted==0){
            char url[100]="youtube-dl -f, --format mp4 ";
            char song[50]=" ";
            int start_id=0;
            char id_song[5];
            while (msg[start_id]!=' ')start_id++;
            strcpy(id_song,msg+start_id+1);
            id_song[strlen(id_song)]='\0';
            char sqlQuery[200] = "SELECT url FROM songs WHERE song_id = ";
            strcat(sqlQuery,id_song);
            mysql_query(connectdb,sqlQuery);
            raspuns=mysql_store_result(connectdb);
            rowdb=mysql_fetch_row(raspuns);
            if (rowdb != NULL) {
                strcat(url,rowdb[0]);
                if(system(url)<= 0)
                return "Videoclipul s-a descarcat in folderul home!";
                else return "Videoclipul nu s-a putut descarca!";
            } else return "Nu exista asa id!";
        } else return "Ai fost blocat de catre admin!";
    } else return "Trebuie sa fii logat!";
}
char * play_song(char * msg){
    if (client_online!=0){
        if (client_restricted==0){
            char song[50]=" ";
            char player[50]=" ";
            char song_name[50]="find /home/andrei/ -iname '";
            int start_id=0;
            char id_song[5];
            while (msg[start_id]!=' ')start_id++;
            strcpy(id_song,msg+start_id+1);
            id_song[strlen(id_song)]='\0';
            char sqlQuery[200] = "SELECT name FROM songs WHERE song_id = ";
            strcat(sqlQuery,id_song);
            mysql_query(connectdb,sqlQuery);
            raspuns=mysql_store_result(connectdb);
            rowdb=mysql_fetch_row(raspuns);
            if (rowdb != NULL) {
                strncat(song_name,rowdb[0],5);
                strcat(song_name,"*.mp3' | head -n 1 >> rezultat.txt ");
                FILE * file = fopen("rezultat.txt", "r");
                char * buffer = (char *)malloc(sizeof(char) * 1024);
                size_t bufferSize = 1024;
                while (getline(&buffer, &bufferSize, file) != -1);
                strcat(player,"mplayer ");
                strcat(player,buffer);
                system(player);
                fclose(file);
                
            } else return "Nu exista asa id!";
        } else return "Ai fost blocat de catre admin!";
    } else return "Trebuie sa fii logat!";
}

char * delete_song(char *msg){
    if (client_online==2){
        int start_id=0;
        char id_song[5];
        while (msg[start_id]!=' ')start_id++;
        strcpy(id_song,msg+start_id+1);
        id_song[strlen(id_song)]='\0';
        char sqlQuery[200] = "SELECT * FROM songs WHERE song_id = ";
        strcat(sqlQuery,id_song);
        mysql_query(connectdb,sqlQuery);
        raspuns=mysql_store_result(connectdb);
        rowdb=mysql_fetch_row(raspuns);
        if (rowdb != NULL) {
            char sqlQuery[200] = "DELETE FROM songs WHERE song_id = ";
            strcat(sqlQuery,id_song);
            mysql_query(connectdb,sqlQuery);
            return "Cantec sters!";
        } else return "Nu exista asa id!";
    } else return "Nu aveti permisiunea sa executati asa comanda!";
}

char * restrict_an_user(char *msg){
    if (client_online==2){
        int start_id=0;
        char id_client[20];
        while (msg[start_id]!=' ')start_id++;
        strcpy(id_client,msg+start_id+1);
        id_client[strlen(id_client)-1]='\0';
        printf("del: *%s*\n",id_client);
        char sqlQuery[200] = "SELECT * FROM clients WHERE login= '";
        strcat(sqlQuery,id_client);
        strcat(sqlQuery,"'");
        mysql_query(connectdb,sqlQuery);
        raspuns=mysql_store_result(connectdb);
        rowdb=mysql_fetch_row(raspuns);
        if (rowdb != NULL) {
            char sqlQuery[200] = "UPDATE clients SET restricted = 1 where login = '";
            strcat(sqlQuery,id_client);
            strcat(sqlQuery,"'");
            mysql_query(connectdb,sqlQuery);
            return "Utilizator restrictionat!";
        } else return "Nu exista asa client!";
    } else return "Nu aveti permisiunea sa executati asa comanda!";
}

char * add_song_to_top(char *msg){
    if (client_online!=0){
        int start_nume=0,start_url=0,start_descriere=0,start_genres=0;
        char name_song[50],url[100],descriere[200],genres[100];
        while (msg[start_nume]!=' ')start_nume++;
        start_nume+=1;
        start_url=start_nume;
        while (msg[start_url]!='@'){
            name_song[start_url-start_nume]=msg[start_url];
            start_url++;
        }
        name_song[start_url-start_nume]='\0';
        start_url++;
        start_descriere=++start_url;
        while (msg[start_descriere]!='@'){
            url[start_descriere-start_url]=msg[start_descriere];
            start_descriere++;
        }
        url[start_descriere-start_url]='\0';
        start_descriere++;
        start_genres=++start_descriere;
        while (msg[start_genres]!='@'){
            descriere[start_genres-start_descriere]=msg[start_genres];
            start_genres++;
        }
        descriere[start_genres-start_descriere]='\0';
        start_genres+=2;
        strcpy(genres,msg+start_genres);
        genres[strlen(genres)-1]='\0';
        printf(" *%s*\n *%s*\n *%s*\n *%s*\n",name_song,url,descriere,genres);
        char sqlQuery[200] = "INSERT INTO songs(name,url,description) VALUES('";
        strcat(sqlQuery,name_song);
        strcat(sqlQuery,"','");
        strcat(sqlQuery,url);
        strcat(sqlQuery,"','");
        strcat(sqlQuery,descriere);
        strcat(sqlQuery,"')");
        mysql_query(connectdb,sqlQuery);
        char sqlQuery1[200]="SELECT song_id FROM songs WHERE name = '";
        strcat(sqlQuery1,name_song);
        strcat(sqlQuery1,"'");
        mysql_query(connectdb,sqlQuery1);
        raspuns=mysql_store_result(connectdb);
        rowdb=mysql_fetch_row(raspuns);
        if (rowdb != NULL) {
            char genre[10][30];
            int nr_genres=0,end=0,start_genre=0;
            while (genres[end]!='\0')
            {
                while (genres[end]!=' ' && genres[end]!='\0')
                {
                    genre[nr_genres][end-start_genre]=genres[end];
                    end++;
                }
                genre[nr_genres][end-start_genre]='\0';
                if (genres[end]!='\0')start_genre=++end;
                printf("*%s*\n",genre[nr_genres]);
                nr_genres++;
            }
            for(int i=0;i<nr_genres;i++){
                char sqlQuery2[200] = "INSERT INTO genres VALUES(";
                strcat(sqlQuery2,rowdb[0]);
                strcat(sqlQuery2,",'");
                strcat(sqlQuery2,genre[i]);
                strcat(sqlQuery2,"')");
                mysql_query(connectdb,sqlQuery2);
            }
            return "Cantec adaugat!";
        } else return "Cantecul nu a putut fi adaugat!";
    } else return "Trebuie sa fii logat!";
}

int show_top(){
    char sqlQuery[200] = "SELECT COUNT(song_id) FROM songs";
    mysql_query(connectdb,sqlQuery);
    raspuns=mysql_store_result(connectdb);
    rowdb =mysql_fetch_row(raspuns);
    if (rowdb !=NULL) {
        int i=atoi(rowdb[0]);
        printf("%d\n",i);
        return i;
    }
    return 0;
}
char * show_top_songs(int client){
    if (client_online!=0){
        char sqlQuery[200] = "SELECT * FROM songs ORDER BY votes DESC";
        mysql_query(connectdb,sqlQuery);
        raspuns=mysql_store_result(connectdb);
        while(rowdb = mysql_fetch_row(raspuns)) {
            char song[500]= "ID: ";
            strcat(song,rowdb[0]);
            strcat(song,"\n Nume: ");
            strcat(song,rowdb[1]);
            strcat(song,"\n URL: ");
            strcat(song,rowdb[2]);
            strcat(song,"\n Voturi: ");
            strcat(song, rowdb[3]);
            strcat(song,"\n Descriere: ");
            strcat(song, rowdb[4]);
            strcat(song,"\n Gen: ");
            char sqlQuery1[200]="SELECT genre FROM genres WHERE song_id =";
            strcat(sqlQuery1,rowdb[0]);
            mysql_query(connectdb,sqlQuery1);
            raspuns2=mysql_store_result(connectdb);
            while(rowdb2 = mysql_fetch_row(raspuns2)){
                strcat(song,rowdb2[0]);
                strcat(song,",");
            }
            strcat(song,"\n Comentarii:");
            strcpy(sqlQuery1,"SELECT comment,login_client FROM comments WHERE song_id =");
            strcat(sqlQuery1,rowdb[0]);
            mysql_query(connectdb,sqlQuery1);
            raspuns2=mysql_store_result(connectdb);
            while(rowdb2 = mysql_fetch_row(raspuns2)){
                strcat(song,"\n");
                strcat(song,rowdb2[0]);
                strcat(song," - ");
                strcat(song,rowdb2[1]);
            }
            song[strlen(song)]='\n';
            printf("*%s*",song);
            if (write (client, song, 500) <= 0)
            {
                perror ("[server]Eroare la write() catre client.\n");
                continue;		/* continuam sa ascultam */
            }
        }
        return "Top Music a fost prezentat cu succes!";
    } else return "Trebuie sa fii logat!";
}
int show_genre(char * msg){
    int start_gen=0;
    char gen_song[20];
    while (msg[start_gen]!=' ')start_gen++;
    strcpy(gen_song,msg+start_gen+1);
    gen_song[strlen(gen_song)-1]='\0';
    printf("*%s*\n",gen_song);
    char sqlQuery[200] = "SELECT COUNT(song_id) FROM songs NATURAL JOIN genres WHERE genre LIKE '";
    strcat(sqlQuery,gen_song);
    strcat(sqlQuery,"'");
    mysql_query(connectdb,sqlQuery);
    raspuns=mysql_store_result(connectdb);
    rowdb =mysql_fetch_row(raspuns);
    if (rowdb !=NULL) {
        int i=atoi(rowdb[0]);
        printf("%d\n",i);
        return i;
    }
    return 0;
}
char * show_top_genres(int client,char *msg){
    if (client_online!=0){
        int start_gen=0;
        char gen_song[20];
        while (msg[start_gen]!=' ')start_gen++;
        strcpy(gen_song,msg+start_gen+1);
        gen_song[strlen(gen_song)-1]='\0';
        char sqlQuery[200] = "SELECT * FROM songs NATURAL JOIN genres WHERE genre like '";
        strcat(sqlQuery,gen_song);
        strcat(sqlQuery,"' ORDER BY votes DESC");
        mysql_query(connectdb,sqlQuery);
        raspuns=mysql_store_result(connectdb);
        while(rowdb = mysql_fetch_row(raspuns)) {
            char song[500]= "ID: ";
            strcat(song,rowdb[0]);
            strcat(song,"\n Nume: ");
            strcat(song,rowdb[1]);
            strcat(song,"\n URL: ");
            strcat(song,rowdb[2]);
            strcat(song,"\n Voturi: ");
            strcat(song, rowdb[3]);
            strcat(song,"\n Descriere: ");
            strcat(song, rowdb[4]);
            strcat(song,"\n Gen: ");
            strcat(song,rowdb[5]);
            song[strlen(song)]='\n';
            printf("*%s*",song);
            if (write (client, song, 500) <= 0)
            {
                perror ("[server]Eroare la write() catre client.\n");
                continue;		/* continuam sa ascultam */
            }
        }
        return "Top Music Genre a fost prezentat cu succes!";
    } else return "Trebuie sa fii logat!";
}
