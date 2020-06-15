/*
 * serial.c
 *
 *  Created on: 2013-7-2
 *      Author: aduo
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"
#include "uart/uartlib.h"
#include "spi/spilib.h"

int serial_fd[MAXCOM];

//return -1 or 1
int open_port (int port, int baudrate, int databit,
                   char *stopbit, char parity){
	int err_code;

    if (port < 10){
    	err_code = uart_open_and_setattr(port,baudrate,databit,stopbit,parity);
    }else{
    	err_code = spi_open(port,baudrate);
    }

    return err_code;

}

int read_port(int port, void *data, int datalength) {
	int len = 0;


	//printf("fd %d \r\n",serial_fd[port]);
	if (serial_fd[port] < 0) {
		return -1;
	}

	if (port < 10) {
		len = uart_recv_data(port, data, datalength);
	} else {
		len = spi_recv_data(port, data, datalength);
	}

	return len;
}

int write_port (int port, char * data, int datalength){
	int len = 0;

	if (serial_fd[port] < 0) {
		return -1;
	}

	if (port < 10){
		len = uart_send_data(port,data,datalength);
	}else{
		len = spi_send_data(port,data,datalength);
	}
	return len;
}

void close_port (int port){
	if (port < 10){
		uart_close(port);
	}else{
		spi_close(port);
	}
}

int clear_port(int port)
{
    if(serial_fd[port]<=0)
    {
        return 0;
    }

    if(port < 10)
    {
        return uart_clear(serial_fd[port]);
    }
    else
    {
        return spi_clear(serial_fd[port]);
    }

    return 1;
}

int get_uard_fd( int port )
{
	return serial_fd[port];
}

int UartWrite( int comfd, unsigned char *iBuf, int iLen )
{
	unsigned char	* addr = NULL;
	int		len;
	int		num = 0, count = 0;

	struct timeval	tout;
	fd_set		write_fd, read_fd;

	if ( comfd <= 0 || !iBuf || iLen <= 0 )
		return(-1);

//	leeDebugData(iBuf, iLen, iLen, 1);

	FD_ZERO( &write_fd );
	FD_SET( comfd, &write_fd );

	while ( iLen != num )
	{
		tout.tv_sec	= 0;
		tout.tv_usec	= 50000;
		if ( select( comfd + 1, NULL, &write_fd, NULL, &tout ) <= 0 )
		{
			return(-1);
		}
		addr	= iBuf + num;
		count	= write( comfd, addr, iLen - num );
		if ( count < 0 )
		{
			return(-1);
		}
		num += count;
	}

//	dbgShowHexData(iBuf, iLen, 1, '>', "SEND");

	return(num);
}


// oTime: 超时时间 单位 ms
int UartRead( int comfd, unsigned char *oBuf, int oLen, unsigned int oTime )
{
	unsigned char	* addr = NULL;
	int		len, i;
	int		num = 0, count = 0;
	unsigned char	buf[2048];

	struct timeval	tout;
	fd_set		write_fd, read_fd;

	if ( comfd <= 0 || !oBuf || oLen < 0 )
		return(-1);

	FD_ZERO( &read_fd );
	FD_SET( comfd, &read_fd );

	while ( oLen != num )
	{
		tout.tv_sec	= 0;
		tout.tv_usec	= oTime * 1000;
		if ( select( comfd + 1, &read_fd, NULL, NULL, &tout ) <= 0 )
		{
			goto END;
		}
		addr	= oBuf + num;
		count	= read( comfd, addr, oLen - num );
		if ( count < 0 )
		{
			goto END;
		}
		num += count;
	}
END:

//	if(num)
//	leeDebugData(oBuf, num, num, 2);

	return(num);
}

