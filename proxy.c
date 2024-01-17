#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "./simpleSocketAPI.h"


#define SERVADDR "127.0.0.1"        // Définition de l'adresse IP d'écoute
#define SERVPORT "0"                // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1                 // Taille de la file des demandes de connexion
#define MAXBUFFERLEN 1024           // Taille du tampon pour les échanges de données
#define MAXHOSTLEN 64               // Taille d'un nom de machine
#define MAXPORTLEN 64               // Taille d'un numéro de port

#define FTPPORT "21"                  // Port d'écoute par défaut du protocole FTP

//void fils();

int main(){
    int ecode;                       // Code retour des fonctions
    char serverAddr[MAXHOSTLEN];     // Adresse du serveur
    char serverPort[MAXPORTLEN];     // Port du server
    int descSockRDV;                 // Descripteur de socket de rendez-vous
    int descSockCOM;                 // Descripteur de socket de communication             
    struct addrinfo hints;           // Contrôle la fonction getaddrinfo
    struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
    struct sockaddr_storage from;    // Informations sur le client connecté
    socklen_t len;                   // Variable utilisée pour stocker les 
				                     // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur
    
    // Initialisation de la socket de RDV IPv4/TCP
    descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
    if (descSockRDV == -1) {
         perror("Erreur création socket RDV\n");
         exit(2);
    }
    // Publication de la socket au niveau du système
    // Assignation d'une adresse IP et un numéro de port
    // Mise à zéro de hints
    memset(&hints, 0, sizeof(hints));
    // Initialisation de hints
    hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par 
				                      // la fonction getaddrinfo

     // Récupération des informations du serveur
     ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
     if (ecode) {
         fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
         exit(1);
     }
     // Publication de la socket
     ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
     if (ecode == -1) {
         perror("Erreur liaison de la socket de RDV");
         exit(3);
     }
     // Nous n'avons plus besoin de cette liste chainée addrinfo
     freeaddrinfo(res);

     // Récuppération du nom de la machine et du numéro de port pour affichage à l'écran
     len=sizeof(struct sockaddr_storage);
     ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
     if (ecode == -1)
     {
         perror("SERVEUR: getsockname");
         exit(4);
     }
     ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
                         serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
     if (ecode != 0) {
             fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
             exit(4);
     }
     printf("L'adresse d'ecoute est: %s\n", serverAddr);
     printf("Le port d'ecoute est: %s\n", serverPort);

     // Definition de la taille du tampon contenant les demandes de connexion
     ecode = listen(descSockRDV, LISTENLEN);
     if (ecode == -1) {
         perror("Erreur initialisation buffer d'écoute");
         exit(5);
     }

	len = sizeof(struct sockaddr_storage);
     // Attente connexion du client
     // Lorsque demande de connexion, creation d'une socket de communication avec le client
     descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
     if (descSockCOM == -1){
         perror("Erreur accept\n");
         exit(6);
    }

    // Echange de données avec le client connecté

    /*****
     * Testez de mettre 220 devant BLABLABLA ...
     * **/

    buffer[MAXBUFFERLEN-1] = '\0';

    memset(buffer, 0, MAXBUFFERLEN);

    strcpy(buffer, "220 Identification user@nomserveur\r\n");
    write(descSockCOM, buffer, strlen(buffer));

    /**********************************************************************************************************/
    /**********************************************************************************************************/
    /**********************************************************************************************************/
/*
    pid_t pid;

    pid = fork();
    switch (pid)
    {
    case -1:
        perror("fils avorté");
        exit(1);

    case 0:
        fils(descSockCOM);
        exit(0);
    }

    close(descSockRDV);
*/
//}

    //void fils(int descSockCOM){

        //char buffer[MAXBUFFERLEN];
        //int ecode;

        read(descSockCOM, buffer, MAXBUFFERLEN-1);
        char infosConnexion[MAXBUFFERLEN];
        strcpy(infosConnexion, buffer);

        char user[50], nomServeur[50];

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //formatage user et nomServeur

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        sscanf(buffer, "%49[^@]@%49s", user, nomServeur);
        strncat(user, "\r\n", 49);

        printf("buffer : %s\nuser : %s\nnom serveur : %s\ninfos connexion : %s\n", buffer, user, nomServeur, infosConnexion);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //creation socket serveur

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int descSockSRV = 0;
        if (connect2Server(nomServeur, FTPPORT, &descSockSRV) != 0) {
            strcpy(buffer, "520 Connexion au serveur impossible\r\n");
            write(descSockCOM, buffer, strlen(buffer));
        }

        memset(buffer, 0, MAXBUFFERLEN);

        ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
        if (ecode < 0){
            perror("erreur lecture serveur\n");
            exit(42);
        }
        buffer[ecode] = '\0';

        memset(buffer, 0, MAXBUFFERLEN);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //envoie user

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ecode = write(descSockSRV, user, strlen(user));
        if (ecode < 0){
            perror("erreur à l'id\n");
            exit(42);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //lecture 331 besoin mdp

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
        if (ecode < 0){
            perror("erreur lecture serveur 2\n");
            exit(42);
        }
        buffer[ecode] = '\0';

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //demande user mdp
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecode = write(descSockCOM, buffer, strlen(buffer));
        if (ecode < 0){
            perror("erreur print id\n");
            exit(42);
        }

        memset(buffer, 0, MAXBUFFERLEN);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //ecriture mdp

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecode = read(descSockCOM, buffer, MAXBUFFERLEN-1);
        if (ecode < 1)
        {
            perror("erreur lecture 3\n");
            exit(42);
        }

        printf("mdp : %s", buffer);
        strcat(buffer,"\r\n");
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //envoie mdp serveur

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecode = write(descSockSRV, buffer, strlen(buffer));
        if (ecode < 0){
            perror("erreur mdp serveur\n");
            exit(42);
        }
        memset(buffer, 0, MAXBUFFERLEN);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //lecture confirmation
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
        if (ecode < 0){
            perror("erreur access\n");
            exit(42);
        }
        buffer[ecode]  = '\0';

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //ecriture access granted client

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecode = write(descSockCOM, buffer, strlen(buffer));
        if (ecode < 0){
            perror("erreur ecriture client\n");
            exit(42);
        }

        ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
        if (ecode < 0) {
            perror("erreur PORT");
            exit(42);
        }

        int ic1, ic2, ic3, ic4, pc1, pc2;
        sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &ic1, &ic2, &ic3, &ic4, &pc1, &pc2);

        memset(buffer, 0, MAXBUFFERLEN);
        
        char adresseClient[MAXBUFFERLEN];
        sprintf(adresseClient, "%d.%d.%d.%d", ic1, ic2, ic3, ic4);

        char portClient[MAXBUFFERLEN];
        sprintf(portClient, "%d", pc1*256 + pc2);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //connection avec le client

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int actif;
        ecode = connect2Server(adresseClient, portClient, &actif);

        if (ecode) {
            perror("erreur connexion data client");
            exit(42);
        }

        strcpy(buffer,"PASV\r\n");
        ecode = write(descSockSRV, buffer, 6);
        if (ecode < 0) {
            perror("erreur passive mode\n");
            exit(42);
        }

        ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
        if (ecode < 0) {
            perror("erreur PORT serveur");
            exit(42);
        }
        buffer[ecode]='\0';

        int is1, is2, is3, is4, ps1, ps2;
        sscanf(buffer, "%*[^(](%d,%d,%d,%d,%d,%d", &is1, &is2, &is3, &is4, &ps1, &ps2);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                //récup de l'adresse et du port du serveur

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        char adresseServeur[MAXBUFFERLEN];
        char portServeur[MAXBUFFERLEN];
        sprintf(adresseServeur, "%d.%d.%d.%d", is1, is2, is3, is4);
        sprintf(portServeur, "%d", ps1*256 + ps2);

        int passif;

        ecode = connect2Server(adresseServeur, portServeur, &passif);
        if (ecode) {
            perror("erreur connexion data serveur");
            exit(42);
        }

        write(descSockCOM, "200 PORT commande réussie\r\n", strlen("200 PORT commande réussie\r\n"));

        while (buffer != "QUIT "){
            int ecode = 0;
            ecode = read(descSockCOM, buffer, MAXBUFFERLEN-1);
            if (ecode <= 0) break;
            buffer[ecode] = '\0';
            
            ecode = write(descSockSRV, buffer, strlen(buffer));
            if (ecode <= 0) break;
            
            ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
            if (ecode <= 0) break;
            buffer[ecode] = '\0';
            
            ecode = write(descSockCOM, buffer, strlen(buffer));
            if (ecode <= 0) break;
        }

        do      // boucle afin de lire l'entierete du ls
        {

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    //lecture des donnees du serveur

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ecode = read(passif, buffer, MAXBUFFERLEN - 1);
            if (ecode == -1)
            {
                perror("probleme de lecture");
                exit(42);
            }
            buffer[ecode] = '\0';
            printf("%s", buffer);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    //envoie des donnees au client

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
            write(actif, buffer, strlen(buffer));
        } while (read(passif, buffer, MAXBUFFERLEN - 1) != 0);

        close(actif);
        close(passif);

        ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
        if (ecode < 0) {
            perror("erreur lecture confirmation");
            exit(42);
        }
        buffer[ecode] = '\0';

        write(descSockCOM, buffer, strlen(buffer));

        //fermeture connexion
        close(descSockRDV);
        close(descSockSRV);
        close(descSockCOM);
    }
    //anonymous@ftp.fau.de
    //todo@truc.com