/**
 * @zhang25_assignment1
 * @author  YUXIN ZHANG <zhang25@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/unistd.h>
#include <math.h>

#include "../include/global.h"
#include "../include/logger.h"

#define TRUE 1
#define FALSE 0
#define BACKLOG 5
#define STDIN 0
#define MSG_SIZE 256
#define CMD_SIZE 100
#define INPUT_SIZE 300
#define BUFFER_SIZE 256




struct cinfo {
    char name[MSG_SIZE+1];
    char ip[INET6_ADDRSTRLEN];
    int numsend;
    int numrecv;
    char port[5];
    int status;
    char block[3][INET6_ADDRSTRLEN];
    int blockflag[3];
    char message[100][MSG_SIZE];
    int mescount;
    int socket;
};      //client info stored in server

struct Clist{
    char name[MSG_SIZE+1];
    char ip[INET6_ADDRSTRLEN];
    char port[5];
};

void tokenize(char *command_str, char* cmdrest, char *input_cmd);
int getmyip(char *argv, struct addrinfo *res, char *myip);
int connect_to_host(char *server_ip, int server_port, int *flag);
char* itoa(int num);
void copystruc(struct cinfo *target, struct cinfo *copy);
void sortcinfo(struct cinfo *Cinfo, int num);



/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/

	if(argc != 3) {     //input check
		printf("Usage:%s [s/c] [port]\n", argv[0]);
		exit(-1);
	}

    /***************client part**********************/

	if(!(strcmp(argv[1], "c"))){

        /*initialization.create a structure to store my information*/
	    struct myInformation {
	        int portnum;
	        char myhostname[MSG_SIZE] ;  //store the name of my computer name
	        char myip[INET6_ADDRSTRLEN];
	        int server;
	    } myinformation ;

	    int loginflag=0;    //mark whether login
	    struct Clist clist[4];
	    int clistcount = 0;

        //create list for select
        int selret,head_socket,sock_index;
	    fd_set master_list, watch_list;

        /*initialization.port number*/
	    memset( &myinformation, '\0' , sizeof myinformation);
	    myinformation.portnum = (int) strtol(argv[2],NULL,10);       //store port number into myinformation

        /*initialization.get my host name*/
        //myinformation.myhostname = (char*) malloc(sizeof(char)*MSG_SIZE);  //store the name of my computer name
        memset( myinformation.myhostname, '\0', MSG_SIZE);
        gethostname( myinformation.myhostname, MSG_SIZE );

        /*initialization.get my ip*/
        struct addrinfo *myipaddr;      //this is a pointer to the struct, not need to initial by memset
        getmyip( myinformation.myhostname , myipaddr, myinformation.myip );

        /*initialization.serverfd*/
        myinformation.server = -1;

        /* Zero select FD sets */
        FD_ZERO(&master_list);
        FD_ZERO(&watch_list);

        /* Register STDIN */
        FD_SET(STDIN, &master_list);

        head_socket = 0;


        while(TRUE){
            printf("\n[PA1-Client@CSE489/589]$ ");
            fflush(stdout);

            memcpy(&watch_list, &master_list, sizeof(master_list));

            /* select() system call. This will BLOCK */
            selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
            if(selret < 0)
                perror("select failed.");

            /* Check if we have sockets/STDIN to process */
            if(selret > 0){
                for(sock_index=0; sock_index<=head_socket; sock_index+=1){

                    if(FD_ISSET(sock_index, &watch_list)){
                            if (sock_index == STDIN){

            /*command input section*/
            char *input_cmd = (char*) malloc(sizeof(char)*INPUT_SIZE);  //store the input command in to input_cmd
            memset(input_cmd, '\0', INPUT_SIZE);
            if(fgets(input_cmd, INPUT_SIZE-1, stdin) == NULL)       //copy the input into input_cmd, if no input, exit
                exit(-1);

            printf("I got: %s (size:%d chars)\n", input_cmd, strlen(input_cmd));

            /*command analysis section*/
            char *command_str = (char*) malloc(sizeof(char)*MSG_SIZE);      //have two string store command and the rest information
            memset(command_str, '\0', MSG_SIZE);
            char *cmdrest = (char*) malloc(sizeof(char)*INPUT_SIZE);
            memset(cmdrest, '\0', INPUT_SIZE);
            tokenize( command_str, cmdrest, input_cmd );        //store the command section into command_str, the rest, if there is , stores in cmdrest

            /*command recognition*/
            if(!(strcmp(command_str, "AUTHOR"))){                           //COMMAND AUTHOR
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "zhang25");
                    cse4589_print_and_log("[%s:END]\n",command_str);
            }
            else if(!(strcmp(command_str, "IP"))){                        //COMMAND IP
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("IP:%s\n", myinformation.myip);
                    cse4589_print_and_log("[%s:END]\n",command_str);
            }
            else if(!(strcmp(command_str, "PORT"))){                        //COMMAND PORT
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("PORT:%d\n", myinformation.portnum);
                    cse4589_print_and_log("[%s:END]\n",command_str);
            }
            /***************************LOGIN****************************/
            else if(!(strcmp(command_str, "LOGIN"))){                        //COMMAND LOGIN


                    char *serverip = (char*) malloc(sizeof(char)*INET6_ADDRSTRLEN);      //have two string store command and the rest information
                    memset(serverip, '\0', INET6_ADDRSTRLEN);
                    char *serverport = (char*) malloc(sizeof(char)*INET6_ADDRSTRLEN);      //have two string store command and the rest information
                    memset(serverport, '\0', MSG_SIZE);
                    tokenize( serverip, serverport, cmdrest );
                    //
                    //printf("serverip : %s ; serverport : %s \n",serverip,serverport);
                    int server, connectflag=1;
                    server = connect_to_host(serverip, atoi(serverport),&connectflag);
                    FD_SET(server, &master_list);
                    if(head_socket<server)   head_socket = server;

                    if(connectflag!=1){
                        cse4589_print_and_log("[%s:ERROR]\n",command_str);
                        cse4589_print_and_log("[%s:END]\n",command_str);
                    }
                    else{
                    myinformation.server = server;

                    char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
                    memset(msg, '\0', MSG_SIZE);
                    strcpy(msg, "LOGIN");
                    strcat(msg, " ");
                    strcat(msg,myinformation.myhostname);
                    perror("int server1: ");
                    strcat(msg, " ");
                    strcat(msg, myinformation.myip );
                    perror("int server2: ");
                    strcat(msg, " ");
                    //char *token0 = (char*) malloc(sizeof(char)*MSG_SIZE);
                    strcat(msg,itoa(myinformation.portnum));
                    perror("int server5: ");


                    //printf("I got: %s(size:%d chars)", msg, strlen(msg));

                    //printf("\nSENDing it to the remote server ... ");
                    if(send(server, msg, strlen(msg), 0) == strlen(msg) )
                        //printf("Done!\n");
                    fflush(stdout);

                    /* Initialize buffer to receieve response */
                    char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                    memset(buffer, '\0', BUFFER_SIZE);
                    int recvsize = 0;
                    char *recvtoken1 = (char*) malloc(sizeof(char)*MSG_SIZE);
                    char *recvtoken2 = (char*) malloc(sizeof(char)*MSG_SIZE);

                    /*reset clistcount for receiving a new list*/
                    clistcount = 0;

                    while(1){
                        if(recvsize = recv(server, buffer, BUFFER_SIZE, 0) >= 0){
                            //printf("Server responded: %s \n", buffer);
                            fflush(stdout);
                            }
                        tokenize(recvtoken1, recvtoken2, buffer);
                        if(strcmp(recvtoken1, "FINISH")){
                           if(!(strcmp(recvtoken1, "LIST"))){           //get the information is list, stores it
                                //copy list info into data structure
                                tokenize(clist[clistcount].name, recvtoken1, recvtoken2);
                                tokenize(clist[clistcount].ip, recvtoken2, recvtoken1);
                                tokenize(clist[clistcount].port, recvtoken1, recvtoken2);
                                clistcount += 1;
                                if(send(server, "LIST", strlen("LIST"), 0) == strlen("LIST") )
                                //printf("LIST DONE %d!\n",clistcount);
                                fflush(stdout);
                           }
                           else if (!(strcmp(recvtoken1, "MSG"))){          //get the information is message, display it
                                command_str = "RECEIVED";
                                char clientip[INET6_ADDRSTRLEN];
                                tokenize(clientip, msg, recvtoken2);
                                cse4589_print_and_log("[%s:SUCCESS]\n", command_str);
                                cse4589_print_and_log("msg from:%s\n[msg]:%s\n", clientip, msg);
                                cse4589_print_and_log("[%s:END]\n", command_str);
                                if(send(server, itoa(recvsize), strlen(itoa(recvsize)), 0) == strlen(itoa(recvsize)) )
                                //printf("MSG DONE!\n");
                                fflush(stdout);
                           }
                        }
                        else{
                            break;
                        }
                    }
                    command_str = "LOGIN";
                    //clistcount = 0;     //reset clistcount for later use //used to indicate how many data in the clist
                    loginflag = 1;      //set login flag for enable other command
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("[%s:END]\n",command_str);
                    }

            }
            /***************************LIST****************************/

            else if((!(strcmp(command_str, "LIST")))&&(loginflag==1)){
                /*display the list*/
                cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                for(int i = 0; i < clistcount ; i++){
                    cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i+1, clist[i].name, clist[i].ip, atoi(clist[i].port));
                }
                cse4589_print_and_log("[%s:END]\n",command_str);

            }
            /***************************REFRESH****************************/
            else if((!(strcmp(command_str, "REFRESH")))&&(loginflag==1)){
                char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
                memset(msg, '\0', MSG_SIZE);
                strcpy(msg, "REFRESH");
                strcat(msg, " ");
                strcat(msg, "123123");
                if(send(myinformation.server, msg, strlen(msg), 0) == strlen(msg) )     //send request to get list
                        //printf("Done!\n");
                fflush(stdout);

                        /* Initialize buffer to receieve LIST */
                char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                memset(buffer, '\0', BUFFER_SIZE);
                int recvsize = 0;
                char *recvtoken1 = (char*) malloc(sizeof(char)*MSG_SIZE);
                char *recvtoken2 = (char*) malloc(sizeof(char)*MSG_SIZE);

                /*reset clistcount for receiving a new list*/
                clistcount = 0;

                while(1){
                    if(recvsize = recv(myinformation.server, buffer, BUFFER_SIZE, 0) >= 0){
                        printf("Server responded: %s \n", buffer);
                        fflush(stdout);
                        }
                    tokenize(recvtoken1, recvtoken2, buffer);
                    if(strcmp(recvtoken1, "FINISH")){
                       if(!(strcmp(recvtoken1, "LIST"))){           //get the information is list, stores it
                            //copy list info into data structure
                            tokenize(clist[clistcount].name, recvtoken1, recvtoken2);
                            tokenize(clist[clistcount].ip, recvtoken2, recvtoken1);
                            tokenize(clist[clistcount].port, recvtoken1, recvtoken2);
                            clistcount += 1;
                            if(send(myinformation.server, "LIST", strlen("LIST"), 0) == strlen("LIST") )
                            //printf("LIST DONE %d!\n",clistcount);
                            fflush(stdout);
                       }
                    }
                    else{
                        break;
                    }
                }
                cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                cse4589_print_and_log("[%s:END]\n",command_str);

            }
            /***************************SEND****************************/
            /*************************************************************************************************************************************/
            /*************************************************************************************************************************************/
            /*************************************************************************************************************************************/
            else if(!(strcmp(command_str, "SEND"))){                        //COMMAND SEND
                    char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
                    memset(msg, '\0', MSG_SIZE);
                    strcpy(msg, "SEND");
                    strcat(msg, " ");
                    strcat(msg, myinformation.myip );
                    strcat(msg, " ");
                    strcat(msg, cmdrest);
                    strcat(msg, " ");
                    //strcat(msg, "SEND");

                    if(send(myinformation.server, msg, strlen(msg), 0) == strlen(msg) )          //send request to get list
                            //printf("Done!\n");
                    fflush(stdout);

                    /* Initialize buffer to receieve msg */
                    char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                    memset(buffer, '\0', BUFFER_SIZE);
                    int recvsize = 0;
                    char *recvtoken1 = (char*) malloc(sizeof(char)*MSG_SIZE);
                    char *recvtoken2 = (char*) malloc(sizeof(char)*MSG_SIZE);

                    if(recvsize = recv(myinformation.server, buffer, BUFFER_SIZE, 0) >= 0){
                        printf("Server responded: %s \n", buffer);
                        fflush(stdout);
                        }
                    tokenize(recvtoken1, recvtoken2, buffer);
                    if(!strcmp(recvtoken1, "RECEIVED")){
                            cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                            cse4589_print_and_log("[%s:END]\n",command_str);
                    }
                    else if(!strcmp(recvtoken1, "ERROR")){
                            cse4589_print_and_log("[%s:ERROR]\n",command_str);
                            cse4589_print_and_log("[%s:END]\n",command_str);
                    }

            }
            /*************************************************************************************************************************************/
            /*************************************************************************************************************************************/
            /*************************************************************************************************************************************/
            else if(!(strcmp(command_str, "EXIT"))){                        //COMMAND EXIT
                    if(myinformation.server!=-1)
                    close(myinformation.server);

                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("[%s:END]\n",command_str);
                    return 0;
                    //break;
            }
            else{                                                           //ERROR COMMAND
                cse4589_print_and_log("[%s:ERROR]\n",command_str);
                cse4589_print_and_log("[%s:END]\n",command_str);
            }
            }

           else if(sock_index == myinformation.server){
                     /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(myinformation.server, buffer, BUFFER_SIZE, 0) <= 0){
                            close(myinformation.server);
                            printf("Remote Host terminated connection!\n");

                            /* Remove from watched list */
                            FD_CLR(myinformation.server, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...
                                printf("\nClient sent me: %s\n", buffer);
                                char *recvtoken1 = (char*) malloc(sizeof(char)*MSG_SIZE);
                                char *recvtoken2 = (char*) malloc(sizeof(char)*MSG_SIZE);
                                char *recvtoken3 = (char*) malloc(sizeof(char)*MSG_SIZE);
                                tokenize(recvtoken1 , recvtoken2 , buffer); //extract command line
                                tokenize(recvtoken3 , buffer , recvtoken2); //extract host name out
                                if(!(strcmp(recvtoken1,"INCOMEMSG"))){

                                        strcpy(recvtoken2, "REVEIVED");
                                        cse4589_print_and_log("[%s:SUCCESS]\n",recvtoken2);
                                        cse4589_print_and_log("msg from:%s\n[msg]:%s", recvtoken3, buffer);
                                        cse4589_print_and_log("[%s:END]\n",recvtoken2);
                                        if(send(myinformation.server, recvtoken2, strlen(buffer), 0) == strlen(buffer))
                                        printf("RECEIVD Done!\n");
                                        fflush(stdout);

                                }
                                else{
                                    printf("UNEXPECTED MESSAGE\n");
                                }

            }
            }
            }
            }
        }
        }
	}


	/*********************server part********************/
	else if(!(strcmp(argv[1], "s"))){

        /*initialization.create a structure to store my information*/
	    struct myInformation {
	        int portnum;
	        char *myhostname ;  //store the name of my computer name
	        char myip[INET6_ADDRSTRLEN];
	    } myinformation ;

	    memset( &myinformation, '\0' , sizeof myinformation);
	    myinformation.portnum = (int) strtol(argv[2],NULL,10);       //store port number into myinformation

        /*initialization.get my host name*/
        myinformation.myhostname = (char*) malloc(sizeof(char)*MSG_SIZE);  //store the name of my computer name
        memset( myinformation.myhostname, '\0', MSG_SIZE);
        gethostname( myinformation.myhostname, MSG_SIZE );
        perror("111");

        /*initialization.get my host name*/
        struct addrinfo *myipaddr;      //this is a pointer to the struct, not need to initial by memset
        getmyip( myinformation.myhostname , myipaddr, myinformation.myip );

        /*initialization.create client data structure for 4 and counter*/
        struct cinfo clientinfo[4];
        clientinfo[0].status = 0;   //initial status as offline
        clientinfo[1].status = 0;
        clientinfo[2].status = 0;
        clientinfo[3].status = 0;
        int clientcounter = 0;

        /*Creat strings for command use*/
        char linestate[2][10];
        strcpy(linestate[0], "offline");
        strcpy(linestate[1], "online");


        /**********************set for data test*********************/
        /*******************************************/
        /*
        strcpy(clientinfo[0].name, "stones.cse.buffalo.edu");
        strcpy(clientinfo[0].ip, "128.205.36.46");
        strcpy(clientinfo[0].port, "4545");
        clientinfo[0].status = 1;
        strcpy(clientinfo[1].name, "embankment.cse.buffalo.edu");
        strcpy(clientinfo[1].ip, "128.205.36.35");
        strcpy(clientinfo[1].port, "3895");
        clientinfo[1].status = 0;
        strcpy(clientinfo[2].name, "highgate.cse.buffalo.edu");
        strcpy(clientinfo[2].ip, "128.205.36.33");
        strcpy(clientinfo[2].port, "5499");
        clientinfo[2].status = 1;
        clientcounter = 3;
        clientinfo[0].mescount = 4;
        perror("333");
        strcpy(clientinfo[0].message[0], "128.205.36.33 One");
        strcpy(clientinfo[0].message[1], "128.205.36.33 Two");
        strcpy(clientinfo[0].message[2], "128.205.36.35 125457");
        strcpy(clientinfo[0].message[3], "128.205.36.35 8858779");

        */
        /*************************************************************************************/

        int port, server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
        struct sockaddr_in server_addr, client_addr;
        fd_set master_list, watch_list;
        /* Socket */
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(server_socket < 0)
            perror("Cannot create socket");

        /* Fill up sockaddr_in struct */
        port = atoi(argv[2]);
        bzero(&server_addr, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        /* Bind */
        if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
            perror("Bind failed");

        /* Listen */
        if(listen(server_socket, BACKLOG) < 0)
            perror("Unable to listen on port");

        /* ---------------------------------------------------------------------------- */

        /* Zero select FD sets */
        FD_ZERO(&master_list);
        FD_ZERO(&watch_list);

        /* Register the listening socket */
        FD_SET(server_socket, &master_list);
        /* Register STDIN */
        FD_SET(STDIN, &master_list);

        head_socket = server_socket;

        printf("\n[PA1-Client@CSE489/589]$ ");
        fflush(stdout);

        while(TRUE){
            memcpy(&watch_list, &master_list, sizeof(master_list));

            //printf("\n[PA1-Server@CSE489/589]$ ");
            //fflush(stdout);

            /* select() system call. This will BLOCK */
            selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
            if(selret < 0)
                perror("select failed.");

            /* Check if we have sockets/STDIN to process */
            if(selret > 0){
                /* Loop through socket descriptors to check which ones are ready */
                for(sock_index=0; sock_index<=head_socket; sock_index+=1){

                    if(FD_ISSET(sock_index, &watch_list)){

                        /* Check if new command on STDIN */
                        if (sock_index == STDIN){

                            /*********************************PROCESSING COMMAND*******************************/
                            /*command input section*/
                            char *input_cmd = (char*) malloc(sizeof(char)*INPUT_SIZE);  //store the input command in to input_cmd
                            memset(input_cmd, '\0', INPUT_SIZE);
                            if(fgets(input_cmd, INPUT_SIZE-1, stdin) == NULL)       //copy the input into input_cmd, if no input, exit
                                exit(-1);

                            printf("I got: %s (size:%d chars)\n", input_cmd, strlen(input_cmd));

                            /*command analysis section*/
                            char *command_str = (char*) malloc(sizeof(char)*MSG_SIZE);      //have two string store command and the rest information
                            memset(command_str, '\0', MSG_SIZE);
                            char *cmdrest = (char*) malloc(sizeof(char)*INPUT_SIZE);
                            memset(cmdrest, '\0', INPUT_SIZE);
                            tokenize( command_str, cmdrest, input_cmd );        //store the command section into command_str, the rest, if there is , stores in cmdrest

                            /*command recognition*/
                            if(!(strcmp(command_str, "AUTHOR"))){                           //COMMAND AUTHOR
                                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                                    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "zhang25");
                                    cse4589_print_and_log("[%s:END]\n",command_str);
                            }
                            else if(!(strcmp(command_str, "IP"))){                        //COMMAND IP
                                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                                    cse4589_print_and_log("IP:%s\n", myinformation.myip);
                                    cse4589_print_and_log("[%s:END]\n",command_str);
                            }
                            else if(!(strcmp(command_str, "PORT"))){                        //COMMAND PORT
                                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                                    cse4589_print_and_log("PORT:%d\n", myinformation.portnum);
                                    cse4589_print_and_log("[%s:END]\n",command_str);
                            }
                            else if(!(strcmp(command_str, "LIST"))){
                                    /*display the list*/
                                    int listid=0;
                                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                                    for(int i = 0; i < clientcounter ; i++){
                                            if(clientinfo[i].status==1){
                                                    listid++;
                                                    cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", listid, clientinfo[i].name, clientinfo[i].ip, atoi(clientinfo[i].port));
                                            }
                                    }
                                    cse4589_print_and_log("[%s:END]\n",command_str);

                            }
                            else if(!(strcmp(command_str, "STATISTICS"))){                  //COMMAND STATISTICS
                                    /*display the statistics*/

                                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                                    for(int i = 0; i < clientcounter ; i++){
                                            cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, clientinfo[i].name, clientinfo[i].numsend, clientinfo[i].numrecv, linestate[clientinfo[i].status]);
                                    }
                                    cse4589_print_and_log("[%s:END]\n",command_str);
                            }
                            else if(!(strcmp(command_str, "BLOCKED"))){                        //COMMAND BLOCKED
                                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                                    cse4589_print_and_log("PORT:%d\n", myinformation.portnum);
                                    cse4589_print_and_log("[%s:END]\n",command_str);
                            }
                            else{                                                           //ERROR COMMAND
                                cse4589_print_and_log("[%s:ERROR]\n",command_str);
                                cse4589_print_and_log("[%s:END]\n",command_str);
                            }

                            free(input_cmd);
                        }
                        /*********************************PROCESSING COMMAND*******************************/


                        /* Check if new client is requesting connection */
                        else if(sock_index == server_socket){
                            caddr_len = sizeof(client_addr);
                            fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                            if(fdaccept < 0)
                                perror("Accept failed.");

                            printf("\nRemote Host connected!\n");

                            /* Add to watched socket list */
                            FD_SET(fdaccept, &master_list);
                            if(fdaccept > head_socket) head_socket = fdaccept;
                        }
                        /* Read from existing clients */
                        else{
                            /* Initialize buffer to receieve response */
                            char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                            memset(buffer, '\0', BUFFER_SIZE);

                            if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                                close(sock_index);
                                printf("Remote Host terminated connection!\n");

                                /* Remove from watched list */
                                FD_CLR(sock_index, &master_list);
                            }
                            else {
                                /*********************************PROCESSING INCOMMING COMMAND*******************************/
                                /*********************************PROCESSING INCOMMING COMMAND*******************************/

                                //Process incoming data from existing clients here ...

                                printf("\nClient sent me: %s\n", buffer);
                                char *recvtoken1 = (char*) malloc(sizeof(char)*MSG_SIZE);
                                char *recvtoken2 = (char*) malloc(sizeof(char)*MSG_SIZE);
                                char *recvtoken3 = (char*) malloc(sizeof(char)*MSG_SIZE);
                                tokenize(recvtoken1 , recvtoken2 , buffer); //extract command line
                                tokenize(recvtoken3 , buffer , recvtoken2); //extract host name out
                                if(!(strcmp(recvtoken1,"LOGIN"))){

                                    int existed = -1;

                                    //printf("recvtoken3:|%s|\n", recvtoken3);

                                    for(int i=0; i<4; i++){
                                        if(!(strcmp(clientinfo[i].name,recvtoken3))){
                                            existed = i;
                                        }
                                    }
                                    if(existed == -1 ){        //this is a new client
                                            //store client info into clientinfo and initialize data//
                                        strcpy(clientinfo[clientcounter].name , recvtoken3);
                                        tokenize(clientinfo[clientcounter].ip,recvtoken1,buffer);
                                        tokenize(clientinfo[clientcounter].port,recvtoken3,recvtoken1);
                                        clientinfo[clientcounter].status = 1; //set online
                                        clientinfo[clientcounter].mescount = 0; //reset message counter
                                        clientinfo[clientcounter].socket = sock_index; //save socket for recognition
                                        clientinfo[clientcounter].numrecv = 0;
                                        clientinfo[clientcounter].numsend = 0;
                                        clientinfo[clientcounter].blockflag[0] = 0;
                                        clientinfo[clientcounter].blockflag[1] = 0;
                                        clientinfo[clientcounter].blockflag[2] = 0;
                                            //have one more client connecting
                                        clientcounter += 1;
                                        sortcinfo(clientinfo, clientcounter);       //every time a new client connect sort the info array to maintain it
                                        for(int i=0; i<clientcounter ; i++){       //send list
                                            if(clientinfo[i].status>0){        //only send the online client
                                                char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                                memset(bufmes,'\0',INPUT_SIZE);
                                                strcpy(bufmes, "LIST");
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].name);
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].ip);
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].port);
                                                strcat(bufmes, " ");
                                                printf("SENDing: %s\n", bufmes);
                                                if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                printf("Done!\n");

                                                recv(sock_index, buffer, BUFFER_SIZE, 0);   //wait for ack
                                                //printf("echo back: %s\n", buffer);
                                            }
                                        }

                                            char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                            memset(bufmes,'\0',INPUT_SIZE);

                                            strcpy(bufmes , "FINISH");
                                            strcat(bufmes, " ");
                                            if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                printf("\nLOGINFINISH! %s\n",bufmes);

                                    }
                                    else{                       //this is a relogin client
                                        clientinfo[existed].socket = sock_index;
                                        for(int i=0; i<clientcounter ; i++){       //send list
                                            if(clientinfo[i].status>0){        //only send the online client
                                                char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                                memset(bufmes,'\0',INPUT_SIZE);
                                                strcpy(bufmes, "LIST");
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].name);
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].ip);
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].port);
                                                strcat(bufmes, " ");
                                                printf("SENDing: %s\n", bufmes);
                                                if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                printf("Done!\n");

                                                recv(sock_index, buffer, BUFFER_SIZE, 0);   //wait for ack
                                                //printf("echo back: %s\n", buffer);
                                            }
                                        }
                                        if(clientinfo[existed].mescount > 0){       //if have buffered message
                                            for(int i=0; i<clientinfo[existed].mescount ; i++){
                                                char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                                memset(bufmes,'\0',INPUT_SIZE);

                                                strcpy(bufmes , "MSG");
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[existed].message[i]);
                                                strcat(bufmes, " ");
                                                if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                    printf("buff message Done!\n");
                                                recv(sock_index, buffer, BUFFER_SIZE, 0);
                                                printf("message echo back: %s\n", buffer);
                                            }
                                            clientinfo[existed].numrecv += clientinfo[existed].mescount;
                                            clientinfo[existed].mescount = 0 ;
                                        }
                                        char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                        memset(bufmes,'\0',INPUT_SIZE);

                                        strcpy(bufmes, "FINISH");
                                        strcat(bufmes, " ");
                                        strcat(bufmes, "123123");
                                        if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                            printf("\nLOGINFINISH! %s\n",bufmes);

                                    }
                                }
                                else if(!(strcmp(recvtoken1,"REFRESH"))){

                                        for(int i=0; i<clientcounter ; i++){       //send list
                                            if(clientinfo[i].status>0){        //only send the online client
                                                char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                                memset(bufmes,'\0',INPUT_SIZE);
                                                strcpy(bufmes, "LIST");
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].name);
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].ip);
                                                strcat(bufmes, " ");
                                                strcat(bufmes, clientinfo[i].port);
                                                strcat(bufmes, " ");
                                                printf("SENDing: %s\n", bufmes);
                                                if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                printf("Done!\n");

                                                recv(sock_index, buffer, BUFFER_SIZE, 0);   //wait for ack
                                                //printf("echo back: %s\n", buffer);
                                            }
                                        }
                                        char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                        memset(bufmes,'\0',INPUT_SIZE);

                                        strcpy(bufmes ,"FINISH");
                                        strcat(bufmes, " ");
                                        if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                            printf("\nREFRESH FINISH! %s\n",bufmes);

                                }
                                else if(!(strcmp(recvtoken1,"SEND"))){
                                        tokenize(recvtoken1,recvtoken2,buffer);
                                        /*recvtoken1 points to clientip, recvtoken2 points to msg,recvtoken3 points to my ip*/

                                        //int ifclientonline = -1;        //to indicate if this one is online, online set to 1, offline set to 0
                                        int ifclientvalid = 0;          //check mark to see a valid client, 0 invalid, 1 valid;
                                        int clientid = -1;
                                        for(int i=0; i<clientcounter ; i++){
                                            if(!(strcmp(clientinfo[i].ip,recvtoken1))){
                                                ifclientvalid = 1;      //client do exist
                                                clientid = i;           //store client id for find fd
                                            }
                                        }
                                        if(ifclientvalid == 0){
                                                //client not valid
                                            char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                            memset(bufmes,'\0',INPUT_SIZE);

                                            strcpy(bufmes ,"ERROR");
                                            strcat(bufmes, " ");

                                            if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                printf("\nREFRESH FINISH! %s\n",bufmes);
                                        }
                                        else{
                                                //client do exist
                                                if(clientinfo[clientid].status==0){
                                                        //not online, buffer message
                                                        strcpy(clientinfo[clientid].message[clientinfo[clientid].mescount], recvtoken3);
                                                        strcat(clientinfo[clientid].message[clientinfo[clientid].mescount], " ");
                                                        strcat(clientinfo[clientid].message[clientinfo[clientid].mescount], recvtoken2);
                                                        clientinfo[clientid].mescount++;
                                                        for(int i =0; i<clientcounter; i++){
                                                            if(!(strcmp(clientinfo[i].ip,recvtoken3))){
                                                                clientinfo[i].numsend++;        //sender #send message +1
                                                            }
                                                        }
                                                }
                                                else{
                                                    //ready to send message to client

                                                    //recvtoken1 = clientip
                                                    //recvtoken2 = msg
                                                    //recvtoken3 = myip
                                                    //clientid indicate clientinfo[i]
                                                        clientinfo[clientid].numrecv++;         //receiver #receiver message +1
                                                        for(int i =0; i<clientcounter; i++){
                                                            if(!(strcmp(clientinfo[i].ip,recvtoken3))){
                                                                clientinfo[i].numsend++;        //sender #send message +1
                                                            }
                                                        }

                                                        /*send msg to receiver*/
                                                        char *bufmes =(char*) malloc(sizeof(char)*INPUT_SIZE);
                                                        memset(bufmes,'\0',INPUT_SIZE);
                                                        strcpy(bufmes, "INCOMEMSG");
                                                        strcat(bufmes, " ");
                                                        strcat(bufmes, recvtoken3);
                                                        strcat(bufmes, " ");
                                                        strcat(bufmes, recvtoken2);
                                                        strcat(bufmes, " ");
                                                        printf("SENDing: %s\nto client...", bufmes);
                                                        if(send(clientinfo[clientid].socket, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                        printf("Done!\n");
                                                        recv(clientinfo[clientid].socket, buffer, BUFFER_SIZE, 0);   //wait for ack

                                                        /*output EVENT message*/
                                                        char command_string[MSG_SIZE];
                                                        perror("commandstring");
                                                        memset(command_string,'\0',MSG_SIZE);
                                                        perror("commandstring2");
                                                        strcpy(command_string, "RECEIVED");
                                                        perror("commandstring3");
                                                        cse4589_print_and_log("[%s:SUCCESS]\n",command_string);
                                                        cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s", recvtoken3, recvtoken1, recvtoken2);
                                                        cse4589_print_and_log("[%s:END]\n",command_string);


                                                        /*SEND back to sender*/
                                                        memset(bufmes,'\0',INPUT_SIZE);
                                                        strcpy(bufmes, "RECEIVED");
                                                        strcat(bufmes, " ");
                                                        printf("SENDing: %s\nto client...", bufmes);
                                                        if(send(sock_index, bufmes, strlen(bufmes), 0) == strlen(bufmes))
                                                        printf("Done!\n");
                                                        //recv(sock_index, buffer, BUFFER_SIZE, 0);   //wait for ack

                                                }
                                        }

                                }
                                else{
                                        printf("UNEXPECTED MESSAGE!\n");
                                }
                            }

                            free(buffer);
                        }
                    }
                }
            }
        }

        return 0;

        /*************************************************************************************/



	}
	else{
        printf("command error");
	}

	return 0;
}


