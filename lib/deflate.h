#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

int deflateFile(FILE *source, FILE *dest, int level);