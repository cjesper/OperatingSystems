/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
            $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */


#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include "parse.h"

/*
 * Function declarations
 */
void interruptHandler(int);
void childHandler(int);
void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void interruptHandler(int);
void execute(Pgm *, int, int, int);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

/*
 * Constans used when accessing FDs
 */
#define READ 0
#define WRITE 1

/*
 * Ignore interrupt signal and print new line for better output.
 */
void interruptHandler(int sig) {
    signal(SIGINT, SIG_IGN);
    printf("\n");
}

/*
 * Wait for termination in child to avoid zombie.
 */
void childHandler(int sig) {
    while(waitpid((pid_t)-1, 0, WNOHANG) > 0){}
}

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
    Command cmd;
    int n;

    /* Stash stdin and stdout for further use */
    int saved_stdout;
    saved_stdout = dup(STDOUT_FILENO);
    int saved_stdin;
    saved_stdin = dup(STDIN_FILENO);

    /* Register handler for interrupt */
    signal(SIGINT, interruptHandler);

    /* Struct used to call childHandler in background processes,
     * to avoid turning into zombies. */
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = childHandler;

    while (!done) {
        char *line;
        line = readline("> ");

        if (!line) {
            /* Encountered EOF at top level */
            done = 1;
        } else {
            /*
             * Remove leading and trailing whitespace from the line
             * Then, if there is anything left, add it to the history list
             * and execute it.
             */
            stripwhite(line);

            if(*line) {
                add_history(line);
                n = parse(line, &cmd);

                /* Handle user exit */
                if (strcmp(cmd.pgm->pgmlist[0], "exit") == 0) {
                    printf("Goodbye.\n");
                    exit(EXIT_SUCCESS);
                }
                
                /* Make it possible for user to change directory */
                if (strncmp(cmd.pgm->pgmlist[0], "cd", 2) == 0) {
                    chdir(cmd.pgm->pgmlist[1]);
                    continue;
                }

                /* If applicable, redirect stdin and stdout to files */
                int stdinFD = 0, stdoutFD = 0;
                if (cmd.rstdin != NULL) {
                   stdinFD = open(cmd.rstdin, O_CREAT|O_RDWR);
                   if (stdinFD < 0) {
                       perror("Could not create file descriptor");
                   }
                   dup2(stdinFD, STDIN_FILENO);
                } else {
                    stdinFD = saved_stdin;                
                }
                if (cmd.rstdout != NULL) {
                   stdoutFD = open(cmd.rstdout, O_CREAT|O_RDWR, 0644);
                   if (stdoutFD < 0) {
                       perror("Could not create file descriptor");
                   }
                   dup2(stdoutFD, STDOUT_FILENO);
                } else {
                    stdoutFD = saved_stdout;
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("could not fork");
                } else if (pid == 0) {
                    /* Child process initiates execution sequence */
                    execute(cmd.pgm, cmd.bakground, stdinFD, stdoutFD);
                } else {
                    /* Let parent wait for child if running in background */
                    if (!cmd.bakground) {
                        waitpid(pid, NULL, 0);
                    } else {
                        sigaction(SIGCHLD, &sa, NULL);
                        printf("Command %s started in background with PID [%d]\n", cmd.pgm->pgmlist[0], pid);
                    }

                    /* Restore stdin and stdout */
                    dup2(saved_stdin, STDIN_FILENO);
                    dup2(saved_stdout, STDOUT_FILENO);
                }
            }
        }
        
        if(line) {
            free(line);
        }
    }
    return 0;
}


/*
 * Recursive method for executing commands.
 */
void execute(Pgm * pgm, int background, int stdinFD, int stdoutFD) {
    
    /* When last command is reached, execute command start returning to the other commands */
    if (pgm->next == NULL) {
        if (execvp(pgm->pgmlist[0], pgm->pgmlist) < 0) {
            perror("could not execute function");
            exit(EXIT_FAILURE);
        }
    } else {
        pid_t pid;

        int fds[2];
        int result;
        fds[READ] = stdinFD;
        fds[WRITE] = stdoutFD;

        if (pipe(fds) != 0) {
            perror("pipe error");
        }

        /* Start execution chain */
        if ((pid = fork()) < 0) {
            perror("could not fork");
        } else if (pid == 0) {
            dup2(fds[WRITE], STDOUT_FILENO);
            close(fds[WRITE]);
            execute(pgm->next, background, stdinFD, stdoutFD);
        } else {
            dup2(fds[READ], STDIN_FILENO);
            close(fds[READ]);
            close(fds[WRITE]);

            wait(NULL);

            if (execvp(pgm->pgmlist[0], pgm->pgmlist) < 0) {
                perror("Could not execute function");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void PrintCommand(int n, Command *cmd) {
    printf("Parse returned %d:\n", n);
    printf("     stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
    printf("     stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
    printf("     bg      : %s\n", cmd->bakground ? "yes" : "no");
    PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void PrintPgm(Pgm *p) {
    if (p == NULL) {
        return;
    } else {
        char **pl = p->pgmlist;

        /* The list is in reversed order so print
         * it reversed to get right
         */
        PrintPgm(p->next);
        printf("        [");
        while (*pl) {
            printf("%s ", *pl++);
        }
        printf("]\n");
    }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void stripwhite(char *string) {
    register int i = 0;

    while (isspace( string[i] )) {
        i++;
    }
    
    if (i) {
        strcpy (string, string + i);
    }

    i = strlen( string ) - 1;
    while (i> 0 && isspace (string[i])) {
        i--;
    }

    string [++i] = '\0';
}
