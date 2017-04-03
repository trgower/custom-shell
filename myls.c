#include <stdio.h>
#include <dirent.h>

int main(int argc, char** argv) {
  
  DIR *dir;
  struct dirent *s_dir;
  dir = opendir(argv[1]);
  
  if (dir) {
    while ((s_dir = readdir(dir)) != NULL) {
      printf("%s  ", s_dir->d_name);
    }
    printf("\n");
    closedir(dir);
  }
  
  return 0;
  
}
