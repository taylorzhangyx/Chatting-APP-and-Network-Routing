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

#include "../include/global.h"
#include "../include/logger.h"

#define TRUE 1
#define FALSE 0
#define MSG_SIZE 256
#define INPUT_SIZE 278 //CMD <IP> <msg> (10+1+16+1+256)
#define BUFFER_SIZE 256


void tokenize(char *command_str, char* cmdrest, char *input_cmd);
int getmyip(char *argv, struct addrinfo *res, char *myip);

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
	        char *myhostname ;  //store the name of my computer name
	        char myip[INET6_ADDRSTRLEN];
	    } myinformation ;

	    memset( &myinformation, 0 , sizeof myinformation);
	    myinformation.portnum = (int) strtol(argv[2],NULL,10);       //store port number into myinformation

        /*initialization.get my host name*/
        myinformation.myhostname = (char*) malloc(sizeof(char)*MSG_SIZE);  //store the name of my computer name
        memset( myinformation.myhostname, '\0', MSG_SIZE);
        gethostname( myinformation.myhostname, MSG_SIZE );

        /*initialization.get my host name*/
        struct addrinfo *myipaddr;      //this is a pointer to the struct, not need to initial by memset
        getmyip( myinformation.myhostname , myipaddr, myinformation.myip );





        while(TRUE){
            printf("\n[PA1-Client@CSE489/589]$ ");
            fflush(stdout);


            /*command input section*/
            char *input_cmd = (char*) malloc(sizeof(char)*INPUT_SIZE);  //store the input command in to input_cmd
            memset(input_cmd, '\0', INPUT_SIZE);
            if(fgets(input_cmd, INPUT_SIZE-1, stdin) == NULL)       //copy the input into input_cmd, if no input, exit
                exit(-1);

            printf("I got: %s(size:%d chars)\n", input_cmd, strlen(input_cmd));

            /*command analysis section*/
            char *command_str = (char*) malloc(sizeof(char)*MSG_SIZE);      //have two string store command and the rest information
            memset(command_str, '\0', MSG_SIZE);
            perror("char *command_str");
            char *cmdrest = (char*) malloc(sizeof(char)*INPUT_SIZE);
            memset(cmdrest, '\0', INPUT_SIZE);
            perror("char *cmdrest");
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
            else if(!(strcmp(command_str, "EXIT"))){                        //COMMAND EXIT
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("[%s:END]\n",command_str);
                    break;
            }
            else{                                                           //ERROR COMMAND
                cse4589_print_and_log("[%s:ERROR]\n",command_str);
                cse4589_print_and_log("[%s:END]\n",command_str);
            }
        }
	}

	/*********************server part********************/


	else if(!(strcmp(argv[1], "s"))){

	    while(TRUE){
            printf("\n[PA1-Client@CSE489/589]$ ");
            fflush(stdout);

            char *input_cmd = (char*) malloc(sizeof(char)*INPUT_SIZE);  //store the input command in to input_com
            memset(input_cmd, '\0', INPUT_SIZE);
            if(fgets(input_cmd, INPUT_SIZE-1, stdin) == NULL)
                exit(-1);

            printf("I got: %s(size:%d chars)\n", input_cmd, strlen(input_cmd));

            char *command_str = (char*) malloc(sizeof(char)*MSG_SIZE);      //have two string store command and the rest information
            memset(command_str, '\0', MSG_SIZE);
            perror("char *command_str");
            char *cmdrest = (char*) malloc(sizeof(char)*INPUT_SIZE);
            memset(cmdrest, '\0', INPUT_SIZE);
            perror("char *cmdrest");
            tokenize( command_str, cmdrest, input_cmd );

            if(!(strcmp(command_str, "AUTHOR"))){                           //COMMAND AUTHOR
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "zhang25");
                    cse4589_print_and_log("[%s:END]\n",command_str);
            }
            else if(!(strcmp(command_str, "EXIT"))){                        //COMMAND EXIT
                    cse4589_print_and_log("[%s:SUCCESS]\n",command_str);
                    cse4589_print_and_log("[%s:END]\n",command_str);
                    break;
            }
            else{                                                           //ERROR COMMAND
                cse4589_print_and_log("[%s:ERROR]\n",command_str);
                cse4589_print_and_log("[%s:END]\n",command_str);
            }
        }

	}
	else{
        printf("command error");
	}

	return 0;
}


/*********************subfunction********************
NAME:void tokenize(char *command_str, char *cmdrest, char *input_cmd)
USE: accept 3 pointer of string, seperate the last one  in to two parts and
stores in the first and the second
****************************************************/

void tokenize(char *command_str, char *cmdrest, char *input_cmd){
    char *arg = (char*) malloc(sizeof(char)*INPUT_SIZE);
    memset(arg, '\0', INPUT_SIZE);
    perror("char *arg");
    arg = strtok(input_cmd," \n");
    perror("arg");
    strcpy(command_str , arg);          //copy the first section into command_str as command
    perror("copy1");
    arg = strtok(NULL, " ");
    perror("strtok2");
    if(arg != NULL){
            strcpy(cmdrest, arg);       //copy the rest part into cmdrest
            perror("copy2");
    }

}







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
        printf("IP addresses for %s:\n\n", argv);

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
