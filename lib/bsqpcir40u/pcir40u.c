#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcir40u.h"
#include "../serial/serial.h"
#include "../public/public.h"

static int mlx_fd = -1;

//   AA AA 81 XX XX -------
static unsigned char mlx90621_frame[512] = {0};

static int TEMP_HIGH_Value = -3;


// F1 F2 01 68 F3 F4 41 
static int _MLX90621_frame_analysis(int size)
{	
	unsigned short int *tp = mlx90621_frame + 2; // 温度大端模式

	TEMP_HIGH_Value = mlx90621_frame[2] * 0xFF + mlx90621_frame[3];	
	TEMP_HIGH_Value *= 10;
	
//	leeDebugData(mlx90621_frame, size, 32, 2);

	return TEMP_HIGH_Value;		
}


/*****************************************************************
 *MLX90621_RecvData_Event -- Get current temperature.
 *
 *
 *INPUT:	port --  the uart port number
 *
 *
 *OUTPUT:	Returns with valid temperature or fail value
 *			-1: init uart failed.
 *			-3: get temperature failed.
 *			>0: current temperature       eg: 3640  convert to 36.4℃
 *
 *****************************************************************/
 
int MLX90621_RecvData_Event( int port )
{
	unsigned char	*p = NULL;
	int num = 0, cnt = 0, tmout = 80;
	int read_length = 0, total_length = 0;
	unsigned char cmd1[] = {0xAA, 0xBB, 0xF1, 0xF1};

	if(mlx_fd <= 0)
	{
		mlx_fd = get_uard_fd(port);
		if(mlx_fd <= 0)
		{
			open_port(port, 9600, 8, "1", 'N');		// fix baud is 9600
			mlx_fd = get_uard_fd(port);
			if(mlx_fd <= 0)
			return -1;
		}
	}
	
// 发送指令
	uart_clear(port);
	UartWrite(mlx_fd, cmd1, sizeof(cmd1));
	
//	LeeLog(NULL, cmd1, sizeof(cmd1) , 's');

	read_length = 7;
	p = mlx90621_frame;	
	num = UartRead( mlx_fd, p, read_length, tmout );	
	if(num == read_length && *p == 0xF1 && *(p + 1) == 0xF2)
	{	
//		LeeLog(NULL, p, num , 'r');
		_MLX90621_frame_analysis(7);
		return TEMP_HIGH_Value;		
	}

	TEMP_HIGH_Value = -3;

	return TEMP_HIGH_Value;
	
}


// 波特率:9600,数据格式:0FFH,温度H,温度L,校验和;温度分辨率0.1度,校验和为高低字节的异或值,
// “FF 01 60 61”  0FFH,是数据起始; 01是温度H（高字节); 60是温度L（低字节);
// 61是01和60的异或值(校验和来的);(FF:是数据起始；0160是十六进制转为10进制是所得35.2度；61是01和60的异或值)。里面写有温度分辨率是0.1
//<---FF 01 6E 6F

static int dt8861_fd = -1;

int DT8861_RecvData_Event( int port )
{
	unsigned char	buf[12] = "";

	unsigned char	*p = NULL;
	int num = 0, cnt = 0, tmout = 80;
	int read_length = 0, total_length = 0;
	int baud = 9600; // // fix baud is 4800 for weds, 9600 for synel

	if(dt8861_fd <= 0)
	{
		dt8861_fd = get_uard_fd(port);
		if(dt8861_fd <= 0)
		{
			open_port(port, baud, 8, "1", 'N');			
			dt8861_fd = get_uard_fd(port);
			if(dt8861_fd <= 0)
			return -1;
		}
	}
	
	read_length = 4;
	p = buf;	
	num = UartRead( dt8861_fd, p, read_length, tmout );	
	if(num == read_length && *p == 0xFF && 
		*(p + 3) == (*(p + 1) ^ *(p + 2)))
	{	
		leeDebugData(p, num, num, 2);
		TEMP_HIGH_Value = (*(p + 1) << 8) + *(p + 2);
		TEMP_HIGH_Value *= 10;

		return TEMP_HIGH_Value;		
	}

	TEMP_HIGH_Value = -3;

	return TEMP_HIGH_Value;
	
}





