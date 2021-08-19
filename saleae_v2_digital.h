#ifndef SALEAE_V2_DIGITAL_H
#define SALEAE_V2_DIGITAL_H

#include "saleae_v2.h"
#include "mmap.h"

#define MAX_CHANNELS    16

struct transition_s {
	struct transition_s *next;
	double timestamp;
	uint32_t toggle;
	int channels[MAX_CHANNELS];
	int n_channels;
};

typedef struct {
	mf_t mf[MAX_CHANNELS];

	int n_channels, channel_list[MAX_CHANNELS];

	const double *last_ts[MAX_CHANNELS];
	const double *current_ts[MAX_CHANNELS];

	uint32_t state;

	struct transition_s *transitions;
	struct transition_s trans_pool[MAX_CHANNELS+2];
	struct transition_s *trans_stack[MAX_CHANNELS+1]; /* need one additional buffer */
	int trans_stack_idx;
} logic_t;

void logic_cleanup(logic_t *logic);
int logic_init(logic_t *logic, const char *file_prefix);

/* will return when trigger event occurs
 * trig_falling: bitmask for falling edge triggers
 * trig_rising : bitmask for rising  edge triggers
 * use 0xffff, 0xffff for single-stepping every transition 
 */
int logic_replay(logic_t *l, uint32_t *state, double *ts, uint32_t trig_falling, uint32_t trig_rising);

#endif
