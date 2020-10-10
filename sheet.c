#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char* delims = " ";

    if (argc == 1) {
        printf("ERROR: no arguments specified");
        return EXIT_FAILURE;
    }

    // parse arguments
    for (int i = 1; i < argc; ++i) {
        // parse delimiters
        if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 >= argc) { // index out of range (eg. delimiter unspecified)
                printf("ERROR: delimiter was not specified in -d argument!");
                return EXIT_FAILURE;
            }

            delims = argv[i + 1];
        }
    }

    printf("%s", delims);

    return 0;
}
