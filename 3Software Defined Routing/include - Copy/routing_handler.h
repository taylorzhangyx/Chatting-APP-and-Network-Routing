#ifndef ROUTING_HANDLER_H_
#define ROUTING_HANDLER_H_




void send_routing_table(int sockfd);
char* make_routing_packet();
void writedown_neighbor_table();
bool update_routing_table(uint16_t number_router, uint32_t source_ip);
void process_routing_packet(char *packet);
bool recv_routing_packet(int sockfd);

#endif
