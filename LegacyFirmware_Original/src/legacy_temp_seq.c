/*=============================================================================
 * legacy_temp_seq.c -- heater supervision state machine implementation
 *===========================================================================*/
#include <math.h>
#include <string.h>

#include "legacy_temp_seq.h"

static void tseq_enter(tseq_ctx_t *ctx, tseq_state_t s)
{
    ctx->state    = s;
    ctx->band_cnt = 0u;
}

void tseq_init(tseq_ctx_t *ctx, const tseq_cfg_t *cfg)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->cfg   = *cfg;
    ctx->state = TSEQ_IDLE;
}

tseq_state_t tseq_step(tseq_ctx_t *ctx, int enable, int fault_reset,
                       double setpoint, double actual)
{
    const tseq_cfg_t *c = &ctx->cfg;
    double err = fabs(setpoint - actual);

    /* live (non-latched) alarms */
    ctx->alarms &= ~TSEQ_ALM_NEAR_LIMIT;
    if (actual > c->high_limit - 5.0)
    {
        ctx->alarms |= TSEQ_ALM_NEAR_LIMIT;
    }

    /* overtemperature trip supervision runs in every state except FAULT */
    if (ctx->state != TSEQ_FAULT)
    {
        if (actual > c->high_limit)
        {
            if (++ctx->trip_cnt >= c->trip_ticks)
            {
                ctx->alarms |= TSEQ_ALM_OVERTEMP;
                tseq_enter(ctx, TSEQ_FAULT);
            }
        }
        else
        {
            ctx->trip_cnt = 0u;
        }
    }

    switch (ctx->state)
    {
    case TSEQ_IDLE:
        if (enable)
        {
            tseq_enter(ctx, TSEQ_RAMP);
        }
        break;

    case TSEQ_RAMP:
        if (!enable)
        {
            tseq_enter(ctx, TSEQ_IDLE);
            break;
        }
        if (err <= c->in_band)
        {
            if (++ctx->band_cnt >= c->in_band_ticks)
            {
                tseq_enter(ctx, TSEQ_TRACK);
            }
        }
        else
        {
            ctx->band_cnt = 0u;
        }
        break;

    case TSEQ_TRACK:
        if (!enable)
        {
            tseq_enter(ctx, TSEQ_IDLE);
            break;
        }
        if (err > c->out_band)
        {
            tseq_enter(ctx, TSEQ_RAMP);
        }
        break;

    case TSEQ_FAULT:
        /* latched: needs a RISING EDGE of the reset input (a held or
         * stuck-high reset must not defeat the latch) AND the temperature
         * well below the trip limit */
        if (fault_reset && !ctx->prev_reset &&
            (actual < c->high_limit - c->reset_margin))
        {
            ctx->alarms  &= ~TSEQ_ALM_OVERTEMP;
            ctx->trip_cnt = 0u;
            tseq_enter(ctx, TSEQ_IDLE);
        }
        break;

    default:
        tseq_enter(ctx, TSEQ_IDLE);
        break;
    }

    ctx->prev_reset = fault_reset;

    return ctx->state;
}

uint32_t tseq_alarms(const tseq_ctx_t *ctx)
{
    return ctx->alarms;
}

int tseq_heater_allowed(const tseq_ctx_t *ctx)
{
    return (ctx->state == TSEQ_RAMP) || (ctx->state == TSEQ_TRACK);
}
