/******************************************************************************
*  SpouEx Wifi management File
*  Ver 1.0
*
*  Copyright (C) 2007 AlekMaul . All rights reserved.
*       http://www.portabledev.com
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>

#include <dswifi9.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "wifimanage.h"

//extern const unsigned short scrBottomOption_map[58][32];

// for some odd reason I can't seem to find vcount in ndslib -sigh- (I mean, in a place it doesn't conflict with other things)
#define		VCOUNT		(*((u16 volatile *) 0x04000006))

char szPersName[20];
char szWebPage[16384];
u8 bImgPng[32768];
u16 bImgDecomp[384*272];

//---------------------------------------------------------------------------------
// Dswifi stub functions
void * sgIP_malloc(int size) { return malloc(size); }
void sgIP_free(void * ptr) { free(ptr); }

// sgIP_dbgprint only needed in debug version
void sgIP_dbgprint(char * txt, ...) {		
}

// wifi timer function, to update internals of sgIP
void Timer_50ms(void) {
	Wifi_Timer(50);
}

// notification function to send fifo message to arm7
void arm9_synctoarm7() { // send fifo message
	REG_IPC_FIFO_TX=0x87654321;
}

// interrupt handler to receive fifo messages from arm7
void arm9_fifo() { // check incoming fifo messages
	u32 value = REG_IPC_FIFO_RX;
	if(value == 0x87654321) Wifi_Sync();
}

//******************************************************************************
//  Init wifi communication
//******************************************************************************
void wifiInit() { 
	// send fifo message to initialize the arm7 wifi
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; // enable & clear FIFO

	u32 Wifi_pass= Wifi_Init(WIFIINIT_OPTION_USELED);
	REG_IPC_FIFO_TX=0x12345678;
	REG_IPC_FIFO_TX=Wifi_pass;

	*((volatile u16 *)0x0400010E) = 0; // disable timer3

	irqSet(IRQ_TIMER3, Timer_50ms); // setup timer IRQ
	irqEnable(IRQ_TIMER3);
	irqSet(IRQ_FIFO_NOT_EMPTY, arm9_fifo); // setup fifo IRQ
	irqEnable(IRQ_FIFO_NOT_EMPTY);

	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ; // enable FIFO IRQ

	Wifi_SetSyncHandler(arm9_synctoarm7); // tell wifi lib to use our handler to notify arm7

	// set timer3
	*((volatile u16 *)0x0400010C) = -6553; // 6553.1 * 256 cycles = ~50ms;
	*((volatile u16 *)0x0400010E) = 0x00C2; // enable, irq, 1/256 clock

	while(Wifi_CheckInit()==0) { // wait for arm7 to be initted successfully
		while(VCOUNT>192); // wait for vblank
		while(VCOUNT<192);
	}
} // wifi init complete - wifi lib can now be used!

//******************************************************************************
//  Init wifi communication via WFC
//******************************************************************************
u8 wifiConnect(void) {
	int i;
	u8 uOK=0x00;

#ifdef DEBUG
	iprintf("Connecting via WFC data\n");
#endif
	// simple WFC connect:
	Wifi_AutoConnect(); // request connect
	while(1) {
		i=Wifi_AssocStatus(); // check status
		if(i==ASSOCSTATUS_ASSOCIATED) {
#ifdef DEBUG
			iprintf("Connected successfully!\n");
#endif
			uOK = 0x01;
			break;
		}
		if(i==ASSOCSTATUS_CANNOTCONNECT) {
#ifdef DEBUG
			iprintf("Could not connect!\n");
#endif
			uOK = 0x00;
			break;
		} 
	}
	return(uOK);
} // if connected, you can now use the berkley sockets interface to connect to the internet!



void wifiTest(void)
{
	if (wifiConnect()) {
		int sock;
		unsigned long ip;
		struct sockaddr_in servaddr;

		sock = socket(AF_INET, SOCK_STREAM, 0);	
		// On espere que sock != -1

		ip = *(unsigned long *) gethostbyname("htitt.mons.tec-wl.be")->h_addr_list[0];
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(80);
		servaddr.sin_addr.s_addr = ip;
		if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) == 0) {
			char szBuff[1024];
			int i = 1;
			u32 recvd_len;

			ioctl(sock, FIONBIO, &i);

			sprintf(szBuff, "GET /test.php HTTP/1.1\r\nHost: %s\r\nAccept: * /*\r\n\r\n", "htitt.mons.tec-wl.be");

			send(sock,szBuff,strlen(szBuff),0);	 // if recv returns 0, the socket has been closed.

			while( ( recvd_len = recv( sock, szBuff, 255, 0 ) ) != 0 ) {
				if(recvd_len>0) { // data was received!
					szBuff[recvd_len] = 0; // null-terminate
					strcat(szWebPage,szBuff);
					szWebPage[strlen(szWebPage)] = '\0';
				}
			}
		}
		if (sock) {
			close(sock);
		}
		Wifi_DisconnectAP();
	}


}
