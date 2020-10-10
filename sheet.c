#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define STR_MAX_LEN 81920 / CHAR_BIT

#define MODE_NONE -1
#define MODE_EDIT_TABLE 0
#define MODE_EDIT_DATA 1

char *delims = " ";

// array of table change commands with no args
const char *tc_noargs[] = {
        "acol",
        "arow"
};

// array of table change commands with 1 arg
const char *tc_1arg[] = {
        "irow",
        "drow",
        "icol",
        "dcol"
};

// array of table change commands with 2 args
char *tc_2arg[] = {
        "drows",
        "dcols"
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("ERROR: no arguments were specified");
        return EXIT_FAILURE;
    }

    // parsing arguments
    // modes are edit table and edit data.
    int mode = MODE_NONE;
    char commands[255] = {0}; // sequence of commands (separated with ;)

    for (int i = 1; i < argc; ++i) {
        // parse delimiters
        if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 >= argc) { // index out of range (e.g. delimiter wasn't specified)
                printf("ERROR: delimiter was not specified in -d argument!");
                return EXIT_FAILURE;
            }
            delims = argv[i + 1];
        }

        // commands for editing table
        // commands with 2 args
        for (int j = 0; j < (int) (sizeof(tc_2arg) / sizeof(tc_2arg[0])); ++j) {
            if (strcmp(argv[i], tc_2arg[j]) == 0) {
                if (i + 1 >= argc || i + 2 >= argc) {
                    printf("ERROR: incorrect amount of arguments for the command %s", argv[i]);
                    return EXIT_FAILURE;
                }

                // we control if arguments for the command are correct numbers
                char *eptr;
                long arg0 = strtol(argv[i + 1], &eptr, 10);
                if (arg0 == 0) {
                    printf("ERROR: incorrect argument #1: %s for the command %s", argv[i + 1], argv[i]);
                    return EXIT_FAILURE;
                }
                long arg1 = strtol(argv[i + 2], &eptr, 10);
                if (arg1 == 0) {
                    printf("ERROR: incorrect argument #2: %s for the command %s", argv[i + 2], argv[i]);
                    return EXIT_FAILURE;
                }

                // add command to the sequence
                sprintf(commands, "%s%s %s %s;", commands, argv[i], argv[i + 1], argv[i + 2]);
                mode = MODE_EDIT_TABLE;
            }
        }

        // commands with 1 arg
        for (int j = 0; j < (int) (sizeof(tc_1arg) / sizeof(tc_1arg[0])); ++j) {
            if (strcmp(argv[i], tc_1arg[j]) == 0) {
                if (i + 1 >= argc) {
                    printf("ERROR: incorrect amount of arguments for the command '%s'", argv[i]);
                    return EXIT_FAILURE;
                }

                // we control if arguments for the command are correct numbers
                char *eptr;
                long arg0 = strtol(argv[i + 1], &eptr, 10);
                if (arg0 == 0) {
                    printf("ERROR: incorrect argument #1: %s for the command '%s'", argv[i + 1], argv[i]);
                    return EXIT_FAILURE;
                }

                // add command to the sequence
                sprintf(commands, "%s%s %s;", commands, argv[i], argv[i + 1]);
                mode = MODE_EDIT_TABLE;
            }
        }

        // commands without args
        for (int j = 0; j < (int) (sizeof(tc_noargs) / sizeof(tc_noargs[0])); ++j) {
            if (strcmp(argv[i], tc_noargs[j]) == 0) {
                // add command to the sequence
                sprintf(commands, "%s%s;", commands, argv[i]);
                mode = MODE_EDIT_TABLE;
            }
        }

    } // todo: use while cycle

    if (mode == MODE_NONE) {
        printf("ERROR: no commands were specified");
        return EXIT_FAILURE;
    }

    int currentLine = 1;
    int columnsAmount = 0;
    int normalColumnsAmount = 0; // amount of columns in the first row

    char inputBuffer[STR_MAX_LEN];
    char outputBuffer[STR_MAX_LEN] = {0};

    //we'll calculate amount of columns (according to the first line)
    if (!fgets(inputBuffer, sizeof(inputBuffer), stdin)) {
        printf("ERROR: input was empty!");
        return EXIT_FAILURE;
    }
    else {
        char *token = strtok(inputBuffer, delims);
        while (token != NULL) {
            ++normalColumnsAmount;
            token = strtok(NULL, delims);
        }
    }

    // we'll perform commands on the first line
    char *commandsToPerform = commands;
    char *savePtr_NC;
    char *nextCommand = strtok_r(commandsToPerform, ";", &savePtr_NC);
    while (nextCommand != NULL) {
        // read args
        char *savePtr_NA;
        int arg0, arg1;
        strtok_r(nextCommand, " ", &savePtr_NA);
        arg0 = atoi(strtok_r(NULL, " ", &savePtr_NA));
        arg1 = atoi(strtok_r(NULL, " ", &savePtr_NA));

        nextCommand = strtok_r(NULL, ";", &savePtr_NC);
    }

    // magic is here
    while (fgets(inputBuffer, sizeof(inputBuffer), stdin)) {
        printf("%s", inputBuffer);
    }

    return 0;
}