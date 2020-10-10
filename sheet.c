#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define STR_MAX_LEN 81920 / CHAR_BIT

char *delims = " ";

// array of table change commands with no args
const char tc_noargs[3][4] = {
        "irow",
        "acol"
};

// array of table change commands with 1 arg
const char tc_1arg[3][4] = {
        "drow",
        "icol",
        "dcol"
};

// array of table change commands with 2 args
const char tc_2arg[2][5] = {
        "drows",
        "dcols"
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("ERROR: no arguments were specified");
        return EXIT_FAILURE;
    }

    char inputBuffer[STR_MAX_LEN];

    // parse arguments
    for (int i = 1; i < argc; ++i) {
        // parse delimiters
        if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 >= argc) { // index out of range (e.g. delimiter wasn't specified)
                printf("ERROR: delimiter was not specified in -d argument!");
                return EXIT_FAILURE;
            }
            delims = argv[i + 1];
        }

        // arguments for changing table

    }

    int lastLineNumber = 0;
    while (fgets(inputBuffer, sizeof(inputBuffer), stdin)) {
        printf("%s", inputBuffer);

    }

    return 0;
}
