#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

struct command {
  char** argv;
  int argc;
  int rout;
  int routtar;
  int rin;
  int rintar;
};

int mycd(char*);
char** splitpath();
void getarg(char*, char**, int, int, int);
void exe(struct command);
void proc(int, int, struct command);
void run(int, struct command*);
void cleanup(int, struct command*);
int istokterm(char);

extern char** environ;

int main(int argc, char** argv) {
  // add current directory to PATH
  char* path = getenv("PATH");
  char* wd = getenv("PWD");
  char* newpath = (char*) malloc(strlen(path) + strlen(wd) + 7);
  strcpy(newpath, "PATH=");
  strcat(newpath, path);
  strcat(newpath, ":");
  strcat(newpath, wd);
  putenv(newpath);
  
  int running = 1;
  while (running) {
    char cmd_buf[512];
    
    printf("mysh $ ");
    fgets(cmd_buf, 512, stdin);
    cmd_buf[strcspn(cmd_buf, "\n")] = 0; // get rid of that pesky new line
    
    if (strcmp(cmd_buf, "") != 0) {
      int num_cmds = 0;
      struct command* cmd = NULL;
      cmd = realloc(cmd, sizeof(struct command));
      cmd[num_cmds].rout = cmd[num_cmds].rin = 0;
      int readingCommand = cmd_buf[0] != ' ';
      char** args = NULL;
      int argc = 0;
      int r = 0, l = 0, c;
      // skip whitespace
      while (cmd_buf[r] == ' ') r++;
      l = r;
      while (cmd_buf[r] != 0) {
        // alright so this is gonna be ugly from here on...sorry
        // this is what interprets the commands line
        switch (cmd_buf[r]) {
          case ' ': // args
            args = realloc(args, sizeof(char*) * ++argc);
            getarg(cmd_buf, args, argc, r, l);
            // skip whitespace
            while (cmd_buf[r] == ' ') r++;
            l = r;
            break;
          case '>': // redirect output
          case '<': // redirect input
            c = r;
            if (r != l) {
              args = realloc(args, sizeof(char*) * ++argc);
              getarg(cmd_buf, args, argc, r, l);
            }
            // skip whitespace
            do r++; while (cmd_buf[r] == ' ');
            l = r;
            while (!istokterm(cmd_buf[r])) r++;
            char* path = (char*) malloc(r - l);
            memcpy(path, &cmd_buf[l], r - l);
            path[r - l] = 0;
            // skip whitespace
            while (cmd_buf[r] == ' ') r++;
            l = r;
            if (cmd_buf[c] == '>') {
              cmd[num_cmds].routtar = open(path, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
              );
              cmd[num_cmds].rout = 1;
              //close(cmd[num_cmds].routtar);
            } else if (cmd_buf[c] == '<') {
              cmd[num_cmds].rintar = open(path, O_RDONLY);
              cmd[num_cmds].rin = 1;
              //close(cmd[num_cmds].rintar);
            }
            break;
          case '|': // pipe
            if (r != l) {
              args = realloc(args, sizeof(char*) * ++argc);
              getarg(cmd_buf, args, argc, r, l);
            }
            args = realloc(args, sizeof(char*) * argc + 1);
            args[argc] = 0;
            cmd[num_cmds].argv = args;
            cmd[num_cmds].argc = argc;
            num_cmds++;
            cmd = realloc(cmd, sizeof(struct command) * (num_cmds + 1));
            cmd[num_cmds].rout = cmd[num_cmds].rin = 0;
            args = NULL;
            argc = 0;
            // skip whitespace
            do r++; while (cmd_buf[r] == ' ');
            l = r;
            break;
          case ';': // not complete, idk...
            break;
          default:
            r++;
            break;
        }
      }
      if (r != l) { 
        args = realloc(args, sizeof(char*) * ++argc);
        getarg(cmd_buf, args, argc, r, l);
      }
      // end the arguments with NULL for c!
      args = realloc(args, sizeof(char*) * argc + 1);
      args[argc] = 0;
      cmd[num_cmds].argv = args;
      cmd[num_cmds].argc = argc;
      num_cmds++;
      
      if (strcmp(cmd[num_cmds - 1].argv[0], "exit") == 0) {
        //just kill it...
        exit(0);
      } else if (strcmp(cmd[num_cmds - 1].argv[0], "mycd") == 0) {
        mycd(cmd[num_cmds - 1].argv[1]);
        cleanup(num_cmds, cmd);
        continue;
      } else if (strcmp(cmd[num_cmds - 1].argv[0], "mypwd") == 0) {
        char* wd = (char*) malloc(1024);
        getcwd(wd, 1024);
        printf("%s\n", wd);
        free(wd);
        cleanup(num_cmds, cmd);
        continue;
      }
      
      run(num_cmds, cmd);
      cleanup(num_cmds, cmd);
    }
  }
  return 0;
}

