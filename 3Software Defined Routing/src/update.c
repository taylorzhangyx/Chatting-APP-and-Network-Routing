/**
 * @update
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
 * update response
 */


#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

#define UPDATE_COST_OFFSET 0x02


bool update(char *payload)
{
    uint16_t router_id;
    uint16_t cost;

    memcpy(&router_id, payload, UINT16LEN);
    memcpy(&cost, payload + UPDATE_COST_OFFSET, UINT16LEN);

    router_id = ntohs(router_id);
    cost = ntohs(cost);

    int i = 0;
    for(i=0; i<NUM_ROUTER; i++)
    {
        if(router_id = router_list[i].ID){
            router_list[i].cost = cost;
            break;
        }
    }

    return TRUE;
}

void update_response(int sockfd, char *payload)
{
    uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response;

	payload_len = 0; // update has no payload

    if(update(payload)){
        //the update is successful
        cntrl_response_header = create_response_header(sockfd, 3, 0, payload_len);
    }
    else{
        //the update is failed
        cntrl_response_header = create_response_header(sockfd, 3, 1, payload_len);
    }

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);

	sendALL(sockfd, cntrl_response, response_len);

	free(cntrl_response);

}
