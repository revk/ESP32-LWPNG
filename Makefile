

test:	test.c lwpng.c Makefile
	gcc -DDEBUG -DCONFIG_LWPNG_DECODE -DCONFIG_LWPNG_ENCODE -DCONFIG_LWPNG_CHECKS -O -o $@ $< lwpng.c -g -Wall --std=gnu99 -lz -Iinclude -I.
