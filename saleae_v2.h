#ifndef SALEAE_V2_H
#define SALEAE_V2_H

#include <stdint.h>

/* source: https://support.saleae.com/faq/technical-faq/binary-export-format-logic-2 */

typedef struct __attribute__((packed)) {
	uint8_t id[8];
	int32_t version; /* 0 */
	int32_t type; /* 0 for digital */
	uint32_t initial_state;
	double begin_time;
	double end_time;
	uint64_t num_transitions;
} saleae_v2_digital_t;

#endif
