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
void exe(struct command*, pid_t pid);
void proc(int, int, struct command);
void run(int, struct command*);
void cleanup(struct command*);
int cmdarg(struct command);
struct command* interpretline(char*, int*);
int istokterm(char);
char** splitString(char*, int*);

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
      int n = 0;
      struct command* cmd = interpretline(cmd_buf, &n);
      if (cmd != 0 && n > 0) {
        run(n, cmd);
        cleanup(cmd);
      }
    }
  }
  return 0;
}

struct command* interpretline(char* cmd_buf, int* num_cmds) {
  struct command* cmd = NULL;
  cmd = realloc(cmd, sizeof(struct command));
  cmd[*num_cmds].rout = cmd[*num_cmds].rin = 0;
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
          cmd[*num_cmds].routtar = open(path, O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
          );
          cmd[*num_cmds].rout = 1;
          //close(cmd[*num_cmds].routtar);
        } else if (cmd_buf[c] == '<') {
          cmd[*num_cmds].rintar = open(path, O_RDONLY);
          cmd[*num_cmds].rin = 1;
          //close(cmd[*num_cmds].rintar);
        }
        break;
      case '|': // pipe
        if (r != l) {
          args = realloc(args, sizeof(char*) * ++argc);
          getarg(cmd_buf, args, argc, r, l);
        }
        args = realloc(args, sizeof(char*) * (argc + 1));
        args[argc] = 0;
        cmd[*num_cmds].argv = args;
        cmd[*num_cmds].argc = argc;
        (*num_cmds)++;
        cmd = realloc(cmd, sizeof(struct command) * (*num_cmds + 1));
        cmd[*num_cmds].rout = cmd[*num_cmds].rin = 0;
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
  args = realloc(args, sizeof(char*) * (argc + 1));
  args[argc] = 0;
  cmd[*num_cmds].argv = args;
  cmd[*num_cmds].argc = argc;
  (*num_cmds)++;

  if (strcmp(cmd[*num_cmds - 1].argv[0], "exit") == 0) {
    //just kill it...
    exit(0);
  } else if (strcmp(cmd[*num_cmds - 1].argv[0], "mycd") == 0) {
    mycd(cmd[*num_cmds - 1].argv[1]);
    cleanup(cmd);
    return 0;
  } else if (strcmp(cmd[*num_cmds - 1].argv[0], "mypwd") == 0) {
    char wd[1024];
    getcwd(wd, 1024);
    printf("%s\n", wd);
    cleanup(cmd);
    return 0;
  }
  
  return cmd;
}

void cleanup(struct command* cmd) {
  if (cmd != NULL) 
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
  char* temp = (char*) malloc(strlen(getenv("PATH")) + 2);
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
  args[argc - 1] = malloc(r - l + 1);
  memcpy(args[argc - 1], &cmd_buf[l], r - l);
  args[argc - 1][r - l] = 0;
}

// has the "token" been terminated?
int istokterm(char c) {
  return c == 0 || c == ' ' || c == '|' || c == ';' || c == '>' || c == '<';
}

int cmdarg(struct command c) {
  for (int i = 0; i < c.argc; i++) {
    if (c.argv[i][0] == '$' && c.argv[i][1] == '(') {
      return i;
    }
  }
  return 0;
}

void exe(struct command* c, pid_t pid) {
  char** paths = splitpath();
  int i = 0;
  int foundexe = 0;
  while (paths[i]) {
    char* path = (char*) malloc(strlen(paths[i]) + strlen(c->argv[0]) + 2);
    strcpy(path, paths[i]);
    strcat(path, "/");
    if (access(strcat(path, c->argv[0]), X_OK) == 0) {
      c->argv[0] = path;
      foundexe = 1;
      break;
    } else {
      //free(path);
    }
    i++;
  }
  if (foundexe) {
    if (c->rin) {
      dup2(c->rintar, 0);
      close(c->rintar);
    }
    if (c->rout) {
      dup2(c->routtar, 1);
      close(c->routtar);
    }
    execve(c->argv[0], c->argv, environ);
  } else {
    printf("%s: command not found\n", c->argv[0]);
    exit(1);
  }
}

void proc(int in, int out, struct command c) {
  pid_t pid;
  int ai;
  if (ai = cmdarg(c)) {
    int fd[2];
    pipe(fd);
    pid_t apid;
    if ((apid = fork()) == 0) {
      int len = strlen(c.argv[ai]);
      char* cmd_buf = (char*) malloc(len);
      memcpy(cmd_buf, &c.argv[ai][2], len - 3);
      cmd_buf[len - 3] = 0;
      int n = 0;
      struct command* cmd = interpretline(cmd_buf, &n);
      close(fd[0]);
      dup2(fd[1], 1);
      dup2(fd[1], 2);
      close(fd[1]);
      exe(cmd, 1337);
    } else {
      char* buffer = (char*) malloc(2048);
      close(fd[1]);
      read(fd[0], buffer, 2048);
      int argc = 0;
      char** argv = splitString(buffer, &argc);
      c.argc = c.argc + argc - 1;
      c.argv = realloc(c.argv, sizeof(char*) * (c.argc + 1));
      for (int i = ai + 1; (i + argc - 2) < c.argc; i++) {
        c.argv[i + argc - 2] = c.argv[i];
      }
      for (int i = 0; i < argc; i++) {
        c.argv[i + ai] = argv[i];
        c.argv[i + ai][strcspn(c.argv[i + ai], "\n")] = 0;
      }
    }
    wait(0);
  }
  
  if ((pid = fork()) == 0) {
    if (in != 0) {
      dup2(in, 0);
      close(in);
    }
    if (out != 1) {
      dup2(out, 1);
      close(out);
    }
    exe(&c, pid);
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
  ret[n] = 0; 
  
  *strc = n;
  
  return ret;
}
