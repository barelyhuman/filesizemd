#include "../lib/deflate.h"
#include <brotli/decode.h>
#include <brotli/encode.h>
#include <brotli/types.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 1024

// function defs
void brotli_compress(FILE *infile, FILE *outfile);

void get_filename(char **filePtr, char *ogPathPtr) {
  char *slash = ogPathPtr, *next;
  while ((next = strpbrk(slash + 1, "\\/")))
    slash = next;
  if (ogPathPtr != slash)
    slash++;
  *filePtr = strdup(slash);
}

char *pretty_bytes(long int size) {
  int kb = 1000;
  int mb = kb * 1000;
  int gb = mb * 1000;
  char *str = malloc(size + sizeof("GB"));

  if (str == NULL) {
    perror("Unable to get memory");
    exit(EXIT_FAILURE);
  }

  if (size >= gb) {
    sprintf(str, "%.2fGB", (float)size / mb);
  } else if (size >= mb) {
    sprintf(str, "%.2fMB", (float)size / mb);
  } else if (size >= kb) {
    sprintf(str, "%.2fKB", (float)size / kb);
  } else {
    sprintf(str, "%ldB", size);
  }
  return str;
}

long int size_file(FILE *source) {

  if (source == NULL) {
    perror("Need a file to size...");
    exit(EXIT_FAILURE);
  }

  fseek(source, 0L, SEEK_END);

  // calculating the size of the file
  long int size = ftell(source);

  return size;
}

void table_from_glob(char *globPattern, char **tableBuf, int *tableBuffLen) {
  glob_t globbuf;

  // handle only the immediate next glob/pattern of args
  globbuf.gl_offs = 0;
  glob(globPattern, GLOB_DOOFFS, NULL, &globbuf);

  // if nothing matches then well, show a error regarding the same
  if (globbuf.gl_pathc == 0) {
    perror("No files matching the given pattern");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < globbuf.gl_pathc; i++) {
    // temporary pointers to each file that'll be created
    FILE *source, *gzip_file, *brotli_file;
    char *filename;

    // get the filename
    get_filename(&filename, globbuf.gl_pathv[i]);

    char *gzip_file_name =
        malloc(strlen("compressed") + strlen(filename) + sizeof("_.gz"));
    char *brotli_file_name =
        malloc(strlen("compressed") + strlen(filename) + sizeof("_.br"));

    source = fopen(globbuf.gl_pathv[i], "r");

    if (NULL == source) {
      perror("failed to read source file...");
      exit(EXIT_FAILURE);
    }

    sprintf(gzip_file_name, "compressed_%s.gz", filename);
    gzip_file = fopen(gzip_file_name, "w+b");
    if (NULL == gzip_file) {
      perror("failed to create temporary compressed file...");
      exit(EXIT_FAILURE);
    }

    // compress the file to a gzip archive using level 6, can try with level 7
    deflateFile(source, gzip_file, 6);

    sprintf(brotli_file_name, "compressed_%s.br", filename);
    brotli_file = fopen(brotli_file_name, "w+b");
    if (NULL == brotli_file) {
      perror("failed to create temporary compressed file...");
      exit(EXIT_FAILURE);
    }

    // seek back to the start of the file before compressing using brotli
    fseek(source, 0L, 0);
    brotli_compress(source, brotli_file);

    char *og_size = pretty_bytes(size_file(source));
    char *gz_size = pretty_bytes(size_file(gzip_file));
    char *br_size = pretty_bytes(size_file(brotli_file));

    *tableBuffLen += sprintf(*tableBuf + *tableBuffLen, "|%s|%s|%s|%s|\n",
                             filename, og_size, gz_size, br_size);

    *tableBuf = (char *)realloc(*tableBuf, *tableBuffLen * sizeof("a"));

    // get rid of the temporary archives that were created
    // to calculate the archive size
    if (remove(gzip_file_name) != 0) {
      perror(
          "Failed to remove temporary compressed file. Please do so manually");
      exit(EXIT_FAILURE);
    };

    if (remove(brotli_file_name) != 0) {
      perror(
          "Failed to remove temporary compressed file. Please do so manually");
      exit(EXIT_FAILURE);
    };

    // cleanup
    fclose(source);
    fclose(gzip_file);
    fclose(brotli_file);

    free(og_size);
    free(gz_size);
    free(br_size);
    free(brotli_file_name);
  }
  // cleanup the current glob
  globfree(&globbuf);
}

int main(int argc, char **argv) {

  // if no args are provided, do nothing
  if (argc < 2) {
    perror(
        "filesize needs to be provided with the file/pattern as an argument");
    exit(EXIT_FAILURE);
  }

  // allocate enough memory for the first 2 rows
  // of the table
  char *tableBuf = (char *)malloc(100);
  int tableBuffLen = 0;

  tableBuffLen += sprintf(tableBuf + tableBuffLen, "|file|size|gzip|brotli|\n");
  tableBuffLen += sprintf(tableBuf + tableBuffLen, "|---|---|---|---|\n");

  for (int i = 1; i < argc; i++) {
    table_from_glob(argv[i], &tableBuf, &tableBuffLen);
  }

  fprintf(stdout, "%s", tableBuf);

  free(tableBuf);

  return 0;
}

void brotli_compress(FILE *infile, FILE *outfile) {
  BrotliEncoderState *encoder_state =
      BrotliEncoderCreateInstance(NULL, NULL, NULL);
  if (!encoder_state) {
    return;
  }

  unsigned char input_buffer[CHUNK_SIZE];
  unsigned char output_buffer[CHUNK_SIZE * 2];

  size_t input_size, output_size;

  while ((input_size = fread(input_buffer, 1, CHUNK_SIZE, infile)) > 0) {
    output_size = CHUNK_SIZE * 2;

    if (!BrotliEncoderCompress(6, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_GENERIC,
                               input_size, input_buffer, &output_size,
                               output_buffer)) {
      fprintf(stderr, "Failed to compress data\n");
      BrotliEncoderDestroyInstance(encoder_state);
      break;
    }

    if (fwrite(output_buffer, 1, output_size, outfile) != output_size) {
      perror("Failed to write compressed data to output file");
      BrotliEncoderDestroyInstance(encoder_state);
    }
  }
}