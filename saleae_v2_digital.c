/*
 * Copyright (C) 2021 Benedikt Heinz <Zn000h AT gmail.com>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this code.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <inttypes.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <alloca.h>
#include <assert.h>

#include "saleae_v2_digital.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

static void enqueue_transition(logic_t *logic, int ch, double ts) {
	struct transition_s *prev=NULL, *curr, *tmp;

	/* find insertion position - O(n) */
	for(curr=logic->transitions;ts > curr->timestamp;prev=curr,curr=curr->next) {}

	/* add a new transition */
	if(ts != curr->timestamp) {
		tmp = logic->trans_stack[--logic->trans_stack_idx];
		tmp->next = curr;
		curr=tmp;
		curr->timestamp = ts;
		curr->n_channels = 0;
		curr->toggle = 0;
		/* new 1st element in queue */
		if(!prev)
			logic->transitions = curr;
		else
			prev->next = curr;
	}
	/* add channel to transition */
	curr->toggle|=(1<<ch);
	curr->channels[curr->n_channels++]=ch;
}

static void refill_queue(logic_t *logic, const int *channels, const int n_channels) {
	int i, ch;
	
	for(i=0;i<n_channels;i++) {
		ch=channels[i];

		/* end of file? */
		if(logic->current_ts[ch] > logic->last_ts[ch])
			continue;

		enqueue_transition(logic, ch, *logic->current_ts[ch]);
		/* advance channel */
		logic->current_ts[ch]++;
	}
}

void logic_cleanup(logic_t *logic) {
	int i;
	for(i=0;i<logic->n_channels;i++) {
		int ch = logic->channel_list[i];
		unmap_file(logic->mf+ch);
	}
	memset(logic, 0, sizeof(logic_t));
}

static const char ref_id[]="<SALEAE>";

static int channel_init(logic_t *logic, int ch) {
	mf_t *mf=logic->mf+ch;
	const saleae_v2_digital_t *hdr = mf->mem;
	const uint8_t *ptr=mf->mem+sizeof(saleae_v2_digital_t);
	int res = 0;
	
	res=memcmp(hdr->id,ref_id,8);
	if(res) {
		fprintf(stderr,"invalid ID for channel %d\n",ch);
		return -1;
	}
	
	if(hdr->version != 0) {
		fprintf(stderr,"invalid version for channel %d\n",ch);
		return -1;
	}
	
	if(hdr->type != 0) {
		fprintf(stderr,"invalid type (non-digital?) for channel %d\n",ch);
		return -1;
	}

	/* sanity check that file is complete */
	if(((mf->len - sizeof(saleae_v2_digital_t)) / sizeof(double)) != hdr->num_transitions) {
		fprintf(stderr,"file size mismatch (vs. num_transitions) for channel %d\n",ch);
		return -1;
	}

	logic->current_ts[ch] = (const double*)ptr;
	logic->last_ts[ch] = logic->current_ts[ch] + hdr->num_transitions - 1;

	/* apply initial state from channel */
	logic->state|=hdr->initial_state<<ch;

	fprintf(stderr,"found channel %2d - initial state: %x, %12"PRIu64" transitions\n",
		ch,hdr->initial_state,hdr->num_transitions);

	return res;
}

int logic_init(logic_t *logic, const char *file_prefix) {
	int fn_len = strlen(file_prefix);
	char *fn_buf=alloca(fn_len+8);
	int i, res;
	
	memset(logic,0,sizeof(logic_t));
	
	for(i=0;i<MAX_CHANNELS;i++) {

		/* add one transition to pool of free transition buffers */
		logic->trans_stack[logic->trans_stack_idx++] = logic->trans_pool+i;

		/* mmap channel file */
		sprintf(fn_buf,"%s_%d.bin",file_prefix,i);
		res = map_file(logic->mf+i,fn_buf,0,0);
		if(res)
			continue;
		
		/* init channel */
		res = channel_init(logic, i);
		if(res) {
			unmap_file(logic->mf+i);
			continue;
		}
		
		/* channel good */
		logic->channel_list[logic->n_channels++] = i;
	}

	/* add extra transition buffer to pool - needed for pivoting during remove-first */
	logic->trans_stack[logic->trans_stack_idx++] = logic->trans_pool+i;
	
	/* end-of-queue marker */
	logic->transitions = logic->trans_pool+MAX_CHANNELS+1;
	logic->transitions->timestamp = DBL_MAX;
	
	/* initial fill for the queue - fill queue from all channels */
	refill_queue(logic, logic->channel_list, logic->n_channels);
	
	return logic->n_channels;
}

static inline int logic_step(logic_t *l, uint32_t *state, double *ts) {
	struct transition_s *trans = l->transitions;
	
	/* queue empty */
	if(unlikely(!trans->toggle))
		return 0;

	l->state^=trans->toggle;

	/* remove from transition queue & refill queue */
	l->transitions = trans->next;
	refill_queue(l, trans->channels, trans->n_channels);

	/* put transition buffer back into pool */
	l->trans_stack[l->trans_stack_idx++]=trans;

	if(state)
		*state = l->state;

	if(ts)
		*ts = trans->timestamp;

	return trans->n_channels;
}

/* will return when trigger event occurs
 * trig_falling: bitmask for falling edge triggers
 * trig_rising : bitmask for rising  edge triggers
 * use 0xffff, 0xffff for single-stepping every transition 
 */
int logic_replay(logic_t *l, uint32_t *state, double *ts, uint32_t trig_falling, uint32_t trig_rising) {
	uint32_t last_state, new_state;
	int res=0;
	
	for(last_state=l->state;(res=logic_step(l, &new_state, ts));last_state=new_state) {
		uint32_t changed = last_state ^ new_state;
		if((changed & new_state & trig_rising) || (changed & last_state & trig_falling))
			break;
	}
	
	if(state)
		*state = l->state;
	
	return res;
}
