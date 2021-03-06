#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "../lib/deflate.h"

void getfilename(char **filePtr, char *ogPathPtr)
{
    char *slash = ogPathPtr, *next;
    while ((next = strpbrk(slash + 1, "\\/")))
        slash = next;
    if (ogPathPtr != slash)
        slash++;
    *filePtr = strdup(slash);
}

char *prettybytes(long int size)
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
        sprintf(str, "%ldB", size);
    }
    return str;
}

long int sizefile(FILE *source)
{

    if (source == NULL)
    {
        perror("Need a file to size...");
        exit(EXIT_FAILURE);
    }

    fseek(source, 0L, SEEK_END);

    // calculating the size of the file
    long int size = ftell(source);

    return size;
}

void tablefromglob(char *globPattern, char **tableBuf, int *tableBuffLen)
{
    glob_t globbuf;

    // handle only the immediate next glob/pattern of args
    globbuf.gl_offs = 0;
    glob(globPattern, GLOB_DOOFFS, NULL, &globbuf);

    // if nothing matches then well, show a error regarding the same
    if (globbuf.gl_pathc == 0)
    {
        perror("No files matching the given pattern");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < globbuf.gl_pathc; i++)
    {
        // temporary pointers to each file that'll be created
        FILE *source, *gzip_file;
        char *filename;

        // get the filen
        getfilename(&filename, globbuf.gl_pathv[i]);

        char *gzip_file_name = malloc(strlen("compressed") + strlen(filename) + sizeof("_.gz"));

        source = fopen(globbuf.gl_pathv[i], "r");

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

        // compress the file to a gzip archive using level 6, can try with level 7
        deflateFile(source, gzip_file, 6);

        char *og_size = prettybytes(sizefile(source));
        char *gz_size = prettybytes(sizefile(gzip_file));

        *tableBuffLen += sprintf(*tableBuf + *tableBuffLen, "|%s|%s|%s|\n", filename, og_size, gz_size);

        *tableBuf = (char *)realloc(*tableBuf, *tableBuffLen * sizeof("a"));

        // get rid of the temporary archive that was created
        // to calculate the archive size
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
    }
    // cleanup the current glob
    globfree(&globbuf);
}

int main(int argc, char **argv)
{

    // if no args are provided, do nothing
    if (argc < 2)
    {
        perror("filesize needs to be provided with the file/pattern as an argument");
        exit(EXIT_FAILURE);
    }

    // allocate enough memory for the first 2 rows
    // of the table
    char *tableBuf = (char *)malloc(100);
    int tableBuffLen = 0;

    tableBuffLen += sprintf(tableBuf + tableBuffLen, "|file|size|gzip|\n");
    tableBuffLen += sprintf(tableBuf + tableBuffLen, "|---|---|---|\n");

    for (int i = 1; i < argc; i++)
    {
        tablefromglob(argv[i], &tableBuf, &tableBuffLen);
    }

    fprintf(stdout, "%s", tableBuf);

    free(tableBuf);

    return 0;
}
