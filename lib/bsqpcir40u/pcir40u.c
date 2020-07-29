#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "pcir40u.h"
#include "../serial/serial.h"
#include "../public/public.h"

static int mlx_fd = -1;

//   AA AA 81 XX XX -------
static unsigned char mlx90621_frame[512] = { 0 };

static int TEMP_HIGH_Value = -3;

bool en_firmwarte_update = false;

// F1 F2 01 68 F3 F4 41
static int _MLX90621_frame_analysis( int size )
{
	unsigned short int *tp = mlx90621_frame + 2; // 温度大端模式

	TEMP_HIGH_Value = mlx90621_frame[2] * 0xFF + mlx90621_frame[3];
	TEMP_HIGH_Value *= 10;

//	leeDebugData(mlx90621_frame, size, 32, 2);

	return(TEMP_HIGH_Value);
}


/*****************************************************************
* MLX90621_RecvData_Event -- Get current temperature.
*
*
* INPUT:	port --  the uart port number
*
*
* OUTPUT:	Returns with valid temperature or fail value
*			-1: init uart failed.
*			-3: get temperature failed.
*			>0: current temperature       eg: 3640  convert to 36.4℃
*
*****************************************************************/

int MLX90621_RecvData_Event( int port )
{
	unsigned char	*p = NULL;
	int		num = 0, cnt = 0, tmout = 80;
	int		read_length = 0, total_length = 0;
	unsigned char	cmd1[] = { 0xAA, 0xBB, 0xF1, 0xF1 };

	if(en_firmwarte_update)
	{
		return -1;
	}

	if ( mlx_fd <= 0 )
	{
		mlx_fd = get_uard_fd( port );
		if ( mlx_fd <= 0 )
		{
			open_port( port, 9600, 8, "1", 'N' ); // fix baud is 9600
			mlx_fd = get_uard_fd( port );
			if ( mlx_fd <= 0 )
				return(-1);
		}
	}

// 发送指令
	uart_clear( port );
	UartWrite( mlx_fd, cmd1, sizeof(cmd1) );

//	LeeLog(NULL, cmd1, sizeof(cmd1) , 's');

	read_length	= 7;
	p		= mlx90621_frame;
	num		= UartRead( mlx_fd, p, read_length, tmout );
	if ( num == read_length && *p == 0xF1 && *(p + 1) == 0xF2 )
	{
//		LeeLog(NULL, p, num , 'r');
		_MLX90621_frame_analysis( 7 );
		return(TEMP_HIGH_Value);
	}

	TEMP_HIGH_Value = -3;

	return(TEMP_HIGH_Value);
}


// 波特率:9600,数据格式:0FFH,温度H,温度L,校验和;温度分辨率0.1度,校验和为高低字节的异或值,
// “FF 01 60 61”  0FFH,是数据起始; 01是温度H（高字节); 60是温度L（低字节);
// 61是01和60的异或值(校验和来的);(FF:是数据起始；0160是十六进制转为10进制是所得35.2度；61是01和60的异或值)。里面写有温度分辨率是0.1
//<---FF 01 6E 6F

static int dt8861_fd = -1;

int DT8861_RecvData_Event( int port )
{
	unsigned char buf[12] = "";

	unsigned char	*p = NULL;
	int		num = 0, cnt = 0, tmout = 80;
	int		read_length = 0, total_length = 0;
	int		baud = 9600; // // fix baud is 4800 for weds, 9600 for synel

	if ( dt8861_fd <= 0 )
	{
		dt8861_fd = get_uard_fd( port );
		if ( dt8861_fd <= 0 )
		{
			open_port( port, baud, 8, "1", 'N' );
			dt8861_fd = get_uard_fd( port );
			if ( dt8861_fd <= 0 )
				return(-1);
		}
	}

	read_length	= 4;
	p		= buf;
	num		= UartRead( dt8861_fd, p, read_length, tmout );
	if ( num == read_length && *p == 0xFF &&
	     *(p + 3) == (*(p + 1) ^ *(p + 2) ) )
	{
		leeDebugData( p, num, num, 2 );
		TEMP_HIGH_Value = (*(p + 1) << 8) + *(p + 2);
		TEMP_HIGH_Value *= 10;

		return(TEMP_HIGH_Value);
	}

	TEMP_HIGH_Value = -3;

	return(TEMP_HIGH_Value);
}


