#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"


#define _2416

int main( int argc, char *argv[] )
{
	printf( "Usage: powertest baud_table_index\r\n" );
	printf( "baud table: 2400,4800,9600,19200,38400,57600,115200\r\n" );

	COMINFO cominfo[4];

	if ( argc < 2 || atoi( argv[1] ) > 6 )
	{
		printf( "pls input baud rate\n" );
		return(-1);
	}

	cominfo[0].enable	= 0;
	cominfo[1].enable	= 0;                    //rs485	
	cominfo[2].enable 	= 0;                          //gsm

	cominfo[3].type		= 1;                    //0 = uart 1 = spi reader
	cominfo[3].enable	= 1;
	cominfo[3].baudrate	= atoi( argv[1] );      //2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode	= 1;                    //answer


	if ( OpenCom( cominfo ) == FALSE )              // Initialize serial port
	{
		perror( "com init error" );
		return(-1);
	}

	device_power_off();

	UnCom( 3 + 10 );

	return(0);
}

