#include "linux/limits.h"
#include <unistd.h>
#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define INPUT_MAX_SIZE 48
#define EXECUTION_FAILED 3
#define BUFFER_SIZE 12
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

char buffer[BUFFER_SIZE];
/* returns the decimal representation of a _signed_ integer */
char *itoa(int num)
{

    char *p = buffer + BUFFER_SIZE - 1;
    int neg = num < 0;

    if (neg)
    {
        num = -num;
    }

    *p = '\0';
    do
    {
        *(--p) = '0' + num % 10;
    } while (num /= 10);

    if (neg)
    {
        *(--p) = '-';
    }

    return p;
}

process *list_append(process *process_list, process *toAdd)
{
    process *curr = process_list;
    if (process_list == NULL)
    {
        return toAdd;
    }
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = toAdd;
    return process_list;
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    int status;
    pid_t ret = waitpid(pid, &status, WNOHANG);
    process *toAdd = (process *)malloc(sizeof(process));
    toAdd->cmd = (cmdLine *)malloc(sizeof(cmdLine));
    toAdd->cmd = cmd;
    toAdd->pid = pid;
    toAdd->status = (WIFEXITED(status))    ? TERMINATED  :
                    (WIFSIGNALED(status))  ? SUSPENDED   :
                    (WIFCONTINUED(status)) ? RUNNING     :
                    EXECUTION_FAILED; // should never happen!
    if (toAdd->status == EXECUTION_FAILED) abort();
    toAdd->next = NULL;
    *process_list = list_append((*process_list), toAdd);
}

void printProcessList(process **process_list)
{
    process *curr = *(process_list);
    char *pid_str;
    char *stat;
    updateProcessList(process_list);
    while (curr != NULL)
    {
        pid_str = itoa(curr->pid);
        stat = (curr->status == RUNNING) ? "Running" : (curr->status == TERMINATED) ? "Terminated"
                                                                                    : "Suspended";
        write(STDOUT_FILENO, "PID      Command     STATUS\n", 29);
        write(STDOUT_FILENO, pid_str, strlen(pid_str));
        write(STDOUT_FILENO, "      ", 7);
        write(STDOUT_FILENO, curr->cmd->arguments[0], strlen(curr->cmd->arguments[0]));
        write(STDOUT_FILENO, "      ", 7);
        write(STDOUT_FILENO, stat, strlen(stat));
        write(STDOUT_FILENO, "\n", 1);
        curr = curr->next;
    }
}

void execute(cmdLine *pCmdLine, int debug, process **process_list)
{
    int returnVal;
    char *pid_str;
    int err = 0;
    pid_t pid = 0;
    int status;
    if (strncmp(pCmdLine->arguments[0], "cd", 2) == 0)
    {
        if (strncmp(pCmdLine->arguments[1], "..", 2) == 0)
        {
            err = chdir("..");
        }
        else
        {
            err = chdir(pCmdLine->arguments[1]);
        }
        if (err < 0)
        {
            write(STDERR_FILENO, "no such direcetory\n", 20);       
        }
    }

    else if (strncmp(pCmdLine->arguments[0], "procs", 5) == 0)
    {

        printProcessList(process_list);
    }
    else if ((pid = fork()) == 0)
    {
        if ((returnVal = execv(pCmdLine->arguments[0], pCmdLine->arguments)) < 0)
        {
            perror("couln't execute");
            _exit(EXECUTION_FAILED);
        }
    }

    if (debug == 1)
    {
        write(STDERR_FILENO, "pid is: ", 9);
        pid_str = itoa(pid);
        write(STDERR_FILENO, pid_str, strlen(pid_str));
        write(STDERR_FILENO, "\n", 1);
        write(STDERR_FILENO, "Executing command is: ", 23);
        write(STDERR_FILENO, pCmdLine->arguments[0], strlen(pCmdLine->arguments[0]));
        write(STDERR_FILENO, "\n", 1);
    }
    if (pid != 0)
    {
        addProcess(process_list, pCmdLine, pid);
    }
    if (pCmdLine->blocking == 1)
    {
        waitpid(pid, &status, WEXITED);
    }
}

int main(int argc, char **argv)
{
    char buf[PATH_MAX];
    char input[INPUT_MAX_SIZE];
    cmdLine *cmdL;
    int i;
    int debug = 0;
    process *process_list = NULL;

    for (i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-d", 2) == 0)
            debug = 1;
    }

    while (1)
    {
        char *returnVal = getcwd(buf, PATH_MAX);
        if (returnVal == NULL)
        {
            return 1;
        }

        fprintf(stdout, "%s\n", returnVal);

        fgets(input, INPUT_MAX_SIZE, stdin);

        if (strncmp(input, "quit", 4) == 0)
        {
            break;
        }

        if ((cmdL = parseCmdLines(input)) == NULL)
        {
            fprintf(stdout, "%s", "nothing to parse\n");
            freeCmdLines(cmdL);

            return 0;
        }
        execute(cmdL, debug, &(process_list));
    }
    return 0;
}