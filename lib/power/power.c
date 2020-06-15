/*
 * power.c
 *
 *  Created on: 2013-7-11
 *      Author: aduo
 */


#include "../_precomp.h"
#include "../public/initcom.h"
#include "../public/protocol.h"
#include "power.h"

struct _battery battery;

int get_battery_power_status()
{
	unsigned char status;

	battery.status = bs_unknown;

	switch ( com1value.instruction )
	{
	case 0x8C:
		status = com1value.user_data[0];
		if ( status == 110 )
		{
			battery.status = bs_low_voltage;
		}else if ( status == 120 )
		{
			battery.status = bs_none;
		}else if ( status == 130 )
		{
			battery.status = bs_charging;
		}else if ( status == 140 )
		{
			battery.status = bs_charged;
		}else if ( status == 150 )
		{
			battery.status = bs_high_voltage;
		}else{
			battery.status = bs_discharging;
		}
		break;
	}
	return(battery.status);
}


int get_battery_power_energy()
{
	unsigned char status;

	switch ( com1value.instruction )
	{
	case 0x8C:
		status = com1value.user_data[0];
		if ( status > 0 && status < 100 )
		{
			battery.energy = status;
		}else{
			battery.energy = 0;
		}
		break;
	}
	return(battery.energy);
}


int close_battery_power_report()
{
	_TData asker;

	memset( &asker, 0, sizeof(asker) );
	asker.address		= 0X01;
	asker.instruction	= 0x8C;
	asker.nbytes		= 0X03;
	asker.itemnum		= 0x01;
	asker.user_data[0]	= 0xFF;

#if defined _2410
	if ( _put_data( serialport1, &asker ) != SUCCESS )
		return(FALSE);
#elif defined _2416
	if ( _put_data( serialport3 + 10, &asker ) != SUCCESS )
		return(FALSE);
#endif
	printf( "now close_battery_power_report\n" );
	return(TRUE);
}


int open_battery_power_report()
{
	_TData asker;

	memset( &asker, 0, sizeof(asker) );
	asker.address		= 0X01;
	asker.instruction	= 0x8C;
	asker.nbytes		= 0X03;
	asker.itemnum		= 0x01;
	asker.user_data[0]	= 0x01;

#if defined _2410
	if ( _put_data( serialport1, &asker ) != SUCCESS )
		return(FALSE);
#elif defined _2416
	if ( _put_data( serialport3 + 10, &asker ) != SUCCESS )
		return(FALSE);
#endif
	printf( "ccc\n" );
	return(TRUE);
}


//////////////////////////////
int update_mcu_start()
{
	int		uartport = serialport3 + 10;
	_TData		data;
	_TData		asker;
	struct timeval	oldtime, newtime;
	int		oldsec = 0, overtime = 5000;


	//send command start update
	memset( &data, 0, sizeof(data) );
	data.address		= 0X01;
	data.nbytes		= 0X01;
	data.instruction	= 0x3A;
	data.itemnum		= 0x01;
	data.user_data[0]	= 0x55;

	clear_port( uartport ); //clear com1 buffer
	if ( _put_data( uartport, &data ) != SUCCESS )
		return(FALSE);

	gettimeofday( &oldtime, NULL );
	oldsec = (oldtime.tv_sec % 10000) * 1000 + oldtime.tv_usec / 1000;
	while ( 1 )
	{
		gettimeofday( &newtime, NULL );
		if ( abs( ( (newtime.tv_sec % 10000) * 1000 + newtime.tv_usec / 1000) - oldsec ) > overtime )
		{
			printf( "update mcu start overtime\n" );
			return(FALSE);                                                          //there is no ask in 1000ms
		}
		usleep( 650 );
		memset( &asker, 0, sizeof(asker) );
		if ( _get_data( uartport, &asker ) == SUCCESS )                                 //read data in block
		{
			printf( "ask :%02X\n", asker.instruction );
			if ( asker.instruction == 0X00 && asker.user_data[0] == 0X00 )
			{
				overtime = 3000;
				gettimeofday( &oldtime, NULL );
			}else if ( asker.instruction == 0X3A && asker.user_data[0] == 0X06 )    //go to bootloader
			{
				return(TRUE);
			}
		}
	}
	return(FALSE);
}


