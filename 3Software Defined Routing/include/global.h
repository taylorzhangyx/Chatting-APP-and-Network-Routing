#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


#define INF 65535
#define TCP 6
#define UDP 17
#define UINT16LEN 0x02
#define UINT32LEN 0x04

typedef enum {FALSE, TRUE} bool;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}

/* https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) // Interesting stuff to read if you are interested to know how this works

struct timeval *select_time;

uint16_t CONTROL_PORT,ROUTER_PORT,DATA_PORT;
uint32_t IP_ADDR;
uint16_t INTERVAL;
uint16_t NUM_ROUTER;
int num_neighbor;

struct router_info{
    uint16_t ID;
    uint16_t router_port;
    uint16_t data_port;
    uint16_t cost;
    uint16_t next_hop;
    uint32_t ipaddr;
    int update_count;
    int active;
} *router_list;

struct neighbor_info
{
    uint8_t active;
    uint16_t ID;
    uint16_t router_port;
    uint16_t data_port;
    uint32_t ipaddr;
} *neighbor_list;

struct routing_packet_info{
	uint32_t ipaddr;
	uint16_t router_port;
	uint16_t router_id;
	uint16_t cost;
} *routing_update_list;

#endif
