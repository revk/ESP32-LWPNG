

test:	test.c lwpng.c
	gcc -DCONFIG_LWPNG_DEBUG -DCONFIG_LWPNG_CHECKS -O -o $@ $< lwpng.c -g -Wall --std=gnu99 -lz -Iinclude