/*********************subfunction********************
NAME:void tokenize(char *command_str, char *cmdrest, char *input_cmd)
USEAGE: accept 3 pointer of string, seperate the last one  in to two parts and
stores in the first and the second
****************************************************/

void tokenize(char *command_str, char *cmdrest, char *input_cmd){
    char *arg = (char*) malloc(sizeof(char)*INPUT_SIZE);
    memset(arg, '\0', INPUT_SIZE);
    char *input = (char*) malloc(sizeof(char)*INPUT_SIZE);
    memset(input, '\0', INPUT_SIZE);
    strcpy(input , input_cmd);
    arg = strtok(input_cmd," \n\0");
    strcpy(command_str , arg);          //copy the first section into command_str as command
    arg = strpbrk(input, " \n");  //find the first delimiter location
    if (arg != NULL)
    strcpy(cmdrest, arg+1);

}


/*********************subfunction********************
NAME:int getmyip(char *argv, struct addrinfo *res, char *myip)
USEAGE: accept my host name, return the address info back to pointer res and my ip address in myip as a string
****************************************************/

int getmyip(char *argv, struct addrinfo *res, char *myip){
	struct addrinfo hints;
	int status;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(argv, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	/*ip result test*/
    char ipstr[INET6_ADDRSTRLEN];
    memset( ipstr, '\0', INET6_ADDRSTRLEN);
    struct addrinfo *p;
        //printf("IP addresses for %s:\n\n", argv);

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        strcpy( myip , ipstr);

        //printf("  %s: %s\n", ipver, ipstr);   //output IP version and IP address
    }

	return 0;
}


