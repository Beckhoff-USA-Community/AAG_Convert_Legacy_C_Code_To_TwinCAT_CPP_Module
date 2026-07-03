/*=============================================================================
 * legacy_pid.c -- discrete PID controller implementation
 *
 * Derivative acts on the measurement (not the error) so setpoint steps do
 * not kick the output. Anti-windup by conditional integration: while the
 * output is saturated, the integrator only accepts updates that drive it
 * back out of saturation.
 *===========================================================================*/
#include <string.h>

#include "legacy_pid.h"

void pid_init(pid_ctx_t *ctx, double kp, double ki, double kd,
              double out_min, double out_max)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->kp      = kp;
    ctx->ki      = ki;
    ctx->kd      = kd;
    ctx->out_min = out_min;
    ctx->out_max = out_max;
}

void pid_reset(pid_ctx_t *ctx)
{
    ctx->integ     = 0.0;
    ctx->prev_meas = 0.0;
    ctx->primed    = 0;
}

double pid_step(pid_ctx_t *ctx, double setpoint, double meas, double dt)
{
    double err;
    double deriv;
    double integ_new;
    double out;

    if (dt <= 0.0)
    {
        return ctx->out_min;
    }

    err = setpoint - meas;

    /* derivative on measurement: no output kick on setpoint changes */
    if (ctx->primed)
    {
        deriv = -ctx->kd * (meas - ctx->prev_meas) / dt;
    }
    else
    {
        deriv = 0.0;
    }
    ctx->prev_meas = meas;
    ctx->primed    = 1;

    /* conditional integration anti-windup: accept the new integrator value
     * only while the output is unsaturated, or when it drives the output
     * back out of saturation */
    integ_new = ctx->integ + ctx->ki * err * dt;
    out = ctx->kp * err + integ_new + deriv;

    if (out > ctx->out_max)
    {
        if (integ_new < ctx->integ) { ctx->integ = integ_new; }
        out = ctx->out_max;
    }
    else if (out < ctx->out_min)
    {
        if (integ_new > ctx->integ) { ctx->integ = integ_new; }
        out = ctx->out_min;
    }
    else
    {
        ctx->integ = integ_new;
    }

    return out;
}
