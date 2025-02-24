

test:	test.c lwpng.c
	gcc -DTEST -DCONFIG_LWPNG_CHECKS -O -o $@ $< lwpng.o -g -Wall --std=gnu99 -lpopt -lz -Iinclude lwpng.o
