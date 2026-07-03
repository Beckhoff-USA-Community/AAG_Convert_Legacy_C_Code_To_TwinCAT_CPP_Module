/*=============================================================================
 * hal_sim.c -- PC simulation build of the HAL
 *
 * Lets the firmware run on a desktop, unmodified: the "plant" is a
 * first-order thermal model, the 1 kHz tick is free-running (no real-time
 * wait), and the operator inputs follow a scripted scenario. Prints a CSV
 * trace to stdout plus a comment line on every state change.
 *
 * Scenario: enable at t=2 s, setpoint 80 degC; at t=60 s setpoint 120 degC
 * (drives the 110 degC overtemp trip); at t=90 s back to 80; operator
 * presses fault reset at t=110 s.
 *===========================================================================*/
#include <stdio.h>
#include <stdlib.h>

#include "hal.h"

#define SIM_SECONDS   180u
#define TICK_HZ       1000u

/* plant: dT/dt = ((ambient + gain * power) - T) / tau
 * (the TwinCAT demo module simulates the identical plant) */
#define PLANT_AMBIENT 25.0
#define PLANT_GAIN    1.0     /* degC per % heater power */
#define PLANT_TAU     8.0     /* s */

static uint32_t tick;
static double   temp = PLANT_AMBIENT;
static double   power;
static uint32_t cur_state;
static uint32_t cur_alarms;
static uint32_t last_state = 0xFFFFFFFFu;

static double now(void)
{
    return (double)tick / (double)TICK_HZ;
}

void hal_init(void)
{
    printf("time_s,setpoint,temp,power,state,alarms\n");
}

void hal_wait_tick(void)
{
    /* advance the simulated world by one tick instead of waiting */
    double dt = 1.0 / (double)TICK_HZ;
    temp += dt / PLANT_TAU * (PLANT_AMBIENT + PLANT_GAIN * power - temp);

    if (++tick > SIM_SECONDS * TICK_HZ)
    {
        printf("# simulation complete after %u s, final state=%u alarms=0x%x\n",
               SIM_SECONDS, cur_state, cur_alarms);
        exit(0);
    }
}

double hal_read_temp(void)
{
    return temp;
}

/* --- scripted operator --------------------------------------------------*/

double hal_read_setpoint(void)
{
    double t = now();
    if (t < 60.0) { return 80.0; }
    if (t < 90.0) { return 120.0; }   /* forces the overtemp trip */
    return 80.0;
}

int hal_enable_switch(void)
{
    return now() >= 2.0;
}

int hal_reset_button(void)
{
    double t = now();
    return (t >= 110.0 && t < 110.5);   /* operator presses reset */
}

/* --- outputs ------------------------------------------------------------*/

void hal_set_heater(double percent)
{
    power = percent;
}

void hal_set_status(uint32_t state, uint32_t alarms)
{
    static const char *names[] = { "IDLE", "RAMP", "TRACK", "FAULT" };

    cur_state  = state;
    cur_alarms = alarms;

    if (state != last_state)
    {
        printf("# t=%7.3f s: state -> %s (alarms=0x%x)\n",
               now(), (state < 4u) ? names[state] : "?", alarms);
        last_state = state;
    }
}

void hal_kick_watchdog(void)
{
    /* CSV sample every 500 ms */
    if ((tick % 500u) == 0u)
    {
        printf("%.1f,%.1f,%.2f,%.1f,%u,0x%x\n",
               now(), hal_read_setpoint(), temp, power, cur_state, cur_alarms);
    }
}
