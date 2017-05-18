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

int winsize = 0;
int Amsgcount = 1;
int Amsgsend = 1;
int Amsgrecv = 1;

//int Bmsgcount = 1; //the biggest seq number in buffer that has stored
int Bmsgpush = 1; //the latest seq number in order which already push to layer 5

double RTT = 0;

struct sendpktinfo{
    char data[20];
    int flag; //0:no use, 1:in use timer start, 2:stop use complete
    int timer;
};
struct recvpktinfo{
    char data[20];
    int flag;
};

struct sendpktinfo Amsgbuffer[1100]; //store info of pkts in A side
struct recvpktinfo Bmsgbuffer[1100]; //store info of pkts in B side
struct pkt newpacket;


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
    //store message to buffer, count message
    memcpy(Amsgbuffer[Amsgcount].data,message.data,20);
    Amsgcount++;
    //check if there are more message can be sent
    //window is not full && more unsent message
    if(((Amsgsend - Amsgrecv) < winsize) && (Amsgcount > Amsgsend)){
        int counter = 0;
        int upper_bound = min(Amsgrecv + winsize, Amsgcount);
        for(counter = Amsgsend; counter < upper_bound; counter++){
            //package this packet
            newpacket.seqnum = counter;
            newpacket.acknum = 0;
            memcpy(newpacket.payload, Amsgbuffer[counter].data,20);
            newpacket.checksum = getchecksum(newpacket);
            //send out
            tolayer3(0,newpacket);
            //set timer flag to 1
            Amsgbuffer[counter].flag = 1;
        }
        //update Amsgsend
        Amsgsend = upper_bound;
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    //check checksum
    if(packet.checksum == getchecksum(packet)){
        //check corresponding package flag
        if(Amsgbuffer[packet.acknum].flag == 1){
            //set flag to 2: complete translate
            Amsgbuffer[packet.acknum].flag = 2;
            //check and move sending window
            while(Amsgbuffer[Amsgrecv].flag == 2){
                Amsgrecv ++;
            }
            //check more message to send
            if(((Amsgsend - Amsgrecv) < winsize) && (Amsgcount > Amsgsend)){
                int counter = 0;
                int upper_bound = min(Amsgrecv + winsize, Amsgcount);
                for(counter = Amsgsend; counter < upper_bound; counter++){
                    //package this packet
                    newpacket.seqnum = counter;
                    newpacket.acknum = 0;
                    memcpy(newpacket.payload, Amsgbuffer[counter].data,20);
                    newpacket.checksum = getchecksum(newpacket);
                    //send out
                    tolayer3(0,newpacket);
                    //set timer flag to 1
                    Amsgbuffer[counter].flag = 1;
                }
                //update Amsgsend
                Amsgsend = upper_bound;
            }
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    //shift every valid timer
    int upper_bound = min(Amsgrecv + winsize, Amsgcount);
    int shifter = Amsgrecv;
    for(; shifter < upper_bound; shifter++){
        //if in flight
        if(Amsgbuffer[shifter].flag == 1){
            //change time
            Amsgbuffer[shifter].timer--;
            //check timeout
            if(Amsgbuffer[shifter].timer==0){
                //do the retransmission
                //package this packet
                newpacket.seqnum = shifter;
                newpacket.acknum = 0;
                memcpy(newpacket.payload, Amsgbuffer[shifter].data,20);
                newpacket.checksum = getchecksum(newpacket);
                //send out
                tolayer3(0,newpacket);
                //reset timer
                Amsgbuffer[shifter].timer = RTT;
            }
        }
    }
    starttimer(0,1);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    winsize = getwinsize();
    RTT = 12 + winsize/200;
    int i = 0;
    //set every buffer flag and timer
    for(; i<1100; i++){
        Amsgbuffer[i].flag = 0;
        Amsgbuffer[i].timer = RTT;
    }
    starttimer(0,1);
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    //check checksum first
    if(packet.checksum == getchecksum(packet)){
        //seq between Bmsgpush and Bmsgpush+window size
        if((packet.seqnum >= Bmsgpush) && (packet.seqnum < (Bmsgpush+winsize))){
            //can store this message
            //check if already in buffer
            if(Bmsgbuffer[packet.seqnum].flag == 0){
                //not in buffer
                //store this message
                memcpy(Bmsgbuffer[packet.seqnum].data,packet.payload,20);
                //set flag that received
                Bmsgbuffer[packet.seqnum].flag = 1;
                //update Bmsgcount
                //Bmsgcount = max(packet.seqnum,Bmsgcount);
            }

            //send ack back
            packet.acknum = packet.seqnum;
            //update check sum and store it
            packet.checksum = getchecksum(packet);
            //send acknowledgment back
            tolayer3(1,packet);

            //check next data is ready to push
            while(Bmsgbuffer[Bmsgpush].flag == 1){
                //ready to push, package this message and push
                tolayer5(1,Bmsgbuffer[Bmsgpush].data);
                Bmsgpush++;
            }

/**
            if(Bmsgbuffer[Bmsgpush].flag == 1){
                int counter = Bmsgpush;
                //go through the buffer to push
                for(; counter < Bmsgcount; counter++){
                    //current message is ready to push, copy and push data
                    char *data = (char*)malloc(sizeof(char)*20);
                    memcpy(data,Bmsgbuffer[counter].data,20);
                    //send data to layer 5
                    tolayer5(1,data);
                    //check next message if ready, if flag == 1, ready to push,do nothing
                    //if flag != 1, not ready, break loop
                    if(Bmsgbuffer[counter+1].flag != 1){
                        break;
                    }
                }
                //update Bmsgpush
                Bmsgpush = counter;
            }
            **/
        }
        //seq between Bmsgpush - window size and Bmsgpush, already pushed
        else if((packet.seqnum < Bmsgpush) && (packet.seqnum >= (Bmsgpush - winsize))){
            //send ack back
            packet.acknum = packet.seqnum;
            //update check sum and store it
            packet.checksum = getchecksum(packet);
            //send acknowledgment back
            tolayer3(1,packet);
        }
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    int i = 0;
    for(; i<1100; i++){
        Bmsgbuffer[i].flag = 0;
    }
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
