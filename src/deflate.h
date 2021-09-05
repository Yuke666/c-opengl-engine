#ifndef DEFLATE_DEF
#define DEFLATE_DEF

#include <stdio.h>

int Deflate_Read(FILE *fp, void *buffer, int len);
int Deflate_FromMemory(void *memory, void *buffer, int len);

#endif