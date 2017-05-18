/**
 * @router_table
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
 * Routing table response
 */


#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

#define ROUTING_TABLE_ENTRY_LEN 0x08
#define ROUTING_TABLE_NEXT_HOP_OFFSET 0x04
#define ROUTING_TABLE_COST_OFFSET 0x06

void fill_routing_table(char *payload)
{
    uint16_t router_id = 0;
    uint16_t next_hop_id = 0;
    uint16_t cost = 0;

    int i = 0;
    for(i=0; i<NUM_ROUTER; i++)
    {
        router_id = router_list[i].ID;
        next_hop_id = router_list[i].next_hop;
        cost = router_list[i].cost;

        router_id = htons(router_id);
        next_hop_id = htons(next_hop_id);
        cost = htons(cost);

        memcpy(payload + ROUTING_TABLE_ENTRY_LEN * i, &router_id, sizeof router_id);
        memcpy(payload + ROUTING_TABLE_ENTRY_LEN * i + ROUTING_TABLE_NEXT_HOP_OFFSET, &next_hop_id, sizeof next_hop_id);
        memcpy(payload + ROUTING_TABLE_ENTRY_LEN * i + ROUTING_TABLE_COST_OFFSET, &cost, sizeof cost);
    }
}


void routing_table_response(int sockfd)
{
    uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = ROUTING_TABLE_ENTRY_LEN*NUM_ROUTER;
	cntrl_response_payload = malloc(payload_len);
	fill_routing_table(cntrl_response_payload);

	cntrl_response_header = create_response_header(sockfd, 2, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE + payload_len;
	cntrl_response = malloc(response_len);

	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);

	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sockfd, cntrl_response, response_len);

	free(cntrl_response);
}
