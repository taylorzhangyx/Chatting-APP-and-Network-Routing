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


#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/routing_handler.h"


fd_set master_list, watch_list;
int head_fd;

/**
*   update select time to control the time flow
*/
struct timeval* refresh_select_time(int sec, int usec){
    select_time->tv_sec = sec;
    select_time->tv_usec = usec;
    return select_time;
}

/**
*   check neighbor's status, increment neighbor update count by 1
*   deal with crush neighbor
*
*/
bool neighbor_satus()
{
    bool flag = FALSE;
    int i = 0;
    int j = 0;
    for(i=0; i<num_neighbor; i++){
    //loop through all the neighbor
        for(j=0; j<NUM_ROUTER; j++){
        //loop the router list to find this router's info
            if(neighbor_list[i].ID == router_list[j].ID){
                router_list[j].update_count += 1;
                if(router_list[j].update_count >=3){
                    //crush router i
                    router_list[j].cost = INF;
                    //router_list[j].active = 10; //10 means crash
                    //neighbor_list[i].active = 0; //0 means not exist
                    flag = TRUE;
                }
            }
        }
    }

    return flag;
}

void main_loop()
{
    int selret, sock_index, fdaccept;
    select_time = (struct timeval *) malloc(sizeof (struct timeval));
    bzero(select_time, sizeof(struct timeval));
    refresh_select_time(INF,INF);

    while(TRUE){
        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, select_time);

        /*time out, time to send routing table again*/
        if(selret == 0){
            perror("selret == 0");
            neighbor_satus();
            send_routing_table(router_socket); //spread routing table
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

                    printf("get routing table packet!!\n\n");
                    //call handler that will call recvfrom() .....
                    recv_routing_packet(sock_index);

                    printf("update routing table success!\n\n");
                }

                /* data_socket */
                else if(sock_index == data_socket){
                    //new_data_conn(sock_index);
                }

                /* Existing connection */
                else{
                    if(isControl(sock_index)){
                        switch (control_recv_hook(sock_index)){
                            case 0:
                                printf("control_recv_hook case 0!\n");
                                FD_CLR(sock_index, &master_list);
                                break;

                            case 1:
                                printf("control_recv_hook case 1 success!\n");
                                break;

                            case 2:
                                //Finishing initializing, need to create router socket and data socket
                                prepare_port(&router_socket, &data_socket);
                                FD_SET(router_socket, &master_list);
                                FD_SET(data_socket, &master_list);
                                if(router_socket > head_fd) head_fd = router_socket;
                                if(data_socket > head_fd) head_fd = data_socket;
                                writedown_neighbor_table();
                                send_routing_table(router_socket); //spread routing table
                                refresh_select_time(INTERVAL,0); //start timer to update routing table
                                break;

                            case 3:
                                printf("control_recv_hook case 3 success!\nRouting Table Send!\n\n");
                                break;

                            case 4:
                                printf("control_recv_hook case 4 success!\nREFRESH!!\n\n");
                                break;

                            case 5:
                                printf("\n\n\n\nCRUSH!\n");
                                exit(0);
                                break;
                        }
                    }
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index");

                    printf("end of else 11!\n");
                }
            }
        }
    }
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
