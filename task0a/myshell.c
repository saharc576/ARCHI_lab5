#include "linux/limits.h"
#include <unistd.h>
#include <stdio.h>
#include "LineParser.h"
#include <string.h>

#define INPUT_MAX_SIZE 48


void execute(cmdLine *pCmdLine) {
    int returnVal;
    if ((returnVal = execv(pCmdLine->inputRedirect, pCmdLine->arguments)) < 0 ) {
        perror("couln't execute");
        exit(1);
    }
}

int main (int argc, char **agrv) {
    char buf[PATH_MAX];
    char input[INPUT_MAX_SIZE];
    cmdLine *cmdL;

    while (1) {
        char *returnVal = getcwd(buf, PATH_MAX);
        if (returnVal == NULL) {
            return 1;
        }


        fprintf(stdout, "%s\n", returnVal);

        fgets(input, INPUT_MAX_SIZE,stdin);
        sscanf(input, "%s", input);

        if(strncmp(input, "q", 1) == 0) {
            freeCmdLines(cmdL);
            break;
        }

        if ((cmdL = parseCmdLines(input)) == NULL) {
            fprintf(stdout, "%s", "nothing to parse\n");
            return 0;
        }
        execute(cmdL);
        freeCmdLines(cmdL);
    }
    return 0;   
}