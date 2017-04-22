#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int l_option = 0;
int a_option = 0;
char currentdir[] = "."; // This might be better if you retrieve it from PWD in the
                         // environment variables. 
char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Nov", "Dec"};

int filter(const struct dirent *ent) {
  if (!a_option && ent->d_name[0] == '.')
    return 0;
  
  return 1;
}

int gettotalblocks(char* dir, struct dirent** list, int n) {
  char path_buf[512];
  strcpy(path_buf, dir);
  if (dir[strlen(dir) - 1] != '/')
    strcat(path_buf, "/");
  
  
  int total = 0;
  struct stat info;
  for (int i = 0; i < n; i++) {
    char t_path_buf[512];
    strcpy(t_path_buf, path_buf);
    if (stat(strcat(t_path_buf, list[i]->d_name), &info) == 0) {
      total += info.st_blocks / 2; // why divide by 2? idk it works
    }
  }
  return total;
}

int getmaxdigit_size(char* dir, struct dirent** list, int n) {
  char path_buf[512];
  strcpy(path_buf, dir);
  if (dir[strlen(dir) - 1] != '/')
    strcat(path_buf, "/");
  
  int max = 1;
  struct stat info;
  for (int i = 0; i < n; i++) {
    char t_path_buf[512];
    strcpy(t_path_buf, path_buf);
    if (stat(strcat(t_path_buf, list[i]->d_name), &info) == 0) {
      int n = info.st_size, d;
      for (d = 0; n != 0; d++) n /= 10;
      if (d > max) max = d;
    }
  }
  return max;
}

int getmaxdigit_links(char* dir, struct dirent** list, int n) {
  char path_buf[512];
  strcpy(path_buf, dir);
  if (dir[strlen(dir) - 1] != '/')
    strcat(path_buf, "/");
  
  int max = 1;
  struct stat info;
  for (int i = 0; i < n; i++) {
    char t_path_buf[512];
    strcpy(t_path_buf, path_buf);
    if (stat(strcat(t_path_buf, list[i]->d_name), &info) == 0) {
      int n = info.st_nlink, d;
      for (d = 0; n != 0; d++) n /= 10;
      if (d > max) max = d;
    }
  }
  return max;
}

void print(char* path, int byte_pad, int link_pad, int num, int it) {
  struct stat info;
  char* name = path;
  for (int i = strlen(path) - 2; i >= 0; i--) {
    if (path[i] == '/') {
      name = path + i + 1;
      break;
    }
  }
  if (stat(path, &info) == 0 && l_option) {
    struct passwd* pw = getpwuid(info.st_uid);
    struct group* g = getgrgid(info.st_gid);
    struct tm* t = localtime(&info.st_mtime);
    printf(S_ISDIR(info.st_mode) ? "d" : "-");
    printf(info.st_mode & S_IRUSR ? "r" : "-");
    printf(info.st_mode & S_IWUSR ? "w" : "-");
    printf(info.st_mode & S_IXUSR ? "x" : "-");
    printf(info.st_mode & S_IRGRP ? "r" : "-");
    printf(info.st_mode & S_IWGRP ? "w" : "-");
    printf(info.st_mode & S_IXGRP ? "x" : "-");
    printf(info.st_mode & S_IROTH ? "r" : "-");
    printf(info.st_mode & S_IWOTH ? "w" : "-");
    printf(info.st_mode & S_IXOTH ? "x" : "-");
    printf(" %*ld", link_pad, info.st_nlink);
    printf(" %s", pw->pw_name);
    printf(" %s", g->gr_name);
    printf(" %*ld", byte_pad, info.st_size);
    printf(" %s", months[t->tm_mon]);
    printf(" %2d", t->tm_mday);
    printf(" %.2d", t->tm_hour);
    printf(":%.2d", t->tm_min);
    printf(" %s", name);
    printf("\n");
  } else {
    printf("%s", name);
    if (it < (num-1)) printf("  ");
  }
}

int main(int argc, char** argv) {
  
  // Get options yo
  int c;
  while (1) {
    c = getopt(argc, argv, "la");
    if (c == -1) break; // done reading options
    
    switch (c) {
      case 'l':
        l_option = 1;
        break;
      case 'a':
        a_option = 1;
        break;
    }
  }
  
  // This sets the ordering for alphasort
  setlocale(LC_ALL, "");
  
  DIR* dirhandle;
  struct dirent** list = NULL;
  int n, fd;
  
  char* dir;
  if (argv[optind] == NULL) dir = currentdir;
  else dir = argv[optind];
  
  // Scan, filter, and sort all directories and files and put them in list
  n = scandir(dir, &list, filter, alphasort);
  
  // If it is not a directory, it's probably a file
  if (n == -1) {
    if (errno == ENOTDIR) {
      print(dir, 0, 0, 1, 0);
      if (!l_option) printf("\n");
      return 0;
    }
    fprintf(stderr, "myls: ");
    perror(dir);
    return errno;
  }
  
  dirhandle = opendir(dir);
  
  if (dirhandle)
    fd = dirfd(dirhandle); // use this for...well I don't know
  
  if (l_option) 
    printf("total %d\n", gettotalblocks(dir, list, n));
  
  // make it look pretty
  int byte_pad = getmaxdigit_size(dir, list, n);
  int link_pad = getmaxdigit_links(dir, list, n);
  
  char path_buf[512];
  strcpy(path_buf, dir);
  if (dir[strlen(dir) - 1] != '/')
    strcat(path_buf, "/");
  for (int i = 0; i < n; i++) {
    char t_path_buf[512];
    strcpy(t_path_buf, path_buf);
    print(strcat(t_path_buf, list[i]->d_name), byte_pad, link_pad, n, i);
  }
  
  if (!l_option) printf("\n");
  
  return 0;
  
}
