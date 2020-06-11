#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"



#define _2416

char key[16] = {"FFFFFFFFFFFF"};
char keymode=1; //1-Akey ,2-Bkey


int read_sectors()
{
	int i,len=0;
	char cardsno[16], tmpsno[32];
	
	unsigned char buf[16];
	
	int sector = 4, block = 0;

	while(1)
	{
		if(ReadCom1() == FALSE)	
		{
			continue;
		}
		
		memset(cardsno,0,sizeof(cardsno));
        if(ReadCard(cardsno) != 0X94)		// MF1 type
		{
			continue;
		}

		sprintf(tmpsno, "%08X", strtoul(cardsno, NULL, 10));
REDO:
		if(mifs_read(tmpsno, sector, block, buf,keymode,key) == FALSE)
		{
			break;
		}
		for(i=0; i<16; i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");

		block++;
		if(block < 3)
		{
			goto REDO;
		}		
		
		break;
	}

	return 0;
}



int main(int argc, char *argv[]){
	
	COMINFO cominfo[3];
	struct card_INFO cardinfo;
	int mode;

	printf("Usage: rwreadertest baud_table_index\r\n");
	printf("baud table: 2400,4800,9600,19200,38400,57600,115200\r\n");

	if(argc<2||atoi(argv[1])>6)	
	{
		printf("eg: ./main 6 --read\n");
		return -1;
	}
	
	cominfo[0].enable=0;
	cominfo[2].enable=0;//rs485
#if defined _2410
	cominfo[1].enable=1;//reader
	cominfo[1].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[1].workmode=1;
#elif defined _2416
	cominfo[1].enable=0; //gsm

	cominfo[3].type=1;//spi
	cominfo[3].enable=1;
	cominfo[3].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1;

#endif
	//read sectors

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}
	InitCard(&cardinfo);
	
	read_sectors();

	return 0;
}


