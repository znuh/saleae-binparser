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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "mmap.h"

int map_file(mf_t * mf, const char *file, off_t ofs, size_t size) {
	struct stat attr;
	int fd;

	if ((fd = open(file, O_RDONLY)) < 0)
		return fd;

	if (fstat(fd, &attr)) {
		close(fd);
		return -1;
	}

	if (ofs < 1)
		ofs = 0;

	if ((size < 1) || (size > (attr.st_size - ofs)))
		size = attr.st_size - ofs;

	if ((mf->mem =
	     mmap(NULL, size, PROT_READ, MAP_SHARED, fd, ofs)) == MAP_FAILED) {
		close(fd);
		return -1;
	}
	//readahead(fd, ofs, size);

	close(fd);

	mf->len = attr.st_size;

	return 0;
}

void unmap_file(mf_t * mf) {
	munmap((void*)mf->mem, mf->len);
	mf->mem = NULL;
	mf->len = 0;
}
