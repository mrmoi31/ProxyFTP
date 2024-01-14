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

    strcpy(buffer, "220 Identification user@nomserveur\r\n");
    write(descSockCOM, buffer, strlen(buffer));

    /**********************************************************************************************************/
    /**********************************************************************************************************/
    /**********************************************************************************************************/

    read(descSockCOM, buffer, MAXBUFFERLEN-1);
    char infosConnexion[MAXBUFFERLEN];
    strcpy(infosConnexion, buffer);

    char* user = infosConnexion;
    char* nomServeur;

    // USER login@serveur\0

    while (*user != ' ') {
        user++;
    }
    user++;

    int longUser = 0;
    int longServ = 0;
    
    while (*(user + longUser) != '@'){
        longUser++;
    }
    
    *(user + longUser) = '\0';
    nomServeur = user + longUser + 1;

    while (*(nomServeur + longServ) != '\n'){
        longServ++;
    }
    *(nomServeur + longServ - 1) = '\0';

    // USER login\0serveur\0

    int descSockSRV = 0000;
    if (connect2Server(nomServeur, FTPPORT, &descSockSRV) != 0) {
        strcpy(buffer, "520 Connexion au serveur impossible\r\n");
        write(descSockCOM, buffer, strlen(buffer));
    }

    memset(buffer, 0, MAXBUFFERLEN);

    //ping serv
    /*
    strcpy(buffer,"PASV\r\n");
    ecode = write(descSockSRV, buffer, sizeof("PASV\r\n"));
    if (ecode < 0){
        perror("erreur ping\n");
        printf("erreur ping\n");
        exit(42);
    }*/
    ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
    if (ecode < 0){
        perror("erreur lecture serveur\n");
        printf("erreur lecture serveur\n");
        exit(42);
    }
    buffer[ecode] = '\0';
    //*(strchr(buffer, '\n') + 1) = '\0';
    
    //print welcome
    ecode = write(descSockCOM, buffer, strlen(buffer));
    if (ecode < 0){
        perror("erreur welcome\n");
        printf("erreur welcome\n");
        exit(42);
    }
    buffer[ecode] = '\0';
    //printf("%s\n", buffer);

    
    //envoie le login sur le srv
    strcat(infosConnexion, "\r\n");
    ecode = write(descSockSRV, infosConnexion, strlen(infosConnexion));
    if (ecode < 0){
        perror("erreur à l'id\n");
        printf("erreur id\n");
        exit(42);
    }

    // read 331 besoin mdp
    ecode = read(descSockSRV, buffer, MAXBUFFERLEN-1);
    if (ecode < 0){
        perror("erreur lecture serveur 2\n");
        printf("erreur lecture serveur 2\n");
        exit(42);
    }
    //printf("%s\n", infosConnexion);
    printf("%s\n", buffer);
    //*(strchr(buffer, '\n') + 1) = '\0';
    
    // demande user mdp
    ecode = write(descSockCOM, buffer, strlen(buffer));
    if (ecode < 0){
        perror("erreur print id\n");
        printf("erreur print id\n");
        exit(42);
    }

    ecode = read(descSockCOM, buffer, MAXBUFFERLEN-1);
    if (ecode < 1)
    {
        perror("erreur lecture 3\n");
        printf("erreur lecture 3\n");
        exit(42);
    }

    //printf("%s\n", buffer);
    
    /*
    ecode = write(descSockCOM, buffer, strlen(buffer));
    if (ecode < 1)
    {
        perror("erreur envoi\n");
        printf("erreur envoi\n");
        exit(42);
    }
    
    /*
    strcpy(buffer, "331 Mot de passe requis :\r\n");
    //printf("%s\n", buffer);
    */

    // envoi mdp sur serv
    ecode = write(descSockCOM, buffer, strlen(buffer));
    
    if (ecode < 0){
        perror("erreur mdp\n");
        printf("erreur mdp\n");
        exit(42);
    }
    /*
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN-1);
    if (ecode < 0){
        perror("erreur lecture serveur 3\n");
        printf("erreur lecture serveur 3\n");
        exit(42);
    }
    printf("%s\n", buffer);

    strcat(infosConnexion, buffer);
    strcat(infosConnexion, "\r\n");
    ecode = write(descSockSRV, infosConnexion, strlen(infosConnexion));
    if (ecode < 0){
        perror("erreur de connexion\n");
        printf("erreur de connexion\n");
        exit(42);
    }
    //printf("%s\n", buffer);
/*
    strcpy(buffer, strcat(user,"\r\n"));
    write(descSockCOM, buffer, strlen(buffer));

    //print besoin de mdp sur client 
    strcpy(buffer, "331 Mot de passe requis\r\n");
    write(descSockCOM, buffer, strlen(buffer));
    
    //lis mdp client 
    read(descSockCOM, buffer, MAXBUFFERLEN-1);


    //anonymous@ftp.fau.de
    //mdp : todo@truc.com 

    /*******
     * 
     * read descsockcom
     * pointeur nomlogin + pointeur nomserv
     * connect2server
     * envoi instructions client vers serveur et inverse
     * après connexion creer processus pour gerer plusieurs instances du client
     * 
     * *****/

    //Fermeture de la connexion
    close(descSockCOM);
    close(descSockRDV);
}