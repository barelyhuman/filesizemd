#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/deflate.h"

void getFilename(char **filePtr, char *ogPathPtr)
{
    char *slash = ogPathPtr, *next;
    while ((next = strpbrk(slash + 1, "\\/")))
        slash = next;
    if (ogPathPtr != slash)
        slash++;
    *filePtr = strdup(slash);
}

char *prettyBytes(int size)
{
    int kb = 1000;
    int mb = kb * 1000;
    int gb = mb * 1000;
    char *str = malloc(size + sizeof("GB"));

    if (str == NULL)
    {
        perror("Unable to get memory");
        exit(EXIT_FAILURE);
    }

    if (size >= gb)
    {
        sprintf(str, "%.2fGB", (float)size / mb);
    }
    else if (size >= mb)
    {
        sprintf(str, "%.2fMB", (float)size / mb);
    }
    else if (size >= kb)
    {
        sprintf(str, "%.2fKB", (float)size / kb);
    }
    else
    {
        sprintf(str, "%dB", size);
    }
    return str;
}

int file_size(FILE *source)
{
    char ch;
    int size = 0;

    if (NULL == source)
    {
        perror("Need a file to size...");
        exit(EXIT_FAILURE);
    }

    fseek(source, 0, SEEK_SET);

    do
    {
        ch = fgetc(source);
        size += sizeof(ch);
    } while (ch != EOF);
    return size;
}

int main(int argc, char **argv)
{

    // pointers to each file that'll be created
    FILE *source, *gzip_file;
    char *filename;

    // if no args are provided, do nothing
    if (argc < 2)
    {
        perror("filesize needs to be provided with the file/pattern as an argument");
        exit(EXIT_FAILURE);
    }

    // allocated memory for the name that'll be a combination of the file name
    // and the added extensions
    getFilename(&filename, argv[1]);
    char *gzip_file_name = malloc(strlen("compressed") + strlen(filename) + sizeof("_.gz"));

    source = fopen(argv[1], "r");

    if (NULL == source)
    {
        perror("failed to read source file...");
        exit(EXIT_FAILURE);
    }

    sprintf(gzip_file_name, "compressed_%s.gz", filename);
    gzip_file = fopen(gzip_file_name, "w+b");

    if (NULL == gzip_file)
    {
        perror("failed to create temporary compressed file...");
        exit(EXIT_FAILURE);
    }

    deflateFile(source, gzip_file, 6);

    char *og_size = prettyBytes(file_size(source));
    char *gz_size = prettyBytes(file_size(gzip_file));

    printf("|file|size|gzip|\n");
    printf("|---|---|---|\n");
    printf("|%s|%s|%s|\n", argv[1], og_size, gz_size);

    if (remove(gzip_file_name) != 0)
    {
        perror("Failed to remove temporary compressed file. Please do so manually");
        exit(EXIT_FAILURE);
    };

    // cleanup
    fclose(source);
    fclose(gzip_file);

    free(og_size);
    free(gz_size);
    free(gzip_file_name);
    return 0;
}
