/**
 * @zhang25_assignment3
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
 * This controls routing functionality
 */



#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/queue.h>
#include <unistd.h>
#include <string.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/initialize.h"

#define NEIGHBOR_INFO_LEN 11

#define ROUTING_PACKET_HEAD_LEN 0x08
#define ROUTING_PACKET_ENTRY_LEN 0x0c
#define ROUTING_PACKET_SOURCE_ROUTER_PORT_OFFSET 0x02
#define ROUTING_PACKET_SOURCE_IP_ADDRESS_OFFSET 0x04
#define ROUTING_PACKET_ROUTER_PORT_OFFSET 0x04
#define ROUTING_PACKET_ROUTER_ID_OFFSET 0x08
#define ROUTING_PACKET_ROUTER_COST_OFFSET 0x0a



//used to remember the destinations of this router to send routing table to


void writedown_neighbor_table()
{
    num_neighbor = 0;
    int i = 0;
    for(i=0; i<NUM_ROUTER; i++){
        if(router_list[i].cost > 0 && router_list[i].cost < INF)
            //check if neighbor
        {
            router_list[i].next_hop = router_list[i].ID;
            num_neighbor +=1;
        }
    }
    neighbor_list = (struct neighbor_info *) malloc(NEIGHBOR_INFO_LEN*num_neighbor + UINT16LEN);
    //assign info into neighbor list
    int j = 0;
    for(i=0; i<NUM_ROUTER; i++){
        if(router_list[i].cost > 0 && router_list[i].cost < INF)
            //check if neighbor
        {
            neighbor_list[j].active = 1;
            neighbor_list[j].ID = router_list[i].ID;
            neighbor_list[j].router_port = router_list[i].router_port;
            neighbor_list[j].data_port = router_list[i].data_port;
            neighbor_list[j].ipaddr = router_list[i].ipaddr;
            j+=1; //increment j to point to next neighbor
        }
    }
}

/**
*   based on router list to make the routing packet
*
*/
char* make_routing_packet()
{
    int buffer_len = ROUTING_PACKET_HEAD_LEN + NUM_ROUTER*ROUTING_PACKET_ENTRY_LEN;
    char *buffer = (char *) malloc(buffer_len);
    //bzero(buffer, sizeof(*buffer));
    uint16_t number_fields = NUM_ROUTER;
    uint16_t source_router_port = ROUTER_PORT;
    uint32_t ipaddr = IP_ADDR;

    /*fill in packet header*/
    number_fields = htons(number_fields);//number of update fields
    memcpy(buffer, &number_fields, sizeof(number_fields));
    source_router_port = htons(source_router_port);//source router port
    memcpy(buffer+ROUTING_PACKET_SOURCE_ROUTER_PORT_OFFSET, &source_router_port, sizeof(source_router_port));
    ipaddr = htonl(ipaddr);//source router IP address
    memcpy(buffer+ROUTING_PACKET_SOURCE_IP_ADDRESS_OFFSET, &ipaddr, sizeof(ipaddr));

    uint16_t router_ipaddr;
    uint16_t port;
    uint16_t id;
    uint16_t cost;

    /*fill in entries*/
    int i=0;
    for(i=0; i<NUM_ROUTER; i++)
    {
        router_ipaddr = router_list[i].ipaddr;
        port = router_list[i].router_port;
        id = router_list[i].ID;
        cost = router_list[i].cost;

        router_ipaddr = htonl(router_ipaddr);//router IP address
        memcpy(buffer + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i, &router_ipaddr, sizeof(router_ipaddr));
        port = htons(port);//router port
        memcpy(buffer + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i + ROUTING_PACKET_ROUTER_PORT_OFFSET, &port, sizeof(port));
        id = htons(id);//router ID
        memcpy(buffer + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i + ROUTING_PACKET_ROUTER_ID_OFFSET, &id, sizeof(id));
        cost = htons(cost);//cost
        memcpy(buffer + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i + ROUTING_PACKET_ROUTER_COST_OFFSET, &cost, sizeof(cost));
    }
    return buffer;
}

void send_routing_table(int sockfd)
{
    int packet_len = ROUTING_PACKET_HEAD_LEN + NUM_ROUTER*ROUTING_PACKET_ENTRY_LEN;
    char *packet = (char *) malloc(packet_len);

    packet = make_routing_packet();
    //bzero(packet,sizeof(*packet));
    //memcpy(packet, make_routing_packet(), ROUTING_PACKET_HEAD_LEN + NUM_ROUTER*ROUTING_PACKET_ENTRY_LEN);

    printf("Routing packet:");
    printf("size of packet:%d,", sizeof(*packet));
    int n =0;
    for(n=0; n<sizeof(*packet); n++)
    printf("%c", packet[n]);
    printf("\n");

    struct sockaddr_in destination; //destination sockaddr_in for sendto
    socklen_t length = sizeof(destination);
    bzero(&destination, sizeof(destination));

    destination.sin_family = AF_INET;

    int i = 0;
    for(i = 0; i < num_neighbor; i++){
        if(neighbor_list[i].active>0)
        //check is this router still active, only active router will get message
        {
            destination.sin_addr.s_addr = htonl(neighbor_list[i].ipaddr);
            destination.sin_port = htons(neighbor_list[i].router_port);
            if(sendtoALL(sockfd, packet, packet_len, (struct sockaddr *)&destination, sizeof(destination)) < 0){
                ERROR("sendtoALL error!");
            }
        }
    }

    //free(packet);
}


