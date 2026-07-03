/*=============================================================================
 * main.c -- heater controller firmware, main loop
 *
 * Classic bare-metal structure: initialize everything, then run the control
 * law at a fixed 1 kHz tick forever. The HAL (hal.h) hides the hardware --
 * link hal_target.c on the real board, or sim/hal_sim.c for the PC build.
 *===========================================================================*/
#include "hal.h"
#include "legacy_pid.h"
#include "legacy_temp_seq.h"

#define TICK_HZ  1000u
#define TICK_DT  (1.0 / (double)TICK_HZ)

/* controller tuning (on the real board these came from EEPROM) */
#define CFG_KP          8.0
#define CFG_KI          0.5
#define CFG_KD          0.0
#define CFG_HIGH_LIMIT  110.0   /* overtemperature trip [degC] */

int main(void)
{
    pid_ctx_t  pid;
    tseq_ctx_t seq;
    tseq_cfg_t cfg;

    cfg.in_band       = 1.0;    /* "at temperature" within +/-1 degC      */
    cfg.out_band      = 3.0;    /* fall back to RAMP beyond +/-3 degC     */
    cfg.in_band_ticks = 500u;   /* in band for 500 ticks before TRACK     */
    cfg.high_limit    = CFG_HIGH_LIMIT;
    cfg.trip_ticks    = 100u;   /* 100 ticks above limit trips FAULT      */
    cfg.reset_margin  = 10.0;   /* must cool 10 degC below limit to reset */

    hal_init();
    pid_init(&pid, CFG_KP, CFG_KI, CFG_KD, 0.0, 100.0);
    tseq_init(&seq, &cfg);

    for (;;)
    {
        double       temp;
        double       sp;
        double       power;
        tseq_state_t state;

        hal_wait_tick();

        temp = hal_read_temp();
        sp   = hal_read_setpoint();

        state = tseq_step(&seq, hal_enable_switch(), hal_reset_button(),
                          sp, temp);

        power = 0.0;
        if (tseq_heater_allowed(&seq))
        {
            power = pid_step(&pid, sp, temp, TICK_DT);
        }
        else
        {
            pid_reset(&pid);
        }

        hal_set_heater(power);
        hal_set_status((uint32_t)state, tseq_alarms(&seq));
        hal_kick_watchdog();
    }
}
