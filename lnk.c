/*
 * Simple program to get the location a LNK (Windows Shortcut File) points to.
 * It returns the ANSI string as-is; you can pipe the output to iconv with the appropiate codepage.
 *
 * By z411 <z411@omaera.org>
 * Code released into the public domain.
 */

#include <stdio.h>

void
usage()
{
  printf("Usage: lnk <filename>\n");
}

void
read_null_string(char *str, FILE *stream, int max_size)
{
  // This function reads a string from a stream
  // until we reach a null character, EOF or the max_size.
  int ch, i = 0;
  while((ch = fgetc(stream)) != '\0' && ch != EOF && i < max_size-1)
    str[i++] = ch;
  str[i] = '\0';
}

int
main(int argc, char *argv[])
{
  if(argc < 2) {
    usage();
    return 1;
  }
  
  char *file = argv[1];
  
  int num;
  
  int data_flags;
  long location_offset;
  int location_size;
  char path[256];
  
  FILE *ptr_file;
  
  ptr_file = fopen(file, "rb");
  if(!ptr_file) {
    perror("lnk");
    return 1;
  }
  
  // LNK header
  fread(&num, 4, 1, ptr_file); // Header size
  
  // The header size is always 76.
  if(num != 76) {
    fprintf(stderr, "Not an LNK file.\n");
    fclose(ptr_file);
    return 1;
  }
  
  // Data flags
  fseek(ptr_file, 20, SEEK_SET);
  fread(&data_flags, 4, 1, ptr_file);
  
  // Skip the header
  fseek(ptr_file, num, SEEK_SET);
  
  // Skip the Link target identifier
  fread(&num, 2, 1, ptr_file);
  fseek(ptr_file, num, SEEK_CUR);
  
  // Location information
  if(data_flags & 0x2) { // HasLinkInfo
    location_offset = ftell(ptr_file);
    fread(&location_size, 4, 1, ptr_file); // Location info size
    fseek(ptr_file, location_offset+4, SEEK_SET); // Location info header size
    fread(&num, 4, 1, ptr_file);
    
    // LOCAL PATH (offset 16)
    fseek(ptr_file, location_offset+16, SEEK_SET); // Get string offset
    fread(&num, 4, 1, ptr_file);
    fseek(ptr_file, location_offset+num, SEEK_SET); // Read
    read_null_string(path, ptr_file, 256);
    
    // Print path
    fprintf(stdout, "%s\n", path);
    
    // Seek into data strings
    //fseek(ptr_file, location_offset+location_size, SEEK_SET);
  }
  else
    fprintf(stderr, "The LNK file doesn't have any location information.\n");
  
  fclose(ptr_file);
  return 0;
}
