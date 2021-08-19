
/* LICENSE: this file is public domain */

#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include "saleae_v2_digital.h"

/* example: NAND with async parallel 8-Bit bus
 * channels 0..7: Data[0..7]
 *
 * NOTE: this is just an example to demonstrate use of logic_init/_replay/_cleanup
 * doing something useful is left as an exercise for the reader ;)
 */
#define nCE_CH    9
#define nWE_CH    10
#define nRE_CH    11
#define CLE_CH    13
#define ALE_CH    14

static void nand_cmdbyte(uint8_t cmd) {
	printf("NAND command: %02x\n",cmd);
}

static void nand_addrbyte(uint8_t addr) {
	printf("NAND address byte: %02x\n",addr);
}

static void nand_databyte(uint8_t data,int nWE, int nRE) {
	printf("NAND data byte: %02x, nWE: %d, nRE: %d\n",data,nWE,nRE);
}

int main(int argc, char **argv) {
	logic_t logic;
	uint32_t state;
	uint64_t ts;
	double tsd;
	int res;
	
	assert(argc>1);
	res = logic_init(&logic, argv[1]);
	if(res>=0) {
		fprintf(stderr,"%d channels found\n",res);
		fprintf(stderr,"initial state: %04x\n",logic.state);
	}
	
	/* trigger on nRE/nWE rising edges */
	while(logic_replay(&logic, &state, &tsd, 0, (1<<nRE_CH)|(1<<nWE_CH))) {
		int nCE = (state>>nCE_CH)&1;
		int nWE = (state>>nWE_CH)&1;
		int nRE = (state>>nRE_CH)&1;
		int CLE = (state>>CLE_CH)&1;
		int ALE = (state>>CLE_CH)&1;
		
		/* chip selected? */
		if(nCE)
			continue;
		
		/* convert timestamp to integer, 1ns units */
		tsd*=1000000000.0;
		ts=tsd;
		
		printf("%"PRIu64": ",ts);
		
		if(CLE)
			nand_cmdbyte(state&0xff);
		else if(ALE)
			nand_addrbyte(state&0xff);
		else
			nand_databyte(state&0xff,nWE,nRE);
	}
	
	logic_cleanup(&logic);
	
	return 0;
}
