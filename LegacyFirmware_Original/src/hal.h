/*=============================================================================
 * hal.h -- hardware abstraction layer for the heater controller board
 *
 * Everything hardware-specific lives behind these nine functions.
 * Implementations: hal_target.c (the real board) / hal_sim.c (PC build).
 *===========================================================================*/
#ifndef HAL_H
#define HAL_H

#include <stdint.h>

void   hal_init(void);
void   hal_wait_tick(void);           /* blocks until the next 1 kHz tick   */
double hal_read_temp(void);           /* process temperature [degC]         */
double hal_read_setpoint(void);       /* operator setpoint [degC]           */
int    hal_enable_switch(void);       /* 1 = heater enabled                 */
int    hal_reset_button(void);        /* 1 = fault reset pressed            */
void   hal_set_heater(double percent);            /* heater PWM 0..100 [%]  */
void   hal_set_status(uint32_t state, uint32_t alarms);   /* panel LEDs     */
void   hal_kick_watchdog(void);

#endif /* HAL_H */
