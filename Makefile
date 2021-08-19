all:
	gcc -Wall -O3 -o example example_main.c saleae_v2_digital.c mmap.c
	
