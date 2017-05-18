#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

int getchecksum(struct pkt p);

char msg_buffer[1500][20];//enlarge from 1000 to 1500
float msg_time[1500];
int Amsgcount = 0;
int Amsgsend = 0;
int Amsgrecv = 0;
int Bmsgrecv = 0; //the number of message B get
int winsize = 0;
float RTT = 0;

struct pkt newpacket;

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
    //store message to buffer, count message
    strcpy(msg_buffer[Amsgcount],message.data);
    //find if can send **this** message
    if(Amsgcount < (Amsgrecv + winsize)){
        //packet this message and send
        //store sequence number and acknowledge number
        newpacket.seqnum = Amsgcount;//get the sequence number of THIS message
        newpacket.acknum = 0; //reset the acknum when sending
        //copy message to payload
        strcpy(newpacket.payload, message.data);
        //get check sum and store it
        newpacket.checksum = getchecksum(newpacket);

        msg_time[newpacket.seqnum] = get_sim_time();
        tolayer3(0,newpacket);
        //check if this message is the first packet in window
        if(Amsgrecv == Amsgcount){
            //if yes, the timer is stopped. Start timer based on winsize
            starttimer(0,RTT);
        }
        //mark one success sent message
        Amsgsend++;
    }
    Amsgcount++;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    //check check_sum first
    //if checksum matches
    if(getchecksum(packet) == packet.checksum){
        //ignore duplicate acknowledgment
        if(packet.acknum != Amsgrecv - 1){
            if(Amsgrecv <= packet.acknum){
                //move window
                Amsgrecv = packet.acknum+1;
                stoptimer(0);
                //if there are packet on flight
                if(Amsgrecv < Amsgsend){
                    float interTimer = get_sim_time() - msg_time[Amsgrecv];
                    printf("333333333333333\n");
                    if(interTimer < RTT){
                            printf("2222222222222\n");
                        starttimer(0, (RTT-interTimer));
                    }
                    else{
                        printf("111111111\n");
                        int counter3 = 0;
                        for(counter3 = Amsgrecv; counter3 < Amsgsend; counter3++){
                                printf("4444444444+%d\n",counter3);
                            newpacket.seqnum = counter3;
                            newpacket.acknum = 0;
                            strcpy(newpacket.payload, msg_buffer[counter3]);
                            newpacket.checksum = getchecksum(newpacket);
                            //update sending time
                            msg_time[newpacket.seqnum] = get_sim_time();
                            tolayer3(0,newpacket);
                        }
                        starttimer(0, RTT);
                    }
                }

                //check if can send more packets
                if(Amsgsend < Amsgrecv + winsize){
                    //check if there are more packets can be sent
                    if(Amsgcount > Amsgsend){
                        int upper_bound = min(Amsgrecv + winsize, Amsgcount);
                        int counter = 0;

                        for(counter = Amsgsend; counter < upper_bound; counter ++){
                            newpacket.seqnum = counter;
                            newpacket.acknum = 0;
                            strcpy(newpacket.payload, msg_buffer[counter]);
                            newpacket.checksum = getchecksum(newpacket);
                            //record sending time
                            msg_time[newpacket.seqnum] = get_sim_time();
                            printf("4777777777\n");
                            tolayer3(0,newpacket);
                        }
                        Amsgsend = upper_bound;
                    }
                }
            }
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    printf("66666666666666666\n");
    int counter2 = 0;
    for(counter2 = Amsgrecv; counter2 < Amsgsend; counter2++){
        struct pkt newpacket4;
        newpacket4.seqnum = counter2;
        newpacket4.acknum = 0;
        strcpy(newpacket4.payload, msg_buffer[counter2]);
        newpacket4.checksum = getchecksum(newpacket4);
        //update sending time
        msg_time[newpacket4.seqnum] = get_sim_time();
        tolayer3(0,newpacket4);
    }
    starttimer(0, RTT);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    winsize = getwinsize();
    RTT = 12.5 + winsize/10;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    //check check_sum first
    //if checksum matches
    if(getchecksum(packet) == packet.checksum){
        //check whether this packet is in sequence
        if(Bmsgrecv == packet.seqnum){
            //store message, msgrecv++
            char *data = (char*)malloc(sizeof(char)*20);
            strcpy(data,packet.payload);
            //send data to layer 5
            tolayer5(1,data);
            //number of receive in B increment
            Bmsgrecv++;
            //store acknowledgment number back, which is Bmsgrecv - 1
            packet.acknum = (Bmsgrecv - 1);
            //update check sum and store it
            packet.checksum = getchecksum(packet);
            //send acknowledgment back
            tolayer3(1,packet);
        }
        else{
            //store acknowledgment number back, which is Bmsgrecv - 1,
            //point to the last in order sequence number
            printf("B side lost\n");
            packet.acknum = Bmsgrecv-1;
            //update check sum and store it
            packet.checksum = getchecksum(packet);
            //send packet back
            tolayer3(1,packet);
        }
    }
    //if some bits corrupts, send the last in order seq number back
    else{
        //store acknowledgment number back, which is Bmsgrecv - 1,
        //point to the last in order sequence number
        printf("B side currption\n");
        packet.acknum = Bmsgrecv-1;
        //update check sum and store it
        packet.checksum = getchecksum(packet);
        //send packet back
        tolayer3(1,packet);
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}

/*
*   The sub function calculate the checksum
*/

int getchecksum(struct pkt p){
    int tempchecksum = p.acknum + p.seqnum;
    int i=0;
    for(i = 0; i<20; i++){
        tempchecksum = tempchecksum + (int)p.payload[i];
    }
    return tempchecksum;
}
