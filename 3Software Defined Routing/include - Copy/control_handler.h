#ifndef CONTROL_HANDLER_H_
#define CONTROL_HANDLER_H_



int create_sock(uint16_t port, int prorocol);
int new_control_conn(int sock_index);
bool isControl(int sock_index);
int control_recv_hook(int sock_index);
void prepare_port(int *router_socket, int *data_socket);

#endif
