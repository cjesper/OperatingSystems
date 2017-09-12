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
void ExecuteSingleCommand(char **, int);
void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

#define READ 0
#define WRITE 1

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
                ExecuteCommand(n, &cmd);
                // PrintCommand(n, &cmd);
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
    char ** args = cmd->pgm->pgmlist;
    int fds[2];
    pipe(fds);
    
    dup2(fds[WRITE], STDOUT_FILENO);
    ExecuteSingleCommand(cmd->pgm->next->pgmlist, cmd->bakground);
    close(fds[WRITE]);
    dup2(fds[READ], STDIN_FILENO);
    ExecuteSingleCommand(cmd->pgm->pgmlist, cmd->bakground);
    close(fds[READ]);
}


/**
 * Name: ExecuteSingleCommand
 *
 * Description: Executes a single command, when piping is not necessary
 *
 */
void ExecuteSingleCommand(char ** args, bg) {
    pid_t pid;
    int status;

    // handle program exit
    if (strcmp("exit", args[0]) == 0) {
        done = 1;
        return;
    }

    signal(SIGCHLD, SIG_IGN); // kill children

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
        if (bg) {
            // do something?
            // waitpid(pid, &status, WNOHANG);
        } else {
            wait(NULL);
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
