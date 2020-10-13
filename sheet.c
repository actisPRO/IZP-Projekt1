#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define STR_MAX_LEN 81920 / CHAR_BIT

#define MODE_NONE -1
#define MODE_EDIT_TABLE 0
#define MODE_EDIT_DATA 1

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
char* tc_2arg[] = {
        "drows",
        "dcols"
};

// 0 - no args, 1 - 1 arg, 2 - 2 args
int commandType(const char* name)
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

    return 0;
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
    // modes are edit table and edit data.
    int mode = MODE_NONE;
    char commands[255][100] = { 0 }; // sequence of commands
    int i_commands = 0; // index in commands

    int aRows = 0; // amount of rows added with arow
    int deletedRows[1024] = {0};
    int i_deletedRows = 0;

    for (int i = 1; i < argc; ++i)
    {
        // parse delimiters
        if (strcmp(argv[i], "-d") == 0)
        {
            if (i + 1 >= argc)
            { // index out of range (e.g. delimiter wasn't specified)
                printf("ERROR: delimiter was not specified in -d argument!\n");
                return EXIT_FAILURE;
            }
            delims = argv[i + 1];
        }

        // commands for editing table
        // commands with 2 args
        for (int j = 0; j < (int)(sizeof(tc_2arg) / sizeof(tc_2arg[0])); ++j)
        {
            if (strcmp(argv[i], tc_2arg[j]) == 0)
            {
                if (i + 1 >= argc || i + 2 >= argc)
                {
                    printf("ERROR: incorrect amount of arguments for the command %s\n", argv[i]);
                    return EXIT_FAILURE;
                }

                // we control if arguments for the command are correct numbers
                char* eptr;
                long arg0 = strtol(argv[i + 1], &eptr, 10);
                if (arg0 == 0)
                {
                    printf("ERROR: incorrect argument #1: %s for the command %s\n", argv[i + 1], argv[i]);
                    return EXIT_FAILURE;
                }
                long arg1 = strtol(argv[i + 2], &eptr, 10);
                if (arg1 == 0)
                {
                    printf("ERROR: incorrect argument #2: %s for the command %s\n", argv[i + 2], argv[i]);
                    return EXIT_FAILURE;
                }

                if (arg1 < arg0)
                {
                    printf("ERROR: argument #1 can't be bigger then argument #2\n");
                    return EXIT_FAILURE;
                }


                // add command to the sequence
                sprintf(commands[i_commands], "%s %s %s", argv[i], argv[i + 1], argv[i + 2]);
                ++i_commands;
                mode = MODE_EDIT_TABLE;
            }
        }

        // commands with 1 arg
        for (int j = 0; j < (int)(sizeof(tc_1arg) / sizeof(tc_1arg[0])); ++j)
        {
            if (strcmp(argv[i], tc_1arg[j]) == 0)
            {
                if (i + 1 >= argc)
                {
                    printf("ERROR: incorrect amount of arguments for the command '%s'\n", argv[i]);
                    return EXIT_FAILURE;
                }

                // we control if arguments for the command are correct numbers
                char* eptr;
                long arg0 = strtol(argv[i + 1], &eptr, 10);
                if (arg0 == 0)
                {
                    printf("ERROR: incorrect argument #1: %s for the command '%s'\n", argv[i + 1], argv[i]);
                    return EXIT_FAILURE;
                }

                // add command to the sequence
                sprintf(commands[i_commands], "%s %s", argv[i], argv[i + 1]);
                ++i_commands;
                mode = MODE_EDIT_TABLE;
            }
        }

        // commands without args
        for (int j = 0; j < (int)(sizeof(tc_noargs) / sizeof(tc_noargs[0])); ++j)
        {
            if (strcmp(argv[i], tc_noargs[j]) == 0)
            {
                // add command to the sequence
                sprintf(commands[i_commands], "%s", argv[i]);
                ++i_commands;
                mode = MODE_EDIT_TABLE;

                if (strcmp(argv[i], "arow") == 0) ++aRows;
            }
        }

    } // todo: use while cycle, check args

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
            if (commandType(nextCommandName) >= 1)
            {
                arg0 = strtol(strtok(NULL, " "), &eptr, 10);
            }
            if (commandType(nextCommandName) >= 2)
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
            if (commandType(nextCommandName) >= 1)
            {
                arg0 = strtol(strtok(NULL, " "), &eptr, 10);
            }
            if (commandType(nextCommandName) >= 2)
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
    } // todo check if final col is empty, control row length

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