/*********************subfunction********************
NAME:int connect_to_host(char *server_ip, int server_port)
USEAGE: accept server ip and server port to connect to the server
return fdsocket for sending data
****************************************************/
int connect_to_host(char *server_ip, int server_port, int *flag)
{
    int fdsocket, len, mark;
    struct sockaddr_in remote_server_addr;
    //&mark = flag;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0)
        *flag = 0;
       //perror("Failed to create socket");

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        *flag = 0;
        //perror("Connect failed");

    return fdsocket;
}


void sortcinfo(struct cinfo *Cinfo, int num){
    int mirror[num];
    int buffer;
    int index;  //the index of the value of buffer in array mirror
    struct cinfo infobuffer;
    for (int i=0; i<num; i++){
        mirror[i]=atoi(Cinfo[i].port);
        printf("mirror is %d\n",mirror[i]);
    }
    if(num > 1){
        for (int i= 0; i<num-1 ; i++){
            buffer = mirror[i];
            index = i;
            for (int j = i+1; j < num ; j++){
                if(buffer > mirror[j]){
                    buffer = mirror[j];
                    index = j;
                }
                printf("IN J LOOP: i is %d, j is %d, buffer is %d\n",i,j,buffer);
            }
            if(index != i){
                    printf("i is %d, index is %d\n",i,index);
                    //change mirror *forget the first time
                    buffer = mirror[i];
                    mirror[i] = mirror[index];
                    mirror[index] = mirror[buffer];
                    //change struct
                    copystruc( &infobuffer, &Cinfo[i]);
                    copystruc(&Cinfo[i], &Cinfo[index]);
                    copystruc(&Cinfo[index], &infobuffer);
            }
        }
    }
}

void copystruc(struct cinfo *target, struct cinfo *copy){
    strcpy(target->name, copy->name);
    strcpy(target->ip, copy->ip);
    target->numsend = copy->numsend;
    target->numrecv = copy->numrecv;
    strcpy(target->port, copy->port);
    target->status = copy->status;
    strcpy(target->block[0], copy->block[0]);
    strcpy(target->block[1], copy->block[1]);
    strcpy(target->block[2], copy->block[2]);
    target->blockflag[0] = copy->blockflag[0];
    target->blockflag[0] = copy->blockflag[0];
    target->blockflag[0] = copy->blockflag[0];
    for (int i=0; i< copy->mescount; i++){
        strcpy(target->message[i], copy->message[i]);
    }
    target->mescount = copy->mescount;
    target->socket = copy->socket;
}

char* itoa(int num)
{
    int size = 4 + 1;
    char *x = malloc(sizeof(char)*size);
    snprintf(x, size, "%d", num);
    return x;
}


