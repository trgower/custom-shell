#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv){
  if (argc < 2) {                        // if no argument given, echo user input
    char input[512];                     // allocating enough space for user input
    while(fgets(input, 512, stdin)) {    // read in and store user input
      printf("%s", input);               // print to screen input from user
      memset(input, 0, strlen(input));   // reset our string to get ready for next
    }
  } else {
    for (int i = 1; i < argc; i++) {          // iterate through arguments passed
      int fileDesc = open(argv[i], O_RDONLY); // ensure we can find/open desired files
      if(fileDesc < 0) {                      // if unable to find file
        fprintf(stderr, "mycat: ");
        perror(argv[i]);                      // proper error handling, maybe idk
      }
      FILE *file = fopen(argv[i], "r");       // open file read only
      int c;
      if (file) {
        while ((c = getc(file)) != EOF)       // read from file line by line
          fprintf(stdout, "%c" , c);          // stdout each line of file
        fclose(file);                         // close it all up
      }
    }
  }
  return 0;
}
