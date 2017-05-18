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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
#include <string.h>
#include <stdlib.h>
int Aseqnum = 0;
int Aacknum = 0;
int Bseqnum = 0;
int Backnum = 0;
char msg_buffer[1000][20];
int msgcount = 0;
int msgsend = 0;
int msgrecv = 0;

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
    //store message and count
    strcpy(msg_buffer[msgcount], message.data);
    msgcount++;

    if(msgsend == msgrecv){
        //creating sending packet
        struct pkt packet;
        //store sequence number and acknowledge number
        packet.seqnum = Aseqnum;
        packet.acknum = Aacknum;
        //get check sum and store it
        int checksum = Aseqnum + Aacknum;
        int i;
        for(i = 0; i<20; i++){
            checksum = checksum + (int)message.data[i];
        }
        packet.checksum = checksum;
        //copy message to payload
        strcpy(packet.payload, message.data);
        tolayer3(0,packet);
        starttimer(0,11.5);
        //mark one success sent message
        msgsend++;
    }


}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
    //check check sum first
    int checksum = 0;
    checksum = packet.seqnum + packet.acknum;
    int i = 0;
    for(i=0; i<20; i++){
        checksum = checksum + (int)packet.payload[i];
    }
    //if checksum matches
    if(checksum == packet.checksum){
        //check acknowledge number
        if(packet.acknum == Aacknum){
            //flip seqnum and acknum to next state
            Aseqnum = ~Aseqnum;
            Aacknum = ~Aacknum;
            //stop timer
            stoptimer(0);
            //mark one success receive message
            msgrecv++;
            //check msg buffer and counter
            if(msgcount > msgrecv){
                struct msg message;
                strcpy(message.data, msg_buffer[msgsend]);

                //creating sending packet
                struct pkt packet;
                //store sequence number and acknowledge number
                packet.seqnum = Aseqnum;
                packet.acknum = Aacknum;
                //get check sum and store it
                int checksum = Aseqnum + Aacknum;
                int i;
                for(i = 0; i<20; i++){
                    checksum = checksum + (int)message.data[i];
                }
                packet.checksum = checksum;
                //copy message to payload
                strcpy(packet.payload, message.data);
                tolayer3(0,packet);
                starttimer(0,11.5);
                //mark one success sent message
                msgsend++;
            }
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    //retransmission
    struct msg message;
    strcpy(message.data, msg_buffer[msgrecv]); //msgrecv here to point to msg need to be received
    //creating sending packet
    struct pkt packet;
    //store sequence number and acknowledge number
    packet.seqnum = Aseqnum;
    packet.acknum = Aacknum;
    //get check sum and store it
    int checksum = Aseqnum + Aacknum;
    int i;
    for(i = 0; i<20; i++){
        checksum = checksum + (int)message.data[i];
    }
    packet.checksum = checksum;
    //copy message to payload
    strcpy(packet.payload, message.data);
    tolayer3(0,packet);
    starttimer(0,11.5);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
    //check check sum first
    int checksum = 0;
    checksum = packet.seqnum + packet.acknum;
    int i = 0;
    for(i=0; i<20; i++){
        checksum = checksum + (int)packet.payload[i];
    }
    //if checksum matches
    if(checksum == packet.checksum){
        //check expected seq and acknum
        if(Bseqnum == packet.seqnum){
            char *data = (char*)malloc(sizeof(char)*20);
            strcpy(data,packet.payload);
            //send data to layer 5
            tolayer5(1,data);
            //move to next state
            Bseqnum = ~Bseqnum;
            Backnum = ~Backnum;
            //send back ack to sender A
            tolayer3(1,packet);
        }
        else{
            //not expected seq, send back
            tolayer3(1,packet);
        }
    }
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}
