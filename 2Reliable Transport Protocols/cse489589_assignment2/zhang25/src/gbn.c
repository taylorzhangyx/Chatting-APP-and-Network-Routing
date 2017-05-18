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

int getchecksum(struct pkt p);

char msg_buffer[1000][20];
double msg_time[1000];
int Amsgcount = 0;
int Amsgsend = 0;
int Amsgrecv = 0;
int Bmsgrecv = 0; //the number of message B get
int winsize = 0;
double RTT = 0;

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
    //store message to buffer, count message
    strcpy(msg_buffer[Amsgcount],message.data);
    Amsgcount++;
    //find if can send **this** message
    if(Amsgcount < (Amsgrecv + winsize)){
        //packet this message and send
        struct pkt packet;
        //store sequence number and acknowledge number
        packet.seqnum = Amsgcount - 1;//get the sequence number of THIS message
        packet.acknum = 0; //reset the acknum when sending
        //get check sum and store it
        int checksum = Aseqnum + Aacknum;
        int i;
        for(i = 0; i<20; i++){
            checksum = checksum + (int)message.data[i];
        }
        packet.checksum = checksum;
        //copy message to payload
        strcpy(packet.payload, message.data);
        msg_time[packet.seqnum] = get_sim_time();
        tolayer3(0,packet);
        //check if this message is the first packet in window
        if(Amsgrecv == (Amsgcount - 1)){
            //if yes, the timer is stopped. Start timer based on winsize
            starttimer(0,RTT);
        }
        //mark one success sent message
        msgsend++;
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    //check check_sum first
    int checksum = getchecksum(packet);
    //if checksum matches
    if(checksum == packet.checksum){
        //ignore duplicate acknowledgment
        if(packet.acknum != Amsgrecv - 1){
            if(Amsgrecv <= packet.acknum){
                //move window
                Amsgrecv = packet.acknum+1;
                stoptimer(0);
                double interTimer = get_sim_time() - msg_time[Amsgrecv];
                if(interTimer < RTT){
                    starttimer(0, RTT - interTimer);
                }
                else{
                    A_timerinterrupt();
                }
                //check if can send more packets
                if(Amsgsend < Amsgrecv + winsize){
                    //check if there are more packets can be sent
                    if(Amsgcount > Amsgsend){
                        int upper_bound = min(Amsgrecv + winsize, Amsgcount);
                        int counter = 0;
                        struct pkt newpacket;
                        for(counter = Amsgsend; counter < upper_bound; counter ++){
                            newpacket.seqnum = counter;
                            newpacket.acknum = 0;
                            strcpy(newpacket.payload, msg_buffer[counter]);
                            newpacket.checksum = getchecksum(newpacket);
                            //record sending time
                            msg_time[newpacket.seqnum] = get_sim_time();
                            tolayer3(newpacket);
                        }
                    }
                }
            }
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    int counter = 0;
    struct pkt newpacket;
    for(counter = msgrecv; counter < msgsend; counter ++){
            newpacket.seqnum = counter;
            newpacket.acknum = 0;
            strcpy(newpacket.payload, msg_buffer[counter]);
            newpacket.checksum = getchecksum(newpacket);
            //update sending time
            msg_time[newpacket.seqnum] = get_sim_time();
            tolayer3(newpacket);
    }
    starttimer(0, RTT);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    winsize = getwinsize();
    RTT = 12.5 + winsize/20;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    //check check_sum first
    int checksum = getchecksum(packet);
    //if checksum matches
    if(checksum == packet.checksum){
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
    int checksum = p.acknum + p.seqnum;
    int i;
    for(i = 0; i<20; i++){
        checksum = checksum + (int)packet.payload[i];
    }
    return checksum;
}
