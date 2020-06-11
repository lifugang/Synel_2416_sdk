#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

#define _2416


#define ID_ANT  0
#define IC_ANT  1
 
#define Time_on 3
#define Time_off 6 
 
// eg for IC_ANT
 
int ant_test()
{
	int ret = -1;
	int cur_ant_status = 1;
	// 0-off, 1-on
	struct timespec curtime = {0, 0};
	 
	set_antenna_status(ID_ANT, cur_ant_status); // init IC Antenna On
	 
	clock_gettime(CLOCK_MONOTONIC, &curtime); // init timer
	
	while(1)
	{
	 	ReadCom1();

		if(cur_ant_status == 1)
		{
			if(RelativeTimer(curtime, Time_off * 1000) > 0) // timeout
			{
				cur_ant_status = 0;
				ret = set_antenna_status(ID_ANT, cur_ant_status);
				if(ret != TRUE)
				{
					printf("1set_antenna_status %d err\n", cur_ant_status);
				}
				else
				{
					printf("1set_antenna_status %d ok....\n", cur_ant_status);
				}				
				clock_gettime(CLOCK_MONOTONIC, &curtime);
			}
		}
		else
		{
			if(RelativeTimer(curtime, Time_on * 1000) > 0) // timeout
			{
				cur_ant_status = 1;
				ret = set_antenna_status(ID_ANT, cur_ant_status);
				if(ret != TRUE)
				{
					printf("2set_antenna_status %d err\n", cur_ant_status);
				}
				else
				{
					printf("2set_antenna_status %d ok....\n", cur_ant_status);
				}				
				clock_gettime(CLOCK_MONOTONIC, &curtime);
			}
		}
	 
	}
	
	return 0;
 
}