// 固件升级
static unsigned int crc16_modus( unsigned char *modbusdata, unsigned int Length )       //CRC16 MODBUS算法
{
	unsigned int	i, j;
	unsigned int	crc16 = 0xFFFF;
	for ( i = 0; i < Length; i++ )
	{
		crc16 ^= modbusdata[i];                                                 //CRC=BYTE xor CRC
		for ( j = 0; j < 8; j++ )
		{
			if ( (crc16 & 0x01) == 1 )                                      //如果CRC最后一位为1，右移一位后carry=1;
			{                                                               //则将CRC右移一位后，在与POLY16=0XA001进行xor运算
				crc16 = (crc16 >> 1) ^ 0XA001;
			}else  {                                                        //如果CRC最后一位为0，则只将CRC右移一位
				crc16 = crc16 >> 1;
			}
		}
	}
	return(crc16);
}

// AABB2233 查询版本号，返回格式 F1F2xxxxF2F4
int MLX90621_Get_Version(int port)
{		
	unsigned char	*p = NULL;
	int 	num = 0,tmout = 80;
	int 	read_length = 0;
	unsigned char	cmd1[] = { 0xAA, 0xBB, 0x22, 0x33 };
	unsigned char	rBuf[512] = "";
	bool redo = true;
	
	if ( mlx_fd <= 0 )
	{
		mlx_fd = get_uard_fd( port );
		if ( mlx_fd <= 0 )
		{
			open_port( port, 9600, 8, "1", 'N' ); // fix baud is 9600
			mlx_fd = get_uard_fd( port );
			if ( mlx_fd <= 0 )
				return(-1);
		}
	}

// 发送指令
REDO:
	uart_clear( port );
	UartWrite( mlx_fd, cmd1, sizeof(cmd1) );

	read_length = 7;
	p		= rBuf;
	num 	= UartRead( mlx_fd, p, read_length, tmout );
	if ( num == read_length && *p == 0xF2 && *(p + 1) == 0xF2 )   // 帧头修改为 F2F2 (文档未更新)
	{
		return (*(p+2)<<8) + *(p+3);
	}
	else if(redo)  // 第一帧丢弃
	{
		redo = false;
		goto REDO;
	}

	return -1;
	
}


// 输入ccdd5566-点击打开串口-点击发送，此后串口应该停止发送数据
static int MLX90621_Ready_Update(int port)
{		
	unsigned char	*p = NULL;
	int 	num = 0,tmout = 80;
	int 	read_length = 0;
	unsigned char	cmd1[] = { 0xCC, 0xDD, 0x55, 0x66 };
	unsigned char	rBuf[512] = "";
	
	if ( mlx_fd <= 0 )
	{
		mlx_fd = get_uard_fd( port );
		if ( mlx_fd <= 0 )
		{
			open_port( port, 9600, 8, "1", 'N' ); // fix baud is 9600
			mlx_fd = get_uard_fd( port );
			if ( mlx_fd <= 0 )
				return(-1);
		}
	}

// 发送指令
	uart_clear( port );
	UartWrite( mlx_fd, cmd1, sizeof(cmd1) );
	
	return -1;
	
}

// ret:  -1-串口打开失败; -2-文件操作失败; -3-启动升级失败; -4-升级失败