int update_mcu_senddata( char *value, int size, int endflag )
{
	_TData	data;
	int	uartport = serialport3 + 10;

	//send command start update
	memset( &data, 0, sizeof(data) );
	data.address = 0X01;
	if ( endflag == 0 )
	{
		data.nbytes		= size;
		data.instruction	= 0x3A;
	}else  {
		data.nbytes		= 0X80;
		data.instruction	= 0x3B;
	}
	data.itemnum = 0x01;
	memcpy( data.user_data, value, size );

	if ( _put_data( uartport, &data ) != SUCCESS )
		return(FALSE);

	return(TRUE);
}


//download file to mcu
int update_mcu_download( char *value, int size, int endflag )
{
	int		retval		= 0;
	int		uartport	= serialport3 + 10;
	_TData		asker;
	struct timeval	oldtime, newtime;
	int		oldsec = 0, newsec = 0, exitsec = 0, againtime = 2000, exittime = 10000;
	//int i;

	retval = update_mcu_senddata( value, size, endflag ); //send data

	gettimeofday( &oldtime, NULL );
	oldsec	= (oldtime.tv_sec % 10000) * 1000 + oldtime.tv_usec / 1000;
	exitsec = oldsec;
	while ( 1 )
	{
		gettimeofday( &newtime, NULL );
		newsec = ( (newtime.tv_sec % 10000) * 1000 + newtime.tv_usec / 1000);
		if ( abs( newsec - oldsec ) > againtime )
		{
			oldsec	= newsec;
			retval	= update_mcu_senddata( value, size, endflag );          //send data
		}else if ( abs( newsec - exitsec ) > exittime )                         //10s exit
		{
			return(2);
		}
		memset( &asker, 0, sizeof(asker) );
		usleep( 650 );
		if ( _get_data( uartport, &asker ) == SUCCESS )                         //read data in block
		{
			if ( asker.instruction == 0X3A && asker.user_data[0] == 0X00 )
			{
				return(TRUE);
			}else  {
				oldsec	= newsec;
				retval	= update_mcu_senddata( value, size, endflag );  //send data
			}
		}
	}
	return(TRUE);
}


//


/*
 * update keyboad program
 * file : upload file
 * mode :0-auto update,1-Manual update
 */
int update_mcu( char *file, int mode )
{
	unsigned long	filesize = 0, sendsize = 0;
	int		retval = 0;
	unsigned int	CMAXLEN = 128, nbytes;
	char		buf[CMAXLEN];
	FILE		*ffp;
	struct stat	f_stat;


	if ( access( file, F_OK ) != 0 ) //if the directory is not exit
	{
		return(FALSE);
	}
	if ( stat( file, &f_stat ) < 0 )
	{
		return(FALSE);
	}

	filesize = (unsigned long) f_stat.st_size;
	if ( filesize == 0 )
		return(FALSE);

	if ( mode == active_update )
	{
		retval = update_mcu_start();
		if ( retval == FALSE )
			return(FALSE);
	}

	if ( (ffp = fopen( file, "rb" ) ) == NULL )
	{
		return(FALSE);
	}

	while ( !feof( ffp ) && !ferror( ffp ) )
	{
		memset( buf, 0, sizeof(buf) );
		nbytes		= fread( buf, sizeof(char), CMAXLEN, ffp );
		sendsize	+= nbytes;
		if ( (sendsize) >= filesize || nbytes < CMAXLEN )       //file end
			retval = update_mcu_download( buf, nbytes, 1 );
		else
			retval = update_mcu_download( buf, nbytes, 0 );


		if ( retval == 2 )                                      //overtime
		{
			fclose( ffp );
			return(FALSE);
		}
		if ( sendsize >= filesize )
			break;
	}

	fclose( ffp );
	return(TRUE);
}


//
int check_mcu_update( void )
{
	if ( com1value.instruction == UPDATEMCU &&
	     com1value.user_data[0] == 0X06 )
	{
		return(TRUE);
	}
	return(FALSE);
}



// AA 01 03 0F 01 FF F3 13 A5  通知MCU关机 2020/02/24
int device_power_off()
{
	_TData asker;

	memset( &asker, 0, sizeof(asker) );
	asker.address		= 0X01;
	asker.instruction	= 0x0F;
	asker.nbytes		= 0X01;
	asker.itemnum		= 0x01;
	asker.user_data[0]	= 0xFF;

#if defined _2410
	if ( _put_data( serialport1, &asker ) != SUCCESS )
		return(FALSE);
#elif defined _2416
	if ( _put_data( serialport3 + 10, &asker ) != SUCCESS )
		return(FALSE);
#endif
	printf( "[%s]\n", __FUNCTION__ );
	return(TRUE);
}