void print_routing_packet_list()
{
    int i = 0;
    for(i=0; i<NUM_ROUTER; i++){
        printf("Router%d, ipaddr:%x ", i, routing_update_list[i].ipaddr);
        printf("router_port:%d ", routing_update_list[i].router_port);
        printf("router_ID:%d ", routing_update_list[i].router_id);
        printf("cost:%d \n", routing_update_list[i].cost);
    }
}



/**
*   based on routing update list to check and refresh routing list (routing table)
*/
bool update_routing_table(uint16_t number_router, uint32_t source_ip)
{
    bool isChange = FALSE;
    uint16_t neighbor_cost;
    uint16_t neighbor_id;
    int i = 0;
    int j = 0;
    //go through router list to find neighbor cost
    for(i = 0; i < number_router; i++){
        if(router_list[i].ipaddr == source_ip){
            neighbor_cost = router_list[i].cost;
            neighbor_id = router_list[i].ID;
            //reset update count to 0
            router_list[i].update_count = 0;
            break;
        }
    }
    uint16_t temp_id;
    uint16_t temp_cost;
    //loop through routing_update_list to compare cost
    for(i = 0; i < number_router; i++){
        //write down the router id that need to compare
        temp_id = routing_update_list[i].router_id;
        temp_cost = routing_update_list[i].cost;
        //loop through routing table to find current cost
        for(j = 0; j < number_router; j++){
            if(router_list[j].ID == temp_id){
                //compare current cost and the cost through this router
                if(temp_cost+neighbor_cost < router_list[j].cost){
                    //check if this router is active
                    if(router_list[j].active == 10){
                        //dead
                        router_list[j].cost = INF;
                        isChange = TRUE;
                    }
                     else{
                        //assign a new value
                        router_list[j].cost = temp_cost+neighbor_cost;
                        router_list[j].next_hop = neighbor_id;
                        isChange = TRUE;
                    }
                }
            }
        }
    }

    return isChange;
}

/**
*   decomposition packet info different chunks and copy into routing update list
*/
void process_routing_packet(char *packet)
{
    uint16_t number_fields;
    uint16_t source_router_port;
    uint32_t ipaddr;

    memcpy(&number_fields, packet, sizeof(number_fields));
    number_fields = ntohs(number_fields);
    memcpy(&source_router_port, packet+ROUTING_PACKET_SOURCE_ROUTER_PORT_OFFSET, sizeof(source_router_port));
    source_router_port = ntohs(source_router_port);
    memcpy(&ipaddr, packet+ROUTING_PACKET_SOURCE_IP_ADDRESS_OFFSET, sizeof(ipaddr));
    ipaddr = ntohl(ipaddr);

    uint16_t router_ipaddr;
    uint16_t port;
    uint16_t id;
    uint16_t cost;

    /*read router packet info*/
    int i=0;
    for(i=0; i<number_fields; i++)
    {
        memcpy(&router_ipaddr, packet + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i, sizeof(router_ipaddr));
        routing_update_list[i].ipaddr = ntohl(router_ipaddr);
        memcpy(&port, packet + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i + ROUTING_PACKET_ROUTER_PORT_OFFSET, sizeof(port));
        routing_update_list[i].router_port = ntohs(port);
        memcpy(&id, packet + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i + ROUTING_PACKET_ROUTER_ID_OFFSET, sizeof(id));
        routing_update_list[i].router_id = ntohs(id);
        memcpy(&cost, packet + ROUTING_PACKET_HEAD_LEN + ROUTING_PACKET_ENTRY_LEN*i + ROUTING_PACKET_ROUTER_COST_OFFSET, sizeof(cost));
        routing_update_list[i].cost = ntohs(cost);
    }
    /*so far the routing packet has been processed into several tokens*/

    print_routing_packet_list();

    if(update_routing_table(number_fields,ipaddr)){
        print_info();
    }

    /*reset routing list to zero*/
    bzero(routing_update_list, sizeof(*routing_update_list));

    printf("end of process_routing_packet\n\n");
}

/**
*   this function is called after router socket is set
*   receive routing packet and process it
*   based on packet to update its own routing table
*/
bool recv_routing_packet(int sockfd)
{
    char *buffer = (char *) malloc(ROUTING_PACKET_HEAD_LEN + NUM_ROUTER*ROUTING_PACKET_ENTRY_LEN); //the buffer to receive routing packet

    int source_addr_len;
    struct sockaddr source_addr;
    bzero(&source_addr, sizeof(struct sockaddr));
    //get routing table from neighbor
    if(recvfromALL(sockfd, buffer, ROUTING_PACKET_HEAD_LEN + NUM_ROUTER*ROUTING_PACKET_ENTRY_LEN, &source_addr, &source_addr_len) < 0){
        ERROR("Recvfrom error!");
    }

    process_routing_packet(buffer);

    //free(buffer);
    return TRUE;
}
