/**
 * @zhang25_assignment3
 * @author  Yuxin Zhang <zhang25@buffalo.edu>
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
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/unistd.h>
#include "../include/global.h"
#include "../include/network_util.h"

#define true 1
#define false 0
#define BACKLOG 20


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Start Here*/
    int initial_flag = false;
	int control_socket, router_socket, data_socket, head_socket, active_socket, select_result, accept_socket, current_socket;
	int controller_socket;
	struct sockaddr_in router_addr, controller_addr;
	fd_set master_list, watch_list;
	//master list stores all the sockets that need to be checked update all the time
	//watch list is a list that select will go through

	//Socket for Control port
	control_socket = socket(PF_INET, SOCK_DGRAM, 6);
	if(control_socket < 0) perror("Connot create control socket");
	//fill in this router's info into sockaddr_in to create socket
	bzero(&router_addr, sizeof(router_addr));
    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //store control port
    sscanf(argv[1], "%" SCNu16, &CONTROL_PORT);
    router_addr.sin_port = CONTROL_PORT;

    /*Bind the control socket*/
    if(bind(control_socket, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0) perror("Bind control socket failed");

    /*Listen the control socket for control request*/
    if(listen(control_socket, BACKLOG) < 0) perror("Unable to listen on control port");

    /*Select*/
    // Zero select FD sets
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    //Register the listening socket
    FD_SET(control_socket, &master_list);
    //set head socket to set upper bond for select loop
    head_socket = control_socket;
    fflush(stdout);

    memcpy(&watch_list, &master_list, sizeof(master_list));

    while(true){
        /*****************Prepare select to accept controller connect******************/
        select_result = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(select_result < 0) perror("select failed.");

        if(select_result > 0){
            for(current_socket=0; current_socket<=head_socket; current_socket+=1){
                if(FD_ISSET(current_socket, &watch_list)){
                    /**********A controller trying to connect this router**********/
                    if(current_socket == control_socket){
                        //process controller package
                        accept_socket = accept(control_socket, (struct sockaddr *)&controller_addr, sizeof controller_addr);
                        if(accept_socket < 0) perror("Accept controller failed.");
                        printf("\nController connected!\n");

                        //store this socket as controller socket
                        controller_socket = accept_socket;
                        //add socket to watch list
                        FD_SET(accept_socket, &master_list);
                        if(accept_socket > head_socket) head_socket = accept_socket;
                    }

                    /* router_socket */
                    /**********A neighbor router trying to connect this router**********/
                    else if(sock_index == router_socket){
                        //call handler that will call recvfrom() .....
                    }

                    /* data_socket */
                    /**********A neighbor router trying to connect this router to build file path**********/
                    else if(sock_index == data_socket){
                        //new_data_conn(sock_index);
                    }

                    /***An Existing socket trying to send message*/
                    else{

                            if(current_socket == controller_socket){
                            /**********THE CONTROLLER trying to send command to this router**********/


                            /****************/
                            //process the incoming initialization control payload
                        }
                    }

                }
            }
        }
    }

/**
    while(true){
        /*****************Prepare select to check multiple request******************
        //transport data from master list to watch list
        memcpy(&watch_list, &master_list, sizeof(master_list));

        select_result = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(select_result < 0) perror("select failed.");
        //A controller trying to connect this router
        if(select_result > 0){
            //loop all the socket description to process them
            for(active_socket; active_socket <= )
        }
    }
**/
	return 0;
}