int MLX90621_Firmware_Update( int port, char *bin_name )
{
	unsigned char	BootLoader_CMD[]	= { 0X7E, 0X7E, 0X5A, 0X00, 0XB4, 0X42 };       //进入BootLoader指令, 请求固刷指令
	unsigned char	Confirm_CMD[]		= { 0X2B, 0X4F, 0X4B, 0XCF, 0X75 };             //数据传输正确指令
	unsigned char	Error_CMD[]		= { 0X2D, 0X45, 0X52, 0X52, 0X00, 0XA4 };       //数据传输错误指令
	unsigned char App_CMD[] = { 0X7E, 0X7E, 0X5A, 0x02, 0x75, 0XC3 };                       //下载完毕指令
	
	unsigned char datatosend[1034] = { 0X7E, 0X7E, 0X5A, 0x01, 0x00, 0x00, 0x00, 0x00, };   //发送数据缓冲区 连续发送1K，不足1K部分按实际长度发送

	unsigned char datatorecv[128] = "";

	int	packet_len	= 0;                                                            //总帧数
	int	packet_zheng	= 0;                                                            //程序整K字节个数
	int	packet_yu	= 0;                                                            //程序剩余不足1K字节个数
	int	packet_send	= 0;                                                            //已发送的整K字节个数

	int	ret = 0;
	int	i = 0, rsize = 0, tmout = 500;
	int Rtry_Times = 5;
	unsigned int CRC16 = 0;

	struct stat	fs;                                                                     // file stat
	FILE		*fp = NULL;

	en_firmwarte_update = true; // must first
	
	MLX90621_Ready_Update(port);

sleep(1); // must delay some time

	uart_close(port);
	mlx_fd = -1;

	open_port( port, 115200, 8, "1", 'N' ); // fix baud is 115200 when update
	mlx_fd = get_uard_fd( port );
	if ( mlx_fd <= 0 )
	{
		ret = -1;
		goto ec;
	}

	if ( !bin_name || stat( bin_name, &fs ) < 0 )
	{
		ret = -2;
		goto ec;
	}

	packet_zheng	= fs.st_size / 1024;
	packet_yu	= fs.st_size % 1024;
	packet_len	= packet_zheng + (packet_yu > 0 ? 1 : 0);

	if ( !(fp = fopen( bin_name, "r" ) ) )
	{
		ret = -2;
		goto ec;
	}

	UartWrite( mlx_fd, BootLoader_CMD, sizeof(BootLoader_CMD) );
	rsize = UartRead( mlx_fd, datatorecv, sizeof(datatorecv), tmout );
	if ( rsize < 5 || memcmp( datatorecv, Confirm_CMD, sizeof(Confirm_CMD) ) )
	{
		ret = -3;
		goto ec;
	}

	for ( i = 0; i < packet_len; i++ )
	{
		rsize = fread( datatosend + 8, 1024, 1, fp );
		datatosend[4]	= packet_send / 256; datatosend[5] = packet_send % 256; //数据帧序列号赋值，代表发送的是第几K数据
		datatosend[6]	= packet_len / 256;  datatosend[7] = packet_len % 256;	//总帧数
		
		CRC16			= crc16_modus( datatosend, 1032 );
		datatosend[1032]	= CRC16 / 256;									//取校验值低8位
		datatosend[1033]	= CRC16 % 256;									//取校验值高八位
RE_TRY:		
		printf("now, send package %d/%d\n", i+1,packet_len);
		UartWrite( mlx_fd, datatosend, sizeof(datatosend) );

		memset(datatorecv, 0, sizeof(datatorecv));
		rsize = UartRead( mlx_fd, datatorecv, sizeof(datatorecv), tmout );
		if ( memcmp( datatorecv, Confirm_CMD, sizeof(Confirm_CMD) ) )
		{
			if(Rtry_Times--)
			{
				printf("timeout ,retry again\n");
				goto RE_TRY;
			}
			ret = -4;
			goto ec;
		}
		packet_send++;
		Rtry_Times = 5;
	}

	UartWrite( mlx_fd, App_CMD, sizeof(App_CMD) ); // 固件更新完毕

ec:

	uart_close(port);
	mlx_fd = -1;
	en_firmwarte_update = false;
	if(fp) fclose(fp);
	return(ret);
}