int main(int argc, char *argv[]){

	printf("Usage: readertest baud_table_index\r\n");
	printf("baud table: 2400,4800,9600,19200,38400,57600,115200\r\n");

	char buf[128],cardsno[128],ver[32] = "",*Pwg=NULL;
	int cardtype,wgtype=0,tmp=0,i=0,len=0;
	unsigned long  int data=0;
	unsigned long  long int Dat=0;
	int status = 1;
	int ret = -1;
	static int cnt = 0;
	COMINFO cominfo[4];
	struct timespec curtime = {0, 0};

	struct card_INFO cardinfo;

	if(argc<2||atoi(argv[1])>6)	
	{
		printf("pls input baud rate\n");
		return -1;
	}

	cominfo[0].enable=0;
	cominfo[1].enable=0;//gsm
	cominfo[2].enable=0;//rs485

	cominfo[3].enable=1;
	cominfo[3].type = 1; //0 = uart 1 = spi	
	cominfo[3].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1; //answer
	
	cardinfo.track=2;

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}


	if(get_keyboardVer(ver)==TRUE)
	{
		printf("MCU ver = [%s]\n",ver);
	}


	ant_test();

	return 0;

	ret = set_antenna_status(0, status);
	if(ret != TRUE)
	{
		printf("set_antenna_status %d err\n", status);
	}
	else
	{
		printf("set_antenna_status %d ok....\n", status);
	}

	InitCard(&cardinfo);

	while(1)
	{
		if(curtime.tv_sec != 0 && RelativeTimer(curtime, 10 * 1000))	
		{
			curtime.tv_sec = 0;
			status = 1;
			ret = set_antenna_status(0, status);
			if(ret != TRUE)
			{
				printf("set_antenna_status %d err\n", status);
			}
			else
			{
				printf("set_antenna_status %d ok....\n", status);
			}			
		}
		
		if(ReadCom1()==FALSE)	continue;

		memset(buf,0,sizeof(buf));
		
		cardtype=ReadCard(buf);

		if(cardtype!=0)

		printf("ret cmd: %2X\n",cardtype);
		
		switch(cardtype)
		{
			case -1:printf("read magetic failure\n");break;
			case -2:printf("read barcode decode failure\n");break;
			case -3:printf("read barcode failure\n");break;
			case -4:printf("read magetic decode failure\n");break;
			case -5:printf("unknown error\n");break;
			case -6:printf("read EM decode failure\n");break;
			case -7:printf("read EM failure\n");break;
			case -8:printf("read MF1 decode failure\n");break;
			case -9:printf("read MF1 failure\n");break;
			case -10:printf("read HID decode failure\n");break;
			case -11:printf("read HID failure\n");break;
			case 0x8C: close_battery_power_report();break;
			case 0X9C: //HID
					len=buf[0];
					wgtype=buf[1];
					Pwg=buf+2;
					printf("wgtype %d;len=%d\n",wgtype,len);
					switch(wgtype)
					{
					case 26:	//wg26 26bit; 4 byte	
						data=0;
						for(i=0;i<len-1;i++){
							data=(data<<8)|Pwg[i];
							//printf("%X,%X\n",data,Pwg[i]);
						}
						data=data>>(sizeof(data)*8-wgtype);
						data=(data&0x1FFFFFE)>>1;//get wg valid bit ;(clear odd bit/even bit) you can calc these
						
						printf("%02X,%02X,%02X,%02X\n",Pwg[0],Pwg[1],Pwg[2],Pwg[3]);
						printf("cardno %05ld\n",data&0XFFFF);
						break;
						/*
						example:
						B4,C1,07,80
						----bit---->
						10110100 11000001 00000111 10000000
						-----get 26bit ---->
						10110100 11000001 00000111 10
						10 11010011 00000100 00011110
						*/
					case 34://wg34,you can add it	
					printf("aa\n");
						data=0;
						for(i=0;i<len-1;i++){
							data=(data<<8)|Pwg[i];
							printf("%ld,%02X\n",data,Pwg[i]);
						}
						data=data>>(sizeof(data)*8-wgtype);
						data=(data&0x1FFFFFE)>>1;//get wg valid bit ;(clear odd bit/even bit) you can calc these
						
						printf("%02X,%02X,%02X,%02X\n",Pwg[0],Pwg[1],Pwg[2],Pwg[3]);
						printf("cardno %05ld\n",data&0XFFFF);				
						break;
					}
					break;
			case 0x97://voltage wiegand
				printf("wiegand \r\n");
				len=buf[0];
				wgtype=buf[1];
				Pwg=buf+2;
				printf("wgtype %d;len=%d\n",wgtype,len);
				switch(wgtype)
				{
				case 26:	//wg26 26bit; 4 byte
					data=0;
					for(i=0;i<len-1;i++){
						data=(data<<8)|Pwg[i];
						//printf("%X,%X\n",data,Pwg[i]);
					}
					data=data>>(sizeof(data)*8-wgtype);
					data=(data&0x1FFFFFE)>>1;//get wg valid bit ;(clear odd bit/even bit) you can calc these

					printf("%02X,%02X,%02X,%02X\n",Pwg[0],Pwg[1],Pwg[2],Pwg[3]);
					printf("cardno %05ld\n",data&0XFFFFFF);
					break;
					/*
					example:
					B4,C1,07,80
					----bit---->
					10110100 11000001 00000111 10000000
					-----get 26bit ---->
					10110100 11000001 00000111 10
					10 11010011 00000100 00011110
					*/
				case 34://wg34,you can add it
					Dat=0;
					for(i=0;i<len-1;i++){
						Dat=(Dat<<8)|Pwg[i];
						printf("%lld,%02X\n",Dat,Pwg[i]);
					}
					Dat=Dat>>(5*8-wgtype);
					Dat=(Dat&0x1FFFFFFFE)>>1;//get wg valid bit ;(clear odd bit/even bit) you can calc these
					
					printf("%02X,%02X,%02X,%02X,%02X\n",Pwg[0],Pwg[1],Pwg[2],Pwg[3],Pwg[4]);
					printf("cardno %lld\n",Dat&0XFFFFFFFF);				
					break;					

				}
				break;
			case 0x98:break;//power mode
			case 0x90:
				printf("get key...\n");			
				break;
			case 0:break;
			break;
		}

		if(cardtype==0X93)
		{
			memset(cardsno,0,sizeof(cardsno));
			card_linearhandle(buf,cardsno);
			printf("card no.:%s\n",cardsno);
			memset(cardsno,0,sizeof(cardsno));
			card_synelhandle(buf,cardsno);
			printf("card no.:%s\n",cardsno);

			cnt++;
			if(cnt == 4) // now test disable antenna
			{
				cnt = 0;
				status = 0;
				clock_gettime(CLOCK_MONOTONIC, &curtime);
				ret = set_antenna_status(0, status);
				if(ret != TRUE)
				{
					printf("set_antenna_status %d err\n", status);
				}
				else
				{
					printf("set_antenna_status %d ok....\n", status);
				}				
			}
				
		}

		//usleep(200000);
	}

#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);
#endif
	return 0;
}
