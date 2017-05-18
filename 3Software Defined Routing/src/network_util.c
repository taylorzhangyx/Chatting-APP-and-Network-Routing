/**
 * @network_util
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
 * Network I/O utility functions. send/recvALL are simple wrappers for
 * the underlying send() and recv() system calls to ensure nbytes are always
 * sent/received.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

ssize_t recvALL(int sock_index, char *buffer, ssize_t nbytes)
{
    ssize_t bytes = 0;
    bytes = recv(sock_index, buffer, nbytes, 0);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += recv(sock_index, buffer+bytes, nbytes-bytes, 0);

    return bytes;
}

ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes)
{
    ssize_t bytes = 0;
    bytes = send(sock_index, buffer, nbytes, 0);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += send(sock_index, buffer+bytes, nbytes-bytes, 0);

    return bytes;
}

ssize_t recvfromALL(int sock_index, char *buffer, ssize_t nbytes, struct sockaddr *from, int *fromlen)
{
    ssize_t bytes = 0;
    bytes = recvfrom(sock_index, buffer, nbytes, 0, from, fromlen);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += recvfrom(sock_index, buffer+bytes, nbytes-bytes, 0, from, fromlen);

    return bytes;
}

ssize_t sendtoALL(int sock_index, char *buffer, ssize_t nbytes, const struct sockaddr *destination, socklen_t desination_len)
{
    ssize_t bytes = 0;
    bytes = sendto(sock_index, buffer, nbytes, 0, destination, desination_len);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        {bytes += sendto(sock_index, buffer+bytes, nbytes, 0, destination, desination_len);}
    printf("sendtoALL finished\n");
    return bytes;
}
