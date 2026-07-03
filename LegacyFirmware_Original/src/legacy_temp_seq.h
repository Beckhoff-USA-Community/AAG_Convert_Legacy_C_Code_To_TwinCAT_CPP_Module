/*=============================================================================
 * legacy_temp_seq.h -- heater supervision state machine
 *
 * IDLE -> RAMP -> TRACK sequencing with a latched FAULT state on sustained
 * overtemperature. Portable ANSI C99, no OS calls, no dynamic memory.
 * tseq_step() is intended to be called at a fixed rate ("tick").
 *===========================================================================*/
#ifndef LEGACY_TEMP_SEQ_H
#define LEGACY_TEMP_SEQ_H

#include <stdint.h>

typedef enum tseq_state
{
    TSEQ_IDLE  = 0,   /* heater off, waiting for enable            */
    TSEQ_RAMP  = 1,   /* controller active, approaching setpoint   */
    TSEQ_TRACK = 2,   /* controller active, inside tolerance band  */
    TSEQ_FAULT = 3    /* latched overtemperature trip, heater off  */
} tseq_state_t;

/* alarm word bits */
#define TSEQ_ALM_OVERTEMP   (1u << 0)  /* latched overtemp trip              */
#define TSEQ_ALM_NEAR_LIMIT (1u << 1)  /* live: within 5 degC of trip limit  */

typedef struct tseq_cfg
{
    double   in_band;        /* |err| to consider "at temperature"       */
    double   out_band;       /* |err| to fall back from TRACK to RAMP    */
    uint32_t in_band_ticks;  /* consecutive in-band ticks before TRACK   */
    double   high_limit;     /* overtemperature trip threshold           */
    uint32_t trip_ticks;     /* consecutive ticks above limit to trip    */
    double   reset_margin;   /* temp must be this far below the limit
                                before a fault reset is accepted         */
} tseq_cfg_t;

typedef struct tseq_ctx
{
    tseq_cfg_t   cfg;
    tseq_state_t state;
    uint32_t     band_cnt;
    uint32_t     trip_cnt;
    uint32_t     alarms;
    int          prev_reset;  /* last reset input, for edge detection */
} tseq_ctx_t;

void         tseq_init(tseq_ctx_t *ctx, const tseq_cfg_t *cfg);
tseq_state_t tseq_step(tseq_ctx_t *ctx, int enable, int fault_reset,
                       double setpoint, double actual);
uint32_t     tseq_alarms(const tseq_ctx_t *ctx);
int          tseq_heater_allowed(const tseq_ctx_t *ctx);

#endif /* LEGACY_TEMP_SEQ_H */
