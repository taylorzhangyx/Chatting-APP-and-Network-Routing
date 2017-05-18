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
 * INIT [Control Code: 0x01]
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

#define INIT_INTERVAL_OFFSET (0x02)
#define INIT_PAYLOAD_HEAD_OFFSET (0x04)
#define INIT_PORT1_OFFSET (0x02)
#define INIT_PORT2_OFFSET (0x04)
#define INIT_COST_OFFSET (0x06)
#define INIT_ADDR_OFFSET (0x08)
#define INIT_PKT_OFFSET (0x0c)
#define ROUTER_INFO_LEN (0x0e + sizeof(int)*2)
//router info struck length


void print_info()
{
    printf("CONTROL_PORT:%d, ROUTER_PORT:%d, DATA_PORT:%d \n", CONTROL_PORT,ROUTER_PORT,DATA_PORT);
    printf("IP_ADDR:%x, INTERVAL:%d, NUM_ROUTER:%d\n", IP_ADDR,INTERVAL,NUM_ROUTER);
    int i =0;
    for(i =0; i<NUM_ROUTER; i++){
        printf("list:%d  ,ID:%d, router_port:%d, data_port:%d, cost:%d ,next_hop:%d ,ipaddr:%x \n",i,router_list[i].ID,router_list[i].router_port,router_list[i].data_port,router_list[i].cost,router_list[i].next_hop,router_list[i].ipaddr);
    }
}

void initial()
{
    /*allocate a memory for routing update list*/
    routing_update_list = (struct routing_packet_info *) malloc(sizeof(struct routing_packet_info)*NUM_ROUTER);
    bzero(routing_update_list, sizeof(*routing_update_list));

    /*Find my self to set the next hop to myself to my ID*/
    int i = 0;
    for(i=0; i<NUM_ROUTER; i++)
    {
        //find my self
        if(router_list[i].cost == 0){
            router_list[i].next_hop = router_list[i].ID;
            break;
        }
    }
}

bool init_router(char *payload)
{
    //store number of routers
    memcpy(&NUM_ROUTER, payload, sizeof(NUM_ROUTER));
    NUM_ROUTER = ntohs(NUM_ROUTER); //size is 2 bytes, need to transfer from net to host
    //store updates periodic intervals
    memcpy(&INTERVAL, payload+INIT_INTERVAL_OFFSET, sizeof(INTERVAL));
    INTERVAL = ntohs(INTERVAL);
    //Based on number of routers, fill in router list
    router_list = malloc(sizeof(char)*ROUTER_INFO_LEN*NUM_ROUTER + UINT16LEN);
    bzero(router_list, sizeof(*router_list));
    print_info();/************************/
    /*fill in routing table*/
    int i = 0;
    for(i; i<NUM_ROUTER; i++){
        memcpy(&(router_list[i].ID), payload+INIT_PAYLOAD_HEAD_OFFSET+INIT_PKT_OFFSET*i, UINT16LEN);
        router_list[i].ID = ntohs(router_list[i].ID);
        memcpy(&(router_list[i].router_port), payload+INIT_PAYLOAD_HEAD_OFFSET+INIT_PORT1_OFFSET+INIT_PKT_OFFSET*i, UINT16LEN);
        router_list[i].router_port = ntohs(router_list[i].router_port);
        memcpy(&(router_list[i].data_port), payload+INIT_PAYLOAD_HEAD_OFFSET+INIT_PORT2_OFFSET+INIT_PKT_OFFSET*i, UINT16LEN);
        router_list[i].data_port = ntohs(router_list[i].data_port);
        memcpy(&(router_list[i].cost), payload+INIT_PAYLOAD_HEAD_OFFSET+INIT_COST_OFFSET+INIT_PKT_OFFSET*i, UINT16LEN);
        router_list[i].cost = ntohs(router_list[i].cost);
        memcpy(&(router_list[i].ipaddr), payload+INIT_PAYLOAD_HEAD_OFFSET+INIT_ADDR_OFFSET+INIT_PKT_OFFSET*i, UINT32LEN);
        router_list[i].ipaddr = ntohl(router_list[i].ipaddr);
        router_list[i].active = 1;
    }

    //initialize this router for routing table and receiving packets
    initial();

    print_info();/************************/
    return TRUE;
}

void init_response(int sock_index, char *payload)
{
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response;

	payload_len = 0; // initialize has no payload

    if(init_router(payload)){
        //the initialize is successful
        cntrl_response_header = create_response_header(sock_index, 1, 0, payload_len);
    }
    else{
        //the initialize is failed
        cntrl_response_header = create_response_header(sock_index, 1, 1, payload_len);
    }

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
}



