#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define STR_MAX_LEN 81920 / CHAR_BIT

#define MODE_NONE -1
#define MODE_EDIT_TABLE 0
#define MODE_EDIT_DATA 1

// selection modes
#define SELECTION_TABLE 0
#define SELECTION_ROWS 1
#define SELECTION_BEGINSWITH 2
#define SELECTION_CONTAINS 3

char* delims = " ";

// array of table change commands with no args
const char* tc_noargs[] = {
        "acol",
        "arow"
};

// array of table change commands with 1 arg
const char* tc_1arg[] = {
        "irow",
        "drow",
        "icol",
        "dcol"
};

// array of table change commands with 2 args
const char* tc_2arg[] = {
        "drows",
        "dcols"
};

// array of data edit commands with 1 arg
const char* de_1arg[] = {
        "tolower",
        "toupper",
        "round",
        "int"
};

// array of data edit commands with 2 args
const char* de_2arg[] = {
        "cset",
        "copy",
        "swap",
        "move"
};

// 0 - table edit command, 1 - selection command, 2 - content edit command, -1 - is not a command
int commandType(const char* name)
{
    for (int i = 0; i < (int)(sizeof(tc_2arg) / sizeof(tc_2arg[0])); ++i)
    {
        if (strcmp(name, tc_2arg[i]) == 0)
        {
            return 0;
        }
    }
    for (int i = 0; i < (int)(sizeof(tc_1arg) / sizeof(tc_1arg[0])); ++i)
    {
        if (strcmp(name, tc_1arg[i]) == 0)
        {
            return 0;
        }
    }
    for (int i = 0; i < (int)(sizeof(tc_noargs) / sizeof(tc_noargs[0])); ++i)
    {
        if (strcmp(name, tc_noargs[i]) == 0)
        {
            return 0;
        }
    }

    for (int i = 0; i < (int)(sizeof(de_1arg) / sizeof(de_1arg[0])); ++i)
    {
        if (strcmp(name, de_1arg[i]) == 0)
        {
            return 2;
        }
    }
    for (int i = 0; i < (int)(sizeof(de_2arg) / sizeof(de_2arg[0])); ++i)
    {
        if (strcmp(name, de_2arg[i]) == 0)
        {
            return 2;
        }
    }

    if (strcmp(name, "rows") == 0 || strcmp(name, "beginswith") == 0 || strcmp(name, "contains") == 0)
    {
        return 1;
    }

    return -1;
}

// 0 - no args, 1 - 1 arg, 2 - 2 args, -1 - is not a command
int argCount(const char* name)
{
    for (int i = 0; i < (int)(sizeof(tc_2arg) / sizeof(tc_2arg[0])); ++i)
    {
        if (strcmp(name, tc_2arg[i]) == 0)
        {
            return 2;
        }
    }
    for (int i = 0; i < (int)(sizeof(tc_1arg) / sizeof(tc_1arg[0])); ++i)
    {
        if (strcmp(name, tc_1arg[i]) == 0)
        {
            return 1;
        }
    }
    for (int i = 0; i < (int)(sizeof(tc_noargs) / sizeof(tc_noargs[0])); ++i)
    {
        if (strcmp(name, tc_noargs[i]) == 0)
        {
            return 0;
        }
    }

    for (int i = 0; i < (int)(sizeof(de_1arg) / sizeof(de_1arg[0])); ++i)
    {
        if (strcmp(name, de_1arg[i]) == 0)
        {
            return 1;
        }
    }
    for (int i = 0; i < (int)(sizeof(de_2arg) / sizeof(de_2arg[0])); ++i)
    {
        if (strcmp(name, de_2arg[i]) == 0)
        {
            return 2;
        }
    }

    if (strcmp(name, "rows") == 0 || strcmp(name, "beginswith") == 0 || strcmp(name, "contains") == 0)
    {
        return 2;
    }

    return -1;
}

