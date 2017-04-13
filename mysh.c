#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

char** splitString(char*, int*);
int mycd(char*);
void execmd(int, char**);

extern char** environ;

void pipetest() {
  int fd[2];
  pipe(fd);
  
  if (fork() == 0) {
    dup2(fd[STDOUT_FILENO], STDOUT_FILENO);
    close(fd[STDIN_FILENO]);
    close(fd[STDOUT_FILENO]);
    
    char* prog1[] = {"myls", "-l", 0};
    execve(prog1[0], prog1, environ);
    perror("myls pipe");
    exit(1);
  }
  
  if (fork() == 0) {
    dup2(fd[STDIN_FILENO], STDIN_FILENO);
    close(fd[STDOUT_FILENO]);
    close(fd[STDIN_FILENO]);
    
    char* prog2[] = {"/bin/grep", "rwx", 0};
    execve(prog2[0], prog2, environ);
    perror("grep pipe");
    exit(1);
  }
  
  close(fd[STDIN_FILENO]);
  close(fd[STDOUT_FILENO]);
  wait(0);
  wait(0);
  
  // well... it works, finally. Now we just need to parse | in the command line
}

int main(int argc, char** argv) {
  
  int running = 1;
  while (running) {
    char cmd_buf[512];
    char t_cmd_buf[512]; // temporary buffer to preserve cmd_buf
    char bin[] = "/bin/";
    char usr_bin[] = "/usr/bin/";
    int tokc = 0;
    
    printf("mysh $ ");
    fgets(cmd_buf, 512, stdin);
    cmd_buf[strcspn(cmd_buf, "\n")] = 0; // get rid of that pesky new line
    
    if (strcmp(cmd_buf, "") != 0) {
      
      char** tokv = splitString(strcpy(t_cmd_buf, cmd_buf), &tokc);
      
      if (access(tokv[0], X_OK) == 0) { // if the command is in current folder
        execmd(tokc, tokv);
      } else if(access(strcat(bin, tokv[0]), X_OK) == 0) { // check /bin
        tokv[0] = bin;
        execmd(tokc, tokv);
      } else if(access(strcat(usr_bin, tokv[0]), X_OK) == 0) { // check /usr/bin
        tokv[0] = usr_bin;
        execmd(tokc, tokv);
      } else if (strcmp(tokv[0], "mycd") == 0) {
        mycd(tokv[1]); // check if NULL inside mycd, if it is chdir to $HOME
      } else if (strcmp(tokv[0], "mypwd") == 0) {
        printf("%s\n", getenv("PWD")); // i guess that's it lol
      } else if (strcmp(tokv[0], "exit") == 0) {
        running = 0;
      } else if (strcmp(tokv[0], "pipetest") == 0) {
        pipetest();
      } else {
        printf("%s: command not found\n", tokv[0]);
      }
      
      free(tokv);
      
    }
  }
  
  return 0;
  
}

void execmd(int argc, char** argv) {
  pid_t cpid;
  cpid = fork();
  
  if (cpid >= 0) {
    if (cpid == 0) { // child
      execve(argv[0], argv, environ);
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

int mycd(char* dir) {
  // do chdir here
}

