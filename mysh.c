#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

char** splitString(char*, int*);
void execmd(int, char**);

int main(int argc, char** argv) {
  
  int running = 1;
  while (running) {
    char cmd_buf[512];
    char t_cmd_buf[512]; // temporary buffer to preserve cmd_buf
    char bin[] = "/bin/";
    char usr_bin[] = "/usr/bin/";
    int nargc = 0;
    
    printf("mysh $ ");
    fgets(cmd_buf, 512, stdin);
    cmd_buf[strcspn(cmd_buf, "\n")] = 0; // get rid of that pesky new line
    
    if (strcmp(cmd_buf, "") != 0) {
      
      char** nargv = splitString(strcpy(t_cmd_buf, cmd_buf), &nargc);
      
      if (access(nargv[0], X_OK) == 0) { // if the command is in current folder
        execmd(nargc, nargv);
      } else if(access(strcat(bin, nargv[0]), X_OK) == 0) { // check /bin
        nargv[0] = bin;
        execmd(nargc, nargv);
      } else if(access(strcat(usr_bin, nargv[0]), X_OK) == 0) { // check /usr/bin
        nargv[0] = usr_bin;
        execmd(nargc, nargv);
      } else if (!strcmp(nargv[0], "exit")) {
        running = 0; // maybe do some shutdown stuff after
      } else {
        printf("%s: command not found\n", nargv[0]);
      }
      
      free(nargv);
      
    }
  }
  
  return 0;
  
}

void execmd(int argc, char** argv) {
  pid_t cpid;
  cpid = fork();
  
  if (cpid >= 0) {
    if (cpid == 0) { // child
      execv(argv[0], argv);
      exit(0);
    } else { // parent
      wait(NULL); // wait for all child processes to finish
    }
  } else {
    // print informative and helpful error message
  }
}

char** splitString(char* str, int* strc) {
  char** ret = NULL;
  char* tok = strtok(str, " ");
  int n = 0;
  
  while (tok) {
    ret = realloc(ret, sizeof(char*) * ++n);
    ret[n - 1] = tok;
    tok = strtok(NULL, " ");
  }
  
  ret = realloc(ret, sizeof(char*) * (n+1));
  ret[n] = 0; //null terminator for array...apparently is necessary in c
  
  *strc = n;
  
  return ret;
}

