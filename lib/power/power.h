/*
 * power.h
 *
 *  Created on: 2013-7-11
 *      Author: aduo
 */

#ifndef POWER_H_
#define POWER_H_

#include "../version.h"

typedef enum _battery_status{
	bs_none,
	bs_charging,
	bs_charged,
	bs_discharging,
	bs_low_voltage,
	bs_high_voltage,
	bs_unknown
} battery_status;

struct _battery{
	battery_status status;
	int energy;
};

extern struct _battery battery;

int get_battery_power_status();
int get_battery_power_energy();
int open_battery_power_report();
int close_battery_power_report();
//upload keyboard program
typedef enum __UPDATE_MCU_MODE {
    auto_update=0,
    active_update
}UPDATE_MCU_MODE;

int update_mcu_start();
int update_mcu_download(char *value,int size,int endflag);
int update_mcu(char *file,int mode);
int check_mcu_update(void) ;

#endif /* POWER_H_ */
