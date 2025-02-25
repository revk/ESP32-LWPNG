

test:	test.c lwpng.c Makefile
	gcc -DCONFIG_LWPNG_CHECKS -O -o $@ $< lwpng.c -g -Wall --std=gnu99 -lz -Iinclude -I. -DDEBUG
