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

int l_option = 1;
char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Nov", "Dec"};

int filter(const struct dirent *ent) {
  if (ent->d_name[0] == '.')
    return 0;
  
  return 1;
}

void print(const char* name) {
  struct stat info;
  if (stat(name, &info) == 0 && l_option) {
    struct passwd* pw = getpwuid(info.st_uid);
    struct group* g = getgrgid(info.st_gid);
    struct tm* t = localtime(&info.st_atime);
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
    printf(" "); // format better mmk
    printf("%ld", info.st_nlink);
    printf(" "); // format better mmk
    printf("%s", pw->pw_name);
    printf(" "); // format better mmk
    printf("%s", g->gr_name);
    printf(" "); // format better mmk
    printf("%ld", info.st_size);
    printf(" "); // format better mmk
    printf("%s", months[t->tm_mon]);
    printf(" "); // format better mmk
    printf("%d", t->tm_mday);
    printf(" "); // format better mmk
    printf("%d", t->tm_hour);
    printf(":"); // format better mmk
    printf("%d", t->tm_min);
    printf(" "); // format better mmk
    printf("%s", name);
    printf("\n");
    
  } else {
    printf("%s  ", name);
  }
}

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");
  
  DIR* dirhandle;
  struct dirent** list = NULL;
  int n, fd;
  char* dir = argv[1]; // for now
  
  // scandir will return -1 if there are errors
  n = scandir(dir, &list, filter, alphasort);
  
  if (n == -1 && errno == ENOTDIR) {
    print(dir);
    if (!l_option) printf("\n");
    return 0;
  }
  
  dirhandle = opendir(dir);
  
  if (dirhandle)
    fd = dirfd(dirhandle);
  
  for (int i = 0; i < n; i++)
    print(list[i]->d_name);
  
  if (!l_option) printf("\n");
  
  return 0;
  
}
