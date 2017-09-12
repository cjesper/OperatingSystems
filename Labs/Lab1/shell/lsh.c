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
#include "parse.h"

/*
 * Function declarations
 */
void ExecuteCommand(int, Command *);
void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void rec(Pgm *, int);
int execSingle(char **);
int exec(char **, int, int);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

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
                /* execute it */
                n = parse(line, &cmd);
                // PrintCommand(n, &cmd);
                // ExecuteCommand(n, &cmd);
                if (cmd.pgm->next) {
                    rec(cmd.pgm, 1);
                } else {
                    execSingle(cmd.pgm->pgmlist); 
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
 * Name: ExecuteCommand
 *
 * Description: Executes a command
 *
 */
void ExecuteCommand(int n, Command *cmd) {
    pid_t pid;
    char ** commandList = cmd->pgm->pgmlist;
    int status;

    signal(SIGCHLD, SIG_IGN);
    pid = fork();
    if (pid == 0) {
        // child process
        if(execvp(commandList[0], commandList) == -1) {
            // something went wrong
            fprintf(stderr, "-lsh: %s: ", commandList[0]);
            perror("");
        }
    } else if (pid < 0) {
        // could not fork
        perror("Could not fork");
    } else {
        if (cmd->bakground) {
            // do something?
            // waitpid(pid, &status, WNOHANG);
        } else {
            wait(NULL);
        }
    }
}

#define READ 0
#define WRITE 1
int fds[2];
void rec(Pgm * pgm, int last) {
    pipe(fds);
    
    if (pgm->next != NULL) {
        rec(pgm->next, 0);

        if (last) {
            exec(pgm->pgmlist, fds[READ], 0);
            printf("LAST DONE\n");
        } else {
            exec(pgm->pgmlist, fds[READ], fds[WRITE]);
        }

    } else {
        exec(pgm->pgmlist, 0, fds[WRITE]);
    }
}


int execSingle(char ** args) {
    pid_t pid;

    signal(SIGCHLD, SIG_IGN);
    pid = fork();
    if (pid == 0) {
        // child process
        if(execvp(args[0], args) == -1) {
            // something went wrong
            fprintf(stderr, "-lsh: %s: ", args[0]);
            perror("");
        }
    } else if (pid < 0) {
        // could not fork
        perror("Could not fork");
    } else {
        wait(NULL);
    }
}

int exec(char ** args, int in, int out) {
    int status;
    pid_t pid;
    printf("EXECUTING COMMAND %s, in: %d, out: %d\n", args[0], in, out);
    pid = fork();

    if (pid == 0) {
        // child process
        printf("I AM A CHILD! in: %d, out: %d\n", in, out);
        
        if (in != 0) {
            // read from pipe
            if (dup2(in, STDIN_FILENO) != STDIN_FILENO) {
                perror("dup in child when reading from pipe");
            }
            close(in);
        }

        if (out != 0) {
            // write to pipe
            if (dup2(out, STDOUT_FILENO) != STDOUT_FILENO) {
                perror("dup in child when writing to pipe");
            } 
            close(out);
        }

        if (execvp(args[0], args) == -1) {
            perror("exec()");
        }
    } else if (pid < 0) {
        perror("Could not fork()");
    } else {
        // parent process
        int endID = waitpid(pid, &status, WNOHANG|WUNTRACED);
        // TODO handle child end and make sure no print is done before child prints
        // TODO also make sure bg processes are implemented
        printf("parent done waiting for forking child\n");
    }

    printf("EXECUTION DONE\n");
    return pid;
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
