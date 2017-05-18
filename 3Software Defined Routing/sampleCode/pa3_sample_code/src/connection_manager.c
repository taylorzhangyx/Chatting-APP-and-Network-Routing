/**
 * @connection_manager
 * @author  Zhang Yuxin <zhang25@buffalo.edu>
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
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/routing_handler.h"

fd_set master_list, watch_list;
int head_fd;
struct timeval select_time;

void main_loop()
{
    int selret, sock_index, fdaccept;
    select_time = NULL;

    while(TRUE){
        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, select_time);

        /*time out*/
        if(selret == 0){
            refresh_select_time(INTERVAL,0);
        }

        if(selret < 0)
            ERROR("select failed.");

        /* Loop through file descriptors to check which ones are ready */
        for(sock_index=0; sock_index<=head_fd; sock_index+=1){

            if(FD_ISSET(sock_index, &watch_list)){

                /* control_socket */
                if(sock_index == control_socket){
                    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }

                /* router_socket */
                else if(sock_index == router_socket){
                    printf("get routing table packet!!");
                    refresh_select_time(INTERVAL,0);
                    //call handler that will call recvfrom() .....
                }

                /* data_socket */
                else if(sock_index == data_socket){
                    //new_data_conn(sock_index);
                }

                /* Existing connection */
                else{
                    if(isControl(sock_index)){
                        switch (control_recv_hook(sock_index)){
                            case TRUE: break;
                            case 2:
                                //Finishing initializing, need to create router socket and data socket
                                prepare_port(&router_socket; &data_socket);

                                writedown_neighbor_table();
                                send_routing_table(router_socket); //spread routing table
                                refresh_select_time(INTERVAL,0); //start timer to update routing table

                                break;
                            case FALSE: FD_CLR(sock_index, &master_list);
                            }
                    }
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index");
                }
            }
        }
    }
}

/**
*   update select time to control the time flow
*/
void refresh_select_time(int sec, int usec){
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tc_usec = usec;
    select_time = tv;
}

void init()
{
    control_socket = create_sock(CONTROL_PORT, TCP);

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    main_loop();
}
