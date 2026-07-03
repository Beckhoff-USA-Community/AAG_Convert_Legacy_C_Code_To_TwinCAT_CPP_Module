/*=============================================================================
 * legacy_pid.h -- discrete PID controller with clamping anti-windup
 *
 * Portable ANSI C99. No OS calls, no HAL, no dynamic memory.
 * Written for a bare-metal temperature controller: pid_step() is intended
 * to be called at a fixed rate from a timer ISR or the main loop.
 *===========================================================================*/
#ifndef LEGACY_PID_H
#define LEGACY_PID_H

typedef struct pid_ctx
{
    /* tuning */
    double kp;
    double ki;
    double kd;
    double out_min;
    double out_max;

    /* state */
    double integ;      /* integrator accumulator (in output units)      */
    double prev_meas;  /* last measurement, for derivative-on-PV        */
    int    primed;     /* 0 until the first sample has been processed   */
} pid_ctx_t;

void   pid_init(pid_ctx_t *ctx, double kp, double ki, double kd,
                double out_min, double out_max);
void   pid_reset(pid_ctx_t *ctx);
double pid_step(pid_ctx_t *ctx, double setpoint, double meas, double dt);

#endif /* LEGACY_PID_H */