void cleanup(int n, struct command* cmd) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < cmd[i].argc; j++) {
      free(cmd[i].argv[j]);
    }
    free(cmd[i].argv);
  }
  free(cmd);
}

int mycd(char* dir) {
  int ret;
  char *directory = dir;
  ret = chdir(directory);
  if(ret == 0){
    return 0;
  }
  return 1;
}

char** splitpath() {
  char** ret = NULL;
  char* temp = (char*) malloc(strlen(getenv("PATH")) + 1);
  strcpy(temp, getenv("PATH"));
  char* tok = strtok(temp, ":");
  int n = 0;
  
  while (tok) {
    ret = realloc(ret, sizeof(char*) * ++n);
    ret[n - 1] = tok;
    tok = strtok(NULL, ":");
  }
  
  ret = realloc(ret, sizeof(char*) * (n+1));
  ret[n] = 0; // end with NULL for the c!
  
  return ret;
}

void getarg(char* cmd_buf, char** args, int argc, int r, int l) {
  args[argc - 1] = (char*) malloc(r - l);
  memcpy(args[argc - 1], &cmd_buf[l], r - l);
  args[argc - 1][r - l] = 0;
}

// has the "token" been terminated?
int istokterm(char c) {
  return c == 0 || c == ' ' || c == '|' || c == ';' || c == '>' || c == '<';
}

void exe(struct command c) {
  char** paths = splitpath();
  int i = 0;
  int foundexe = 0;
  while (paths[i]) {
    char* path = (char*) malloc(strlen(paths[i]) + strlen(c.argv[0]) + 2);
    strcpy(path, paths[i]);
    strcat(path, "/");
    if (access(strcat(path, c.argv[0]), X_OK) == 0) {
      free(c.argv[0]);
      c.argv[0] = path;
      foundexe = 1;
      break;
    } else {
      free(path);
    }
    i++;
  }
  
  if (foundexe) {
    if (c.rin) {
      dup2(c.rintar, 0);
      close(c.rintar);
    }
    if (c.rout) {
      dup2(c.routtar, 1);
      close(c.routtar);
    }
    execve(c.argv[0], c.argv, environ);
  } else {
    printf("%s: command not found\n", c.argv[0]);
    exit(1);
  }
}

void proc(int in, int out, struct command c) {
  pid_t pid;
  
  if ((pid = fork()) == 0) {
    if (in != 0) {
      dup2(in, 0);
      close(in);
    }
    if (out != 1) {
      dup2(out, 1);
      close(out);
    }
    exe(c);
  }
  wait(0);
}

void run(int n, struct command* cmd) {
  int in, fd[2];
  
  // first command gets input from the good ol stdin
  in = 0;
  
  int i;
  for (i = 0; i < n - 1; i++)  {
    pipe(fd);
    proc(in, fd[1], cmd[i]);
    close(fd[1]);
    in = fd[0];
  }
  // run command at end of pipeline
  proc(in, 1, cmd[i]);
}
