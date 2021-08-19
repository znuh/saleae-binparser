#ifndef MMAP_H
#define MMAP_H

#include <stdint.h>
#include <sys/types.h>

typedef struct mf_s {
	const void *mem;
	size_t len;
} mf_t;

int map_file(mf_t * mf, const char *file, off_t ofs, size_t size);
void unmap_file(mf_t * mf);

#endif
