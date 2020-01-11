/*
 * Simple hexudmper (1 col = address, 2nd = value in hex, 3nd = char if printable or '.')
 */			
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define BUFFERSIZE 2048
int main(int argc, char **argv) {
  FILE *file;
  int address = 0;
  unsigned char BUFFER[BUFFERSIZE];
  int bytesread;

  if (argc == 2) {
    if ( (file=fopen(argv[1], "r")) == NULL ) {
      printf("Error: Can't open %s\n", argv[0]);
      exit(1);
    }
  }
  else { file = stdin; }
  while(bytesread=fread(BUFFER, sizeof(char), BUFFERSIZE, file)) {
    int i;
    char c;
    for(i=0; i<bytesread; i++) {
      if (isgraph(BUFFER[i])) { c=BUFFER[i]; }
      else if (isspace(BUFFER[i])) { c=' '; }
      else { c='.'; }
      
      printf("%04x %02x %c", address++, BUFFER[i], c);
      if (BUFFER[i]==0xf0) { printf(" --start SYSEX"); }
      printf("\n");
    }
  }
  if (file != stdin) { fclose(file);}
  exit(0);
}
  
    
