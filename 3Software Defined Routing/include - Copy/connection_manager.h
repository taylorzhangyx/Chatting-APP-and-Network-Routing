#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_


#include <sys/select.h>
#include <inttypes.h>
#include <strings.h>

#include "../include/global.h"

int control_socket, router_socket, data_socket;


void init();
struct timeval* refresh_select_time(int sec, int usec);
bool neighbor_satus();

#endif
