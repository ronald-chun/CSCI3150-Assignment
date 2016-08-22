#include <stdio.h>
#include <limits.h> // Needed by PATH_MAX
#include <unistd.h> // Needed by getcwd()
#include <string.h>
#include <stdlib.h> // Nedded by setenv()
#include <errno.h> // Needed by errno
#include <sys/wait.h>
#include <sys/types.h>
#include <glob.h> // Needed by glob()
#include <signal.h>



int main(int argc, char *argv[]) {
    // while(1) {
    int c = 1;
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    while(c) {
        char input[255];
        char cwd[PATH_MAX+1];
        glob_t globbuf;
        globbuf.gl_offs = 1;
        getcwd(cwd,PATH_MAX+1);

        printf("[3150 shell:%s]$ ", cwd);
        fgets(input,255,stdin);
        if (input[0] == 0) {
            c = 0;
            printf("\n[Process completed]\n");
            continue;
        }
        input[strlen(input)-1] = '\0';
        char *token = strtok(input," ");

        while(token != NULL) {
            if(strcmp(input, "cd") == 0) {
                // printf("%s: ",token);
                // printf("Change directory\n");
                token = strtok(NULL," ");
                if (token == NULL) {
                    printf("cd: wrong number of arguments\n");
                } else {
                    // printf("Change to: %s\n", token);
                    char *tmp = strtok(NULL," ");
                    if (tmp != NULL) {
                        printf("cd: wrong number of arguments\n");
                        break;
                    } else {
                        if(chdir(token) != -1) {
                            getcwd(token,PATH_MAX+1);
                            // printf("Now it is %s\n",token);
                            break;
                        } else {
                            printf("[%s]: cannot change directory\n", token);
                            break;
                        }
                    }
                }
            } else if(strcmp(input, "exit") == 0) {
                char *tmp = strtok(NULL," ");
                if (tmp != NULL) {
                    printf("exit: wrong number of arguments\n");
                    break;
                } else {
                    exit(0);
                }
            } else {
                pid_t child_pid;
                setenv("PATH","/bin:/usr/bin:.", 1);

                // if(!fork()) {
                if(!( child_pid = fork() ) ) {
                    signal(SIGINT,SIG_DFL);
                    signal(SIGQUIT,SIG_DFL);
                    signal(SIGTERM,SIG_DFL);
                    signal(SIGTSTP,SIG_DFL);
                    char **argList = (char**) malloc(sizeof(char*) * 255);
                        argList[0] = (char*)malloc(sizeof(char));
                        argList[0] = token;
                        token = strtok(NULL," ");

                    int globCount = 0;
                    int loopCount = 1;
                    int star = 0;
                    while (token != NULL) {
                        for (int i = 0; i < strlen(token); i++) {
                            if (token[i] == 42) {
                                star = 1;
                            }
                        }
                        if (star == 1) {
                            if(globCount == 0) {
                                glob(token, GLOB_DOOFFS | GLOB_NOCHECK, NULL, &globbuf);
                                globCount = 1;
                            } else {
                                glob(token, GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND, NULL, &globbuf);
                            }
                            token = strtok(NULL," ");
                        } else {
                            argList[loopCount] = (char*)malloc(sizeof(char));
                            argList[loopCount] = token;
                            token = strtok(NULL," ");
                        }
                        loopCount++;
                    }

                    if (globCount == 1) {
                        globbuf.gl_pathv[0] = argList[0];
                        execvp(globbuf.gl_pathv[0],globbuf.gl_pathv);
                    } else {
                        execvp(*argList, argList);
                    }

                    if(errno == ENOENT) {
                        printf("[%s]: command not found\n", argList[0]);
                    } else {
                        printf("[%s]: unknown error\n", argList[0]);
                    }

                    return 0;
                } else {
                    wait(NULL);
                }
                break;
            }
        }
    }
    return 0;
}
