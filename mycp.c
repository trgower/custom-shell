#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

int R_option = 0;

void copyFile(char* infile, char* outfile)
{
  int in;
  int out;
  char temp[512];
  
  struct stat info;
  mode_t m;
  if (stat(infile, &info) == 0 && !S_ISDIR(info.st_mode)) {
    if((in = open(infile, O_RDONLY)) < 0) {
      fprintf(stderr, "mycp: ");
      perror(infile);
      exit(errno);
    }

    if((out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, info.st_mode)) < 0) {
      fprintf(stderr, "mycp: ");
      perror(outfile);
      exit(errno);
    } else {
      memset(temp, 0, strlen(temp)); // for recursion! yay c!
      while(read(in, temp, sizeof(temp))) {
        write(out, temp, strlen(temp));
        memset(temp, 0, strlen(temp));
      }
    }
  } else {
    fprintf(stderr, "mycp: You must use the -R option to copy directories\n");
    exit(1);
  }
  
  close(in);
  close(out);
  
}

void SearchDirectory(const char *name, const char *name2) {
  DIR *dir = opendir(name);               
  if (dir) {
    // create name2 if it doesn't exist
    struct stat test, odir;
    if (stat(name2, &test) == -1) 
      if (stat(name, &odir) == 0) 
        mkdir(name2, odir.st_mode);
    
    char Path[256], *EndPtr = Path;
    char Path2[256], *EndPtr2 = Path2;
    struct dirent *e;
  
    strcpy(Path, name);                 
    EndPtr += strlen(name); 
    strcpy(Path2, name2);                 
    EndPtr2 += strlen(name2); 
    
    while((e = readdir(dir)) != NULL) {
      if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0) {
        strcpy(EndPtr, "/"); 
        strcat(EndPtr, e->d_name);  
        
        strcpy(EndPtr2, "/"); 
        strcat(EndPtr2, e->d_name);
        
        struct stat info;
        if(stat(Path, &info) == 0) {        
          if(S_ISDIR(info.st_mode)) {
            SearchDirectory(Path, Path2);
          } else if(S_ISREG(info.st_mode)) {
            copyFile(Path, Path2);          
          }
        }
      }
    }
  } else {
    fprintf(stderr, "mycp: ");
    perror(name);
    exit(errno);
  }
  closedir(dir);
}

int main(int argc, char** argv) {
  
  if (argc < 3) {
    printf("mycp: Usage: mycp srcfile destfile - for single files\n");
    printf("             mycp -R srcdir destdir - for directories\n");
    exit(1);
  }
  
  // Get options yo
  int c;
  while (1) {
    c = getopt(argc, argv, "R");
    if (c == -1) break; // done reading options
    
    switch (c) {
      case 'R':
        R_option = 1;
        break;
      default:
        exit(errno);
        break;
    }
  }
  
  if (R_option)
    SearchDirectory(argv[optind], argv[optind + 1]);
  else
    copyFile(argv[optind], argv[optind + 1]);
  
  return 0;
  
}


