/*=============================================================================
 * hal_target.c -- HAL implementation for the real controller board
 *
 * This is the hardware-specific part of the firmware: memory-mapped
 * ADC/PWM/GPIO registers and the hardware watchdog. In the TwinCAT retrofit
 * this file is exactly the part that gets REPLACED -- the TcCOM module's
 * mapped data areas take over the role of these registers.
 *
 * Kept for reference; compile with -DTARGET_BUILD on the board toolchain.
 *===========================================================================*/
#ifdef TARGET_BUILD

#include "hal.h"

#define REG32(a)        (*(volatile uint32_t *)(a))

#define ADC_TEMP_RAW    REG32(0x40012440u)  /* 12 bit, 0.05 degC / LSB     */
#define ADC_SETP_RAW    REG32(0x40012444u)  /* setpoint potentiometer      */
#define GPIO_IN         REG32(0x40010800u)  /* bit0 enable, bit1 reset     */
#define PWM_HEATER_DUTY REG32(0x40010C34u)  /* 0..10000 = 0..100.00 %      */
#define GPIO_LED_OUT    REG32(0x40010804u)
#define WDG_KICK        REG32(0x40003000u)
#define SYSTICK_FLAG    REG32(0xE000E010u)

void hal_init(void)
{
    /* clock tree, ADC calibration, PWM timer setup ... */
}

void hal_wait_tick(void)
{
    while ((SYSTICK_FLAG & 0x10000u) == 0u)
    {
        /* spin until the 1 kHz systick fires */
    }
}

double hal_read_temp(void)     { return (double)ADC_TEMP_RAW * 0.05; }
double hal_read_setpoint(void) { return (double)ADC_SETP_RAW * 0.05; }
int    hal_enable_switch(void) { return (int)(GPIO_IN & 1u); }
int    hal_reset_button(void)  { return (int)((GPIO_IN >> 1) & 1u); }

void hal_set_heater(double percent)
{
    PWM_HEATER_DUTY = (uint32_t)(percent * 100.0);
}

void hal_set_status(uint32_t state, uint32_t alarms)
{
    GPIO_LED_OUT = (state & 0x3u) | ((alarms & 0x3u) << 2);
}

void hal_kick_watchdog(void)
{
    WDG_KICK = 0xA5A5A5A5u;
}

#endif /* TARGET_BUILD */