int isDelim(char symbol)
{
    for (int i = 0; i < (int)strlen(delims); ++i)
    {
        if (symbol == delims[i])
        {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        printf("ERROR: no arguments were specified");
        return EXIT_FAILURE;
    }

    // parsing arguments
    int mode = MODE_NONE;
    // variables for MODE_EDIT_TABLE
    char commands[255][100] = { 0 }; // sequence of commands
    int i_commands = 0; // index in commands

    int aRows = 0; // amount of rows added with arow
    int deletedRows[1024] = {0};
    int i_deletedRows = 0;

    // variables for MODE_EDIT_DATA
    // selection variables
    int selectionMode = SELECTION_TABLE;
    int sRowsStart = 0;
    int sRowsEnd = INT_MAX;
    int colI = 0;
    char str[101] = { 0 };

    int arg = 1;
    while (arg < argc)
    {
        // parse delimiters
        if (strcmp(argv[arg], "-d") == 0)
        {
            if (arg + 1 >= argc)
            { // index out of range (e.g. delimiter wasn't specified)
                printf("ERROR: delimiter was not specified in -d argument!\n");
                return EXIT_FAILURE;
            }
            delims = argv[arg + 1];
            arg += 2;
        }
        else
        {
            int currArgC = argCount(argv[arg]);
            if (currArgC == -1)
            {
                printf("ERROR: error in argument #%d: '%s' is not a command or a valid command argument", arg, argv[arg]);
                return EXIT_FAILURE;
            }
            else
            {
                int currCommandT = commandType(argv[arg]);
                if (currCommandT == 0)
                {
                    if (mode == MODE_EDIT_DATA)
                    {
                        printf("ERROR: please run commands for editing table separately from commands for editing data\n");
                        return EXIT_FAILURE;
                    }

                    mode = MODE_EDIT_TABLE;

                    if (currArgC == 0)
                    {
                        sprintf(commands[i_commands], "%s", argv[arg]);
                        ++i_commands;
                        if (strcmp(argv[arg], "arow") == 0) ++aRows;

                        arg += 1;
                    }
                    else if (currArgC == 1)
                    {
                        if (arg + 1 >= argc)
                        {
                            printf("ERROR: incorrect amount of arguments for the command '%s'\n", argv[arg]);
                            return EXIT_FAILURE;
                        }

                        // we control if arguments for the command are correct numbers
                        char* eptr;
                        long arg0 = strtol(argv[arg + 1], &eptr, 10);
                        if (arg0 == 0)
                        {
                            printf("ERROR: incorrect argument #1: %s for the command '%s'\n", argv[arg + 1], argv[arg]);
                            return EXIT_FAILURE;
                        }

                        // add command to the sequence
                        sprintf(commands[i_commands], "%s %s", argv[arg], argv[arg + 1]);
                        ++i_commands;

                        arg += 2;
                    }
                    else if (currArgC == 2)
                    {
                        if (arg + 2 >= argc)
                        {
                            printf("ERROR: incorrect amount of arguments for the command %s\n", argv[arg]);
                            return EXIT_FAILURE;
                        }

                        // we control if arguments for the command are correct numbers
                        char* eptr;
                        long arg0 = strtol(argv[arg + 1], &eptr, 10);
                        if (arg0 == 0)
                        {
                            printf("ERROR: incorrect argument #1: %s for the command %s\n", argv[arg + 1], argv[arg]);
                            return EXIT_FAILURE;
                        }
                        long arg1 = strtol(argv[arg + 2], &eptr, 10);
                        if (arg1 == 0)
                        {
                            printf("ERROR: incorrect argument #2: %s for the command %s\n", argv[arg + 2], argv[arg]);
                            return EXIT_FAILURE;
                        }

                        if (arg1 < arg0)
                        {
                            printf("ERROR: argument #1 can't be bigger then argument #2\n");
                            return EXIT_FAILURE;
                        }

                        // add command to the sequence
                        sprintf(commands[i_commands], "%s %s %s", argv[arg], argv[arg + 1], argv[arg + 2]);
                        ++i_commands;

                        arg += 3;
                    }
                }
                else if (currCommandT == 1)
                {
                    if (mode == MODE_EDIT_TABLE)
                    {
                        printf("ERROR: please run commands for editing table separately from commands for editing data\n");
                        return EXIT_FAILURE;
                    }

                    mode = MODE_EDIT_DATA;

                    if (strcmp(argv[arg], "rows") == 0)
                    {
                        if (arg + 2 >= argc)
                        {
                            printf("ERROR: incorrect amount of arguments for the command %s\n", argv[arg]);
                            return EXIT_FAILURE;
                        }

                        sRowsStart = atoi(argv[arg + 1]);
                        if (sRowsStart == 0)
                        {
                            printf("ERROR: incorrect argument #1: %s for the command %s\n", argv[arg + 1], argv[arg]);
                            return EXIT_FAILURE;
                        }

                        if (strcmp(argv[arg + 2], "-") != 0)
                        {
                            sRowsEnd = atoi(argv[arg + 2]);
                            if (sRowsEnd == 0)
                            {
                                printf("ERROR: incorrect argument #2: %s for the command %s\n", argv[arg + 1], argv[arg]);
                                return EXIT_FAILURE;
                            }
                        }

                        selectionMode = SELECTION_ROWS;

                        arg += 3;
                    }
                    else
                    {
                        if (strcmp(argv[arg], "beginswith") == 0)
                        {
                            selectionMode = SELECTION_BEGINSWITH;
                        }
                        else if (strcmp(argv[arg], "contains") == 0)
                        {
                            selectionMode = SELECTION_CONTAINS;
                        }

                        if (arg + 2 >= argc)
                        {
                            printf("ERROR: incorrect amount of arguments for the command %s\n", argv[arg]);
                            return EXIT_FAILURE;
                        }

                        colI = atoi(argv[arg + 1]);
                        if (colI == 0)
                        {
                            printf("ERROR: incorrect argument #1: %s for the command %s\n", argv[arg + 1], argv[arg]);
                            return EXIT_FAILURE;
                        }

                        // reading STR
                        int isCommand = commandType(argv[arg + 2]);
                        if (isCommand != -1)
                        {
                            printf("ERROR: incorrect argument #2: unexpected command %s after the command %s\n", argv[arg + 2], argv[arg]);
                            return EXIT_FAILURE;
                        }
                        int pos = 0;
                        while (strlen(str) < 100)
                        {
                            if ((strlen(str) + strlen(argv[arg + 2 + pos]) > 100)) break;

                            sprintf(str, "%s%s", str, argv[arg + 2 + pos]);
                            ++pos;

                            if (arg + 2 + pos >= argc) break;
                            isCommand = commandType(argv[arg + 2 + pos]);
                            if (isCommand != -1) break;

                            sprintf(str, "%s ", str);
                        }

                        arg += 2 + pos;
                    }
                }
                else if (currCommandT == 2)
                {
                    if (mode == MODE_EDIT_TABLE)
                    {
                        printf("ERROR: please run commands for editing table separately from commands for editing data\n");
                        return EXIT_FAILURE;
                    }

                    mode = MODE_EDIT_DATA;
                }
            }
        }
    }

    if (mode == MODE_NONE)
    {
        printf("ERROR: no commands were specified\n");
        return EXIT_FAILURE;
    }

    char input[STR_MAX_LEN] = { 0 };

    int currInputRow = 1; // current row based on input
    int realRow = 1; // real row index

    int renderColumns = 0; // amount of columns to render
    int originalCols = 0;
    int colsWithAcol = 0; //amount of columns before modifications, but with acols

    // start parsing rows
    while (fgets(input, STR_MAX_LEN, stdin))
    {
        int currColumn = 0;
        char columns[105][100] = { 0 };
        int i_column = 0;

        for (int i = 0; i < (int)strlen(input); ++i)
        {
            if (i == 0 && isDelim(input[i]))
            {
                ++currColumn;
                continue;
            }

            if (currInputRow > 1 && currColumn
                    >= originalCols) // if some row has more columns then the first row, we'll set values of all extra columns to \0
            {
                break;
            }

            if (!isDelim(input[i]) && input[i] != '\n' && input[i] != EOF)
            {
                columns[currColumn][i_column] = input[i];
                ++i_column;
            }
            else
            {
                ++currColumn;
                i_column = 0;
            }
        }

        int columnCount = currColumn;
        if (currInputRow == 1)
        {
            renderColumns = columnCount;
            originalCols = columnCount;
            colsWithAcol = columnCount;
        }

        // now run column commands
        for (int ci = 0; ci < i_commands; ++ci)
        {
            char nextCommand[100];
            strcpy(nextCommand, commands[ci]);

            int arg0, arg1;
            char* nextCommandName = strtok(nextCommand, " ");
            char* eptr;
            // get args count to prevent segmentation fault
            if (argCount(nextCommandName) >= 1)
            {
                arg0 = strtol(strtok(NULL, " "), &eptr, 10);
            }
            if (argCount(nextCommandName) >= 2)
            {
                arg1 = strtol(strtok(NULL, " "), &eptr, 10);
            }

            // run command
            if (strcmp(nextCommandName, "icol") == 0)
            {
                if (arg0 <= renderColumns)
                { // argument > amount of columns => ignore;
                    if (currInputRow == 1) ++renderColumns;
                    // copy content from each column to the next one. arg0 - 1 because array of columns starts with 0, but table's indexes start with 1
                    for (int i = renderColumns; i > arg0 - 1; --i)
                    {
                        strcpy(columns[i], columns[i - 1]);
                    }

                    strcpy(columns[arg0 - 1], ""); // new empty column
                }
            }
            else if (strcmp(nextCommandName, "dcol") == 0)
            {
                if (arg0 <= colsWithAcol)
                {
                    // copy content
                    for (int i = arg0 - 1; i <= colsWithAcol; ++i)
                    {
                        strcpy(columns[i], columns[i + 1]);
                    }

                    // decrease table length
                    if (currInputRow == 1) --renderColumns;
                }
            }
            else if (strcmp(nextCommandName, "dcols") == 0)
            {
                if (arg0 <= colsWithAcol)
                {
                    if (arg1 > colsWithAcol) arg1 = colsWithAcol;
                    int removedCols = arg1 - arg0 + 1;

                    for (int i = arg0 - 1; i <= columnCount; ++i)
                    {
                        strcpy(columns[i], columns[i + removedCols]);
                    }

                    if (currInputRow == 1) renderColumns -= removedCols;
                }
            }
            else if (strcmp(nextCommandName, "acol") == 0)
            {
                if (currInputRow == 1)
                {
                    ++colsWithAcol;
                    ++renderColumns;
                }
            }
        }

        // row commands
        int renderThis = 1;
        int emptyBeforeThis = 0;
        for (int ci = 0; ci < i_commands; ++ci)
        {
            char nextCommand[100];
            strcpy(nextCommand, commands[ci]);

            int arg0, arg1;
            char* nextCommandName = strtok(nextCommand, " ");
            char* eptr;
            // get args count to prevent segmentation fault
            if (argCount(nextCommandName) >= 1)
            {
                arg0 = strtol(strtok(NULL, " "), &eptr, 10);
            }
            if (argCount(nextCommandName) >= 2)
            {
                arg1 = strtol(strtok(NULL, " "), &eptr, 10);
            }

            if (strcmp(nextCommandName, "irow") == 0)
            {
                if (arg0 >= realRow - emptyBeforeThis && arg0 <= realRow)
                {
                    ++emptyBeforeThis;
                    ++realRow;
                }
            }
            else if (strcmp(nextCommandName, "drow") == 0)
            {
                if (arg0 >= realRow - emptyBeforeThis && arg0 < realRow) // deleting rows added with irow
                {
                    --emptyBeforeThis;
                    --realRow;
                }
                else if (arg0 == realRow)
                {
                    renderThis = 0;
                }
            }
            else if (strcmp(nextCommandName, "drows") == 0)
            {
                for (int i = arg0; i <= arg1; ++i)
                {
                    if (i >= realRow - emptyBeforeThis && i < realRow) // deleting rows added with irow
                    {
                        --emptyBeforeThis;
                        --realRow;
                    }
                    else if (i == realRow)
                    {
                        renderThis = 0;
                    }
                }
            }
        }

        // rendering
        for (int i = 1; i <= emptyBeforeThis; ++i)
        {
            for (int j = 0; j < renderColumns - 1; ++j)
            {
                printf("%c", delims[0]);
            }
            printf("\n");
        }
        if (renderThis)
        {
            for (int i = 0; i < renderColumns; ++i)
            {
                if (i == renderColumns - 1) printf("%s", columns[i]);
                else printf("%s%c", columns[i], delims[0]);
            }
            printf("\n");
        }
        ++realRow;
        ++currInputRow;
    }

    // render arows
    for (int i = 1; i <= aRows; ++i)
    {
        // check if it isn't deleted
        for (int j = 0; j < renderColumns; ++j)
        {
            printf("%c", delims[0]);
        }
        printf("\n");
    }

    return 0;
}