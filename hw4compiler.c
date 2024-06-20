/*
  Name: Anthony Bonus & Adam Disanti
  Purpose: Recursive Descent Parser
    and Intermediate Code Generator  HW#4
  Date: 4/3/2024
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// Max size for arrays
#define MAX_SIZE 1000

// token struct
typedef struct token
{
    // int tokenTypeSym;
    char *tokenName;
    int tokenValue;
    int errorHandle;
} token;

// symbol struct
typedef struct symbol
{
    int kind;      // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    int mark;      // to indicate unavailable or deleted
} symbol;
// constants: kind, name, value
// variables: kinda name, L, M
// procedures: kind, name, L, M

typedef struct instruction
{
    int op;   // Operation code
    int l;    // Lexicographical level
    int m;    // Modifier
    int line; // line HW4
} instruction;

// enum stores the token value for each symbol
typedef enum
{
    oddsym = 1,
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    fisym,
    eqsym,
    neqsym,
    lessym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym
} token_type;

// Function Prototypes
int isReservedWord(char *word);
int isSpecialSymbol(char curChar);
token_type returnTokenTypeForReserved(char *word);
token_type returnTokenTypeForSymbol(char curChar);
token_type returnTokenTypeForIdentifierOrNumber(char *curCharSet);

// HW3 functions
void emit(int op, int l, int m);
void printCode(int op, int l, int m, int i);
void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void condition();
void expression();
void term();
void factor();
int symbolTableCheck(char *name);
void printCleanList();
void updateMark();
void printTokenList();
void printHw1Input();
void printSymbolTable();
void printGeneratedCode();
void procedureDeclaration();

// HW4 FUNCTIONS
int symbolTablePosition(char *identifier);
int symbolTableCheckWithLevel(char *string);
void symbolInitialize();

// Global Vars

//  Reserved Words Array
int reservedWordsCount = 15;

// procedure and else should be ignored in this program
char *reservedWords[] = {"const", "var", "procedure", "call", "begin", "end", "if", "fi", "then", "$else", "while", "do", "read", "write", "odd"};

// Special Symbols Array
int specialSymbolsCount = 13;
char specialSymbols[] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'};

// Token Array
int tokenCount = 0;
int tokenIndex = 0;
token *tokens[MAX_SIZE];

// Cleaned up version of token list
int tokenCountOff = 0;
int tokenIndexOff = 0;
token tokensOff[MAX_SIZE];

// Symbol Table
int symbolCount = 0;
int symbolIndex = 0;
symbol symbolTable[MAX_SIZE];

// hw4
int prevSymbolIndex = 0;
int currentProcedureIndex = 0;
int curLevel = -1; // Lexicographical level
int procedureAddress = 0;

// Store generated code
int codeCount = 0;
int currentCodeIndex = 0;
int nextCodeAddr = 0;
instruction code[MAX_SIZE];

int main(int argc, char **inputs)
{
    // variables
    int maxCharLength = 11;
    int maxNumLength = 5;
    char currentChar;

    // Build word that but also resets appropiately
    char currentString[MAX_SIZE];

    // File I/O to open the whole file and print sourse code
    FILE *inputFile = fopen(inputs[1], "r");
    if (inputFile == NULL)
    {
        printf("Error opening input file.\n");
        return 1;
    }

    // Print the input file

    printf("\nSource Program:\n\n");
    while ((currentChar = fgetc(inputFile)) != EOF)
    {
        printf("%c", currentChar);
    }
    printf("\n-------------------\n");

    // Start print for lexeme table
    /*
    printf("\n Lexeme Table:\n\n");
    printf("lexeme \t\ttoken type\n");
    */

    // File I/O to read file again line by line
    FILE *inputFile2 = fopen(inputs[1], "r");
    if (inputFile2 == NULL)
    {
        printf("Error opening input file.\n");
        return 1;
    }

    // Initialize string builder index
    int currentStringBuildIndex = 0;

    // for special cases to look ahead
    char nextChar;

    // USE THIS FOR NEXT CHAR
    FILE *inputFile3 = fopen(inputs[1], "r");
    long int position2;
    int inComment = 0;
    while ((currentChar = fgetc(inputFile2)) != EOF)
    {

        // make inputfile3 point to inputfile 2
        long int position = ftell(inputFile2);
        // fseek(inputFile2, 0, SEEK_SET);
        fseek(inputFile3, position, SEEK_SET);

        // Reset the word builder
        currentStringBuildIndex = 0;
        strcpy(currentString, "");

        // immediately handle comments
        if (currentChar == '/' && (nextChar = fgetc(inputFile3)) == '*')
        {
            // Start of a comment
            inComment = 1;

            // MAKE FILE 2 POINT TO 3
            position2 = ftell(inputFile3);
            // move inputfile2 to poisiton inputFIle3
            fseek(inputFile2, position2, SEEK_SET);
            continue;
        }
        else if (currentChar == '*' && (nextChar = fgetc(inputFile3)) == '/')
        {
            // End of a comment
            inComment = 0;

            // MAKE FILE 2 POINT TO 3
            position2 = ftell(inputFile3);
            // move inputfile2 to poisiton inputFIle3
            fseek(inputFile2, position2, SEEK_SET);
            continue;
        }

        // Not inside a comment, process the character
        else if (inComment == 1)
        {
            continue;
        }

        if (isdigit(currentChar))
        {
            // add first char to build string
            currentString[currentStringBuildIndex] = currentChar;
            currentStringBuildIndex++;

            // continue reading NUMS until the end of the NUM
            while (((nextChar = fgetc(inputFile3)) != EOF) && (isdigit(nextChar)))
            {

                // MAKE FILE 2 POINT TO 3
                position2 = ftell(inputFile3);
                // move inputfile2 to poisiton inputFIle3
                fseek(inputFile2, position2, SEEK_SET);

                // Build string
                currentString[currentStringBuildIndex] = nextChar;
                currentStringBuildIndex++;
            }

            // complete the string with a null operator
            currentString[currentStringBuildIndex] = '\0';

            // create token
            tokens[tokenCount] = (token *)malloc(sizeof(token));

            // Initialize errorHandle and comment flag
            tokens[tokenCount]->errorHandle = 0;

            // Use function to grab token value
            tokens[tokenCount]->tokenValue = 3;

            // Store the input number
            // first malloc the strcpyf
            tokens[tokenCount]->tokenName = (char *)malloc(sizeof(char) * (strlen(currentString)) + 1);

            strcpy(tokens[tokenCount]->tokenName, currentString);

            // Set errorHandle flag to 1
            if (currentStringBuildIndex > maxNumLength)
            {
                tokens[tokenCount]->errorHandle = 1;
                tokens[tokenCount]->tokenValue = -1;
            }

            // Print lexeme table and account for errors
            if (tokens[tokenCount]->errorHandle == 0)
            {
                // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);
            }

            if (tokens[tokenCount]->errorHandle == 1)
            {
                printf("Error: Number is too long %s\n", currentString);
                exit(1);
            }

            // increment the token count
            tokenCount++;

            continue;
        }

        // If current character is a letter
        else if (isalpha(currentChar))
        {
            // add first char to build string
            currentString[currentStringBuildIndex++] = currentChar;

            // continue reading characters until the end of the word
            while ((currentChar = fgetc(inputFile3)) != EOF && (isalpha(currentChar) || isdigit(currentChar)))
            {

                // MAKE FILE 2 POINT TO 3
                position2 = ftell(inputFile3);
                // move inputfile2 to poisiton inputFIle3
                fseek(inputFile2, position2, SEEK_SET);

                //  build the current word
                currentString[currentStringBuildIndex++] = currentChar;
            }

            // complete the string with a null operator
            currentString[currentStringBuildIndex] = '\0';

            // check if the word is a reserved word
            if (isReservedWord(currentString))
            {
                // store the right token of reservedWord using the enum
                tokens[tokenCount] = (token *)malloc(sizeof(token));

                // Initialize errorHandle
                tokens[tokenCount]->errorHandle = 0;

                // Use function to grab token value
                tokens[tokenCount]->tokenValue = returnTokenTypeForReserved(currentString);

                // Print lexeme table
                if (tokens[tokenCount]->errorHandle == 0)
                {
                    // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);
                }

                // increment the token count
                tokenCount++;

                continue;
            }

            // It's a normal identifier
            else
            {
                // store the right token of identifier using the enum
                tokens[tokenCount] = (token *)malloc(sizeof(token));

                // Initialize errorHandle and comment flag
                tokens[tokenCount]->errorHandle = 0;

                // Use function to grab token value
                tokens[tokenCount]->tokenValue = returnTokenTypeForIdentifierOrNumber(currentString);

                // Store name of the identifier
                // allocate memory for it first
                tokens[tokenCount]->tokenName = (char *)malloc(sizeof(char) * (strlen(currentString)) + 1);

                strcpy(tokens[tokenCount]->tokenName, currentString);

                // Set errorHandle flag to 1
                if (currentStringBuildIndex > maxCharLength)
                {
                    tokens[tokenCount]->errorHandle = 1;
                }

                // Print lexeme table and account for errors
                if (tokens[tokenCount]->errorHandle == 0)
                {
                    // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);
                }
                if (tokens[tokenCount]->errorHandle == 1)
                {
                    printf("Error: Name is too long %s\n", currentString);
                    exit(1);
                }

                // Increment the token count
                tokenCount++;

                continue;
            }

            continue;
        }

        // If current character is a special symbol. Use current char not string
        else if (isSpecialSymbol(currentChar) && currentChar != ':' && currentChar != '<' && currentChar != '>')
        {
            // initialize token
            tokens[tokenCount] = (token *)malloc(sizeof(token));

            // Initialize errorHandle and comment flag
            tokens[tokenCount]->errorHandle = 0;

            // Use function to grab token value

            tokens[tokenCount]->tokenValue = returnTokenTypeForSymbol(currentChar);

            // print lexeme table
            // printf("%c\t\t%d\n", currentChar, tokens[tokenCount]->tokenValue);

            // Update token count
            tokenCount++;
        }

        // print invalid symbols
        else if ((returnTokenTypeForSymbol(currentChar) == -1) && currentChar != ':')
        {
            tokens[tokenCount] = (token *)malloc(sizeof(token));

            // Initialize errorHandle
            tokens[tokenCount]->errorHandle = 1;
            printf("Error: Invalid Symbol %c\n", currentChar);
            exit(1);
        }

        // case for ':'
        if (currentChar == ':')
        {
            // initialize token
            tokens[tokenCount] = (token *)malloc(sizeof(token));
            tokens[tokenCount]->errorHandle = 1;

            currentString[currentStringBuildIndex] = currentChar;

            // check if next char is '='
            if ((nextChar = fgetc(inputFile3)) == '=')
            {
                // MAKE FILE 2 POINT TO 3
                position2 = ftell(inputFile3);
                // move inputfile2 to poisiton inputFIle3
                fseek(inputFile2, position2, SEEK_SET);

                tokens[tokenCount]->errorHandle = 0;

                // manipulate string to add nextChar
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = nextChar;
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = '\0';

                // store token of := using the enum
                tokens[tokenCount]->tokenValue = becomessym;

                // Print lexeme table
                // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);

                tokenCount++;
                // Reset the word builder
                currentStringBuildIndex = 0;
                strcpy(currentString, "");
            }

            else
            {
                tokens[tokenCount]->errorHandle = 1;
                printf("Error: Invalid Symbol %c\n", currentChar);
                exit(1);
                tokenCount++;
            }
        }

        // case For >
        if (currentChar == '>')
        {
            // initialize
            tokens[tokenCount] = (token *)malloc(sizeof(token));
            tokens[tokenCount]->errorHandle = 0;

            currentString[currentStringBuildIndex] = currentChar;

            // check if next char is '='
            if ((nextChar = fgetc(inputFile3)) == '=')
            {
                // MAKE FILE 2 POINT TO 3
                position2 = ftell(inputFile3);
                // move inputfile2 to poisiton inputFIle3
                fseek(inputFile2, position2, SEEK_SET);

                tokens[tokenCount]->errorHandle = 0;

                // manipulate string to add nextChar
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = nextChar;
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = '\0';

                // store token of := using the enum
                tokens[tokenCount]->tokenValue = geqsym;

                // Print lexeme table
                // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);

                tokenCount++;
                // Reset the word builder
                currentStringBuildIndex = 0;
                strcpy(currentString, "");
            }

            // else print it alone
            // Print lexeme table
            else
            {
                tokens[tokenCount]->errorHandle = 0;
                tokens[tokenCount]->tokenValue = gtrsym;
                // printf("%c\t\t%d\n", currentChar, tokens[tokenCount]->tokenValue);
                tokenCount++;
            }
        }

        // case For <
        else if (currentChar == '<')
        {
            // initialize
            tokens[tokenCount] = (token *)malloc(sizeof(token));
            tokens[tokenCount]->errorHandle = 0;

            currentString[currentStringBuildIndex] = currentChar;

            // check if next char is '='
            // inputFile3 = inputFile2;
            if ((nextChar = fgetc(inputFile3)) == '=')
            {

                // MAKE FILE 2 POINT TO 3
                position2 = ftell(inputFile3);
                // move inputfile2 to poisiton inputFIle3
                fseek(inputFile2, position2, SEEK_SET);

                tokens[tokenCount]->errorHandle = 0;

                // manipulate string to add nextChar
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = nextChar;
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = '\0';

                // store token of := using the enum
                tokens[tokenCount]->tokenValue = leqsym;

                // Print lexeme table
                // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);

                tokenCount++;
                // Reset the word builder
                currentStringBuildIndex = 0;
                strcpy(currentString, "");

                continue;
            }

            // reset input file 3 position to 2
            else
            {
                // make inputfile3 point to inputfile 2
                long int position = ftell(inputFile2);
                fseek(inputFile3, position, SEEK_SET);
            }

            // check if next char is '>'
            if ((nextChar = fgetc(inputFile3)) == '>')
            {
                // MAKE FILE 2 POINT TO 3
                position2 = ftell(inputFile3);
                // move inputfile2 to poisiton inputFIle3
                fseek(inputFile2, position2, SEEK_SET);

                tokens[tokenCount]->errorHandle = 0;

                // manipulate string to add nextChar
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = nextChar;
                currentStringBuildIndex++;
                currentString[currentStringBuildIndex] = '\0';

                // store token of := using the enum
                tokens[tokenCount]->tokenValue = neqsym;

                // Print lexeme table
                // printf("%s\t\t%d\n", currentString, tokens[tokenCount]->tokenValue);

                tokenCount++;
                // Reset the word builder
                currentStringBuildIndex = 0;
                strcpy(currentString, "");

                continue;
            }

            // else print it alone
            // Print lexeme table
            else
            {
                tokens[tokenCount]->errorHandle = 0;
                tokens[tokenCount]->tokenValue = lessym;
                // printf("%c\t\t%d\n", currentChar, tokens[tokenCount]->tokenValue);
                tokenCount++;
            }
        }

    } // end of while loop
    // check if im still in comment mode
    if (inComment == 1)
    {
        printf("Error: Comment was never closed");
        exit(1);
    }

    // Print the token List
    // printTokenList();

    // clean up token list to pass in new token list
    for (int i = 0; i < tokenCount; i++)
    {
        // skip -1
        if (tokens[i]->tokenValue == -1 || tokens[i]->errorHandle == 1)
        {
            continue;
        }

        // add to new tokenslist otherwise
        else
        {

            tokensOff[tokenIndexOff] = *tokens[i];
            tokenIndexOff++;
            tokenCountOff++;
        }
    }

    //------------------------------------------------------------------------------
    // HW4 EXECUTION/OUTPUT AREA

    // Set the token index back to the start
    tokenIndexOff = 0;

    // start the parser HW3
    program();

    printGeneratedCode();

    // After execution. Program works.
    printf("No errors, program is syntactically correct.\n");

    printHw1Input();

    //------------------------------------------------------------------------------

    // close file ptrs
    fclose(inputFile);
    fclose(inputFile2);
    fclose(inputFile3);

    return 0;
} // end of main

// Functions hw2
// Returns 1 if isReservedWord, 0 otherwise
int isReservedWord(char *word)
{
    for (int i = 0; i < reservedWordsCount; i++)
    {
        if (strcmp(word, reservedWords[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Returns 1 if isSpecialSymbol, 0 otherwise
int isSpecialSymbol(char curChar)
{
    for (int i = 0; i < specialSymbolsCount; i++)
    {
        // Check for a match with curChar and specialSymbols
        if (curChar == specialSymbols[i])
        {
            return 1;
        }
    }
    return 0;
}

// Returns token type for a reserved word
token_type returnTokenTypeForReserved(char *word)
{
    if (strcmp(word, "const") == 0)
    {
        return constsym;
    }
    else if (strcmp(word, "var") == 0)
    {
        return varsym;
    }
    else if (strcmp(word, "procedure") == 0)
    {
        return procsym;
    }
    else if (strcmp(word, "call") == 0)
    {
        return callsym;
    }
    else if (strcmp(word, "begin") == 0)
    {
        return beginsym;
    }
    else if (strcmp(word, "end") == 0)
    {
        return endsym;
    }
    else if (strcmp(word, "if") == 0)
    {
        return ifsym;
    }
    else if (strcmp(word, "fi") == 0)
    {
        return fisym;
    }
    else if (strcmp(word, "then") == 0)
    {
        return thensym;
    }
    else if (strcmp(word, "else") == 0)
    {
        return elsesym;
    }
    else if (strcmp(word, "while") == 0)
    {
        return whilesym;
    }
    else if (strcmp(word, "do") == 0)
    {
        return dosym;
    }
    else if (strcmp(word, "read") == 0)
    {
        return readsym;
    }
    else if (strcmp(word, "write") == 0)
    {
        return writesym;
    }

    else if (strcmp(word, "odd") == 0)
    {
        return oddsym;
    }

    // Return -1 if not recognized
    return -1;
}

// Returns token type for a Symbol
token_type returnTokenTypeForSymbol(char curChar)
{
    switch (curChar)
    {
    case '+':
        return plussym;
    case '-':
        return minussym;
    case '*':
        return multsym;
    case '/':
        return slashsym;
    case '(':
        return lparentsym;
    case ')':
        return rparentsym;
    case '=':
        return eqsym;
    case ',':
        return commasym;
    case '.':
        return periodsym;
    case '<':
        return lessym;
    case '>':
        return gtrsym;
    case ';':
        return semicolonsym;
    case ' ':
        return 1;
    case '\n':
        return 1;
    case '\t':
        return 1;
    case '\r':
        return 1;
    default:
        return -1; // Not in symbol list, return -1
    }
}

// Returns token type for Identifier or Number
token_type returnTokenTypeForIdentifierOrNumber(char *curCharSet)
{
    // If we look at the first character of the char array,
    // we can determine if its a set of numbers or an identifier

    // if first char is number, then we need the number token
    if (isdigit(curCharSet[0]))
    {
        return numbersym;
    }

    // if first char is letter, then we need the identifier token
    if (isalpha(curCharSet[0]))
    {
        return identsym;
    }

    // return -1 if not recognized
    return -1;
}

// HW 3 FUNCTIONS
int symbolTableCheck(char *string)
{

    for (int i = symbolIndex; i >= 0; i--)
    {
        if (strcmp(string, symbolTable[i].name) == 0 && symbolTable[i].level <= curLevel && symbolTable[i].kind != 3)
        {
            return i;
            break;
        }
    }

    // handle the case for procedure or const Assignment. Look for a same identiffier name with a var kind thats 3 for procedure.
    for (int i = symbolIndex; i >= 0; i--)
    {
        if (strcmp(string, symbolTable[i].name) == 0 && symbolTable[i].level <= curLevel && (symbolTable[i].kind == 3 || symbolTable[i].kind == 1))
        {
            return i;
            // return -2;
            break;
        }
    }

    return -1;
}

void error(int errorChoice)
{
    // Switch case for each error type
    switch (errorChoice)
    {
    case 1:
        printf("Error 1: Program must end with period\n");
        break;
    case 2:
        printf("Error 2: const, var, procedure, and call must be followed by identifier\n");
        break;
    case 3:
        printf("Error 3: Symbol name has already been declared\n");
        break;
    case 4:
        printf("Error 4: Constants must be assigned with =\n");
        break;
    case 5:
        printf("Error 5: Constants must be assigned an integer value\n");
        break;
    case 6:
        printf("Error 6: Constant, variable, procedure, and end declarations must be followed by a semicolon\n");
        break;
    case 7:
        printf("Error 7: Undeclared identifier %s\n", tokensOff[tokenIndexOff].tokenName);
        break;
    case 8:
        printf("Error 8: Only variable values may be altered. Assignment to constant or procedure is not allowed.\n");
        break;
    case 9:
        printf("Error 9: Assignment statements must use :=\n");
        break;
    case 10:
        printf("Error 10: begin must be followed by end\n");
        break;
    case 11:
        printf("Error 11: if must be followed by then\n");
        break;
    case 12:
        printf("Error 12: while must be followed by do\n");
        break;
    case 13:
        printf("Error 13: Condition must contain comparison operator\n");
        break;
    case 14:
        printf("Error 14: Right parenthesis must follow left parenthesis\n");
        break;
    case 15:
        printf("Error 15: Arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
        break;
    case 16:
        printf("Error 16: then must be followed by fi\n");
        break;
    case 17:
        printf("Error 17: Identifer is not a procedure. Call of a constant or variable is meaningless.\n");
        break;
    case 18:
        printf("Error 18: Max level exceded.\n");
        break;
    case 19:
        printf("Error 19: = must be followed by a number.\n");
        break;
    case 20:
        printf("Error 20: Identifer must be followed by =\n");
        break;
    case 21:
        printf("Error 21: Expression must not contain a procedure identifer.\n");
        break;
    }
    // Halt the program becuase error was detected
    exit(1);
}

// Function to emit code
void emit(int op, int l, int m)
{
    code[currentCodeIndex].op = op;
    code[currentCodeIndex].l = l;
    code[currentCodeIndex].m = m;
    code[currentCodeIndex].line = currentCodeIndex;

    // Increment count count and idex
    codeCount++;
    currentCodeIndex++;
}

// Function to parse PROGRAM
void program()
{
    block();

    // tokenIndex should be at the end
    if (tokenIndexOff > tokenCountOff || tokensOff[tokenIndexOff].tokenValue != periodsym)
    {
        error(1);
    }

    // emit EOP. program has successfully ended.
    emit(9, 0, 3);
}

// Function to parse BLOCK //hw4
void block() // wanna know what "spcae" means
{
    // initialize level system
    curLevel++;

    // used to later delete recent symbol
    prevSymbolIndex = symbolIndex;

    // bonus
    // emit jump and store jump address
    int jmpAddr = currentCodeIndex; // this stores jump address. We access this address through code[jmpAddr] later.
    emit(7, 0, 0);                  // jump

    int space = 3; // Space is 3 because StaticLink, DynamicLink, and ReturnAddress.

    int numVars = 0;

    // check if level out of bounds
    if (symbolTable[symbolIndex].level > curLevel)
    {
        error(18);
    }

    // CONST
    if (tokensOff[tokenIndexOff].tokenValue == constsym)
    {
        constDeclaration();
    }

    // VAR
    if (tokensOff[tokenIndexOff].tokenValue == varsym)
    {
        numVars = varDeclaration();
    }

    // PROCEDURE
    if (tokensOff[tokenIndexOff].tokenValue == procsym)
    {
        procedureDeclaration();
    }

    // emit INC (M = 3 + numVars)
    emit(6, 0, numVars + space);

    // Bonus:
    // Recitation shows: code[jmpaddr].addr = NEXT_CODE_ADDR;
    nextCodeAddr = code[currentCodeIndex - 1].line;
    code[jmpAddr].m = nextCodeAddr * 3; // do the * 3

    // STATEMENT
    statement();

    if (curLevel != 0)
    {
        // Emit RTN
        emit(2, 0, 0);
    }

    // delete used symbols from procedure
    curLevel--;
    symbolIndex = prevSymbolIndex;
}

void procedureDeclaration() // hw4
{
    // Bonus: I think this while should be an if, but it shouldnt change anything.
    while (tokensOff[tokenIndexOff].tokenValue == procsym)
    {
        // Get next token
        tokenIndexOff++;
        if (tokensOff[tokenIndexOff].tokenValue != identsym)
        {
            error(2); // const, var, and read keywords must be followed by identifier
        }

        // Add to symbol table
        symbolTable[symbolIndex].kind = 3;                                         // kind (proc)
        strcpy(symbolTable[symbolIndex].name, tokensOff[tokenIndexOff].tokenName); // name
        symbolTable[symbolIndex].val = 0;                                          // val
        symbolTable[symbolIndex].level = curLevel;                                 // level
        symbolTable[symbolIndex].addr = currentCodeIndex;                          // addr
        symbolTable[symbolIndex].mark = 0;                                         // mark

        // Store current procedure index in the global variable to be used later in "call"
        currentProcedureIndex = symbolIndex;

        symbolCount++;
        symbolIndex++;

        // Get next token
        tokenIndexOff++;
        if (tokensOff[tokenIndexOff].tokenValue != semicolonsym)
        {
            error(6); // Constant, variable, procedure, and end declarations must be followed by a semicolon
        }

        tokenIndexOff++;

        // CALL BLOCK AND GET NEW procedureAddress.
        block();

        // CHECK NEXT TOKEN TO BE SEMICOLON
        if (tokensOff[tokenIndexOff].tokenValue != semicolonsym)
        {
            error(6); // Constant, variable, procedure, and end declarations must be followed by a semicolon
        }

        // Get next token
        tokenIndexOff++;
    }
}

// Function to parse constDeclaration
void constDeclaration()
{

    if (tokensOff[tokenIndexOff].tokenValue == constsym)
    {
        do
        {
            // Get next token
            tokenIndexOff++;
            if (tokensOff[tokenIndexOff].tokenValue != identsym)
            {
                error(2); // const, var, and read keywords must be followed by identifier
            }

            // Check if identifier already exists. This is fine i think

            if (symbolTableCheckWithLevel(tokensOff[tokenIndexOff].tokenName) != -1)
            {
                error(3); // Symbol name has already been declared + (AT THAT LEVEL)
            }

            // Save identifier name
            strcpy(symbolTable[symbolIndex].name, tokensOff[tokenIndexOff].tokenName); // name

            // Get next token
            tokenIndexOff++;
            if (tokensOff[tokenIndexOff].tokenValue != eqsym)
            {
                error(4); // Constants must be assigned with =
            }

            // Get next token
            tokenIndexOff++;
            if (tokensOff[tokenIndexOff].tokenValue != numbersym)
            {
                error(5); // Constants must be assigned an integer value
            }

            // Add to symbol table
            symbolTable[symbolIndex].kind = 1;                                       // kind (const)
            symbolTable[symbolIndex].val = atoi(tokensOff[tokenIndexOff].tokenName); // val
            symbolTable[symbolIndex].level = curLevel;                               // level
            symbolTable[symbolIndex].addr = 0;                                       // addr
            symbolTable[symbolIndex].mark = 0;                                       // mark

            symbolCount++;
            symbolIndex++;

            tokenIndexOff++;
        } while (tokensOff[tokenIndexOff].tokenValue == commasym);

        if (tokensOff[tokenIndexOff].tokenValue != semicolonsym)
        {
            error(6); // Constant and variable declarations must be followed by a semicolon
        }
        // Get next token
        tokenIndexOff++;
    }
}

// Function to parse varDeclaration
int varDeclaration()
{
    int numVars = 0;

    // If token is varsym
    if (tokensOff[tokenIndexOff].tokenValue == varsym)
    {
        do
        {
            numVars++;

            // Get next token
            tokenIndexOff++;

            // Check if the next token is an identifier
            if (tokensOff[tokenIndexOff].tokenValue != identsym)
            {
                error(2); // var keyword must be followed by an identifier
            }

            // BONUS: Modify code to account for different levels
            //  Check if the identifier has already been declared
            if (symbolTableCheckWithLevel(tokensOff[tokenIndexOff].tokenName) != -1)
            {
                error(3); // Symbol name has already been declared
            }

            // Add the variable to the symbol table
            symbolTable[symbolIndex].kind = 2;                                         // kind (var)
            strcpy(symbolTable[symbolIndex].name, tokensOff[tokenIndexOff].tokenName); // name
            symbolTable[symbolIndex].val = 0;                                          // val
            symbolTable[symbolIndex].level = curLevel;                                 // level
            symbolTable[symbolIndex].addr = (numVars + 2);                             // addr
            symbolTable[symbolIndex].mark = 0;                                         // mark

            symbolCount++;
            symbolIndex++;

            // Get next token
            tokenIndexOff++;
        } while (tokensOff[tokenIndexOff].tokenValue == commasym);

        // After the loop, check for semicolonsym
        if (tokensOff[tokenIndexOff].tokenValue != semicolonsym)
        {
            error(6); // Constant, variable, procedure, and end declarations must be followed by a semicolon
        }
        // Get next token
        tokenIndexOff++;
    }

    return numVars;
}

// Function to parse statement
void statement()
{
    // If token is an identifier
    if (tokensOff[tokenIndexOff].tokenValue == identsym)
    {
        int symIdx = symbolTableCheck(tokensOff[tokenIndexOff].tokenName);

        // Get the identifier
        char identifier[15];
        strcpy(identifier, tokensOff[tokenIndexOff].tokenName);

        // Find the position of the identifier in the symbol table
        int i = symbolTablePosition(identifier);

        if (symbolTable[symIdx].kind == 3)
        {
            error(8);
        }

        if (symIdx == -1)
        {
            error(7); // Undeclared identifier
        }
        if (symbolTable[symIdx].kind != 2) // Not a variable
        {
            error(8); // Only variable values may be altered
        }

        // Get next token
        tokenIndexOff++;
        if (tokensOff[tokenIndexOff].tokenValue != becomessym)
        {
            error(9); // Assignment statements must use :=
        }

        // Get next token
        tokenIndexOff++;
        expression();

        // Emit STO (M = table[symIdx].addr)
        emit(4, curLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr);
        return;
    }

    // hw4 start
    // If token is callsym
    if (tokensOff[tokenIndexOff].tokenValue == callsym)
    {
        // Get next token
        tokenIndexOff++;

        // Check if the next token is an identifier
        if (tokensOff[tokenIndexOff].tokenValue != identsym)
        {
            error(2); // callsym must be followed by an identifier
        }
        else
        {
            // Get the identifier
            char identifier[15];
            strcpy(identifier, tokensOff[tokenIndexOff].tokenName);

            // Find the position of the identifier in the symbol table
            int i = symbolTablePosition(identifier);

            if (i == -1)
            {
                error(7); // Undeclared Identifer
            }
            else
            {
                // Check if the identifier is a procedure
                if (symbolTable[i].kind == 3) // Assuming kind 3 represents a procedure
                {
                    // Generate code for procedure call
                    emit(5, curLevel - symbolTable[i].level, symbolTable[i].addr * 3);
                }
                else
                {
                    error(17); // Identifier is not a procedure
                }
            }

            // Get next token
            tokenIndexOff++;
        }
    }
    // hw4 end

    // If token is beginsym
    if (tokensOff[tokenIndexOff].tokenValue == beginsym)
    {
        do
        {
            // Get next token
            tokenIndexOff++;
            statement();
        } while (tokensOff[tokenIndexOff].tokenValue == semicolonsym);

        if (tokensOff[tokenIndexOff].tokenValue != endsym)
        {
            error(10); // begin must be followed by end
        }
        // Get next token
        tokenIndexOff++;
        return;
    }

    // If token is ifsym
    if (tokensOff[tokenIndexOff].tokenValue == ifsym)
    {
        // Get next token
        tokenIndexOff++;
        condition();
        int jpcIdx = currentCodeIndex;
        // Emit JPC
        emit(8, 0, 0);
        if (tokensOff[tokenIndexOff].tokenValue != thensym)
        {
            error(11); // if must be followed by then
        }
        // Get next token
        tokenIndexOff++;

        statement();

        code[jpcIdx].m = currentCodeIndex;
        code[jpcIdx].m = code[jpcIdx].m * 3;

        if (tokensOff[tokenIndexOff].tokenValue != fisym)
        {
            error(16); // then must be followed by fi
        }

        // Get next token
        tokenIndexOff++;

        return;
    }

    // If token is whilesym
    if (tokensOff[tokenIndexOff].tokenValue == whilesym)
    {
        // Get next token
        tokenIndexOff++;
        int loopIdx = currentCodeIndex;
        condition();
        if (tokensOff[tokenIndexOff].tokenValue != dosym)
        {
            error(12); // while must be followed by do
        }

        // Get next token
        tokenIndexOff++;

        int jpcIdx = currentCodeIndex;
        // Emit JPC
        emit(8, 0, 0);
        statement();
        // Emit JMP (M = loopIdx)
        emit(7, 0, loopIdx * 3);

        code[jpcIdx].m = currentCodeIndex;
        code[jpcIdx].m = code[jpcIdx].m * 3;
        return;
    }

    // If token is readsym
    if (tokensOff[tokenIndexOff].tokenValue == readsym)
    {
        // Get next token
        tokenIndexOff++;
        if (tokensOff[tokenIndexOff].tokenValue != identsym)
        {
            error(2); // const, var, and read keywords must be followed by identifier
        }

        int symIdx = symbolTableCheck(tokensOff[tokenIndexOff].tokenName);

        if (symIdx == -1)
        {
            error(7); // Undeclared identifier
        }
        if (symbolTable[symIdx].kind != 2) // Not a variable
        {
            error(8); // Only variable values may be altered
        }
        // Get next token
        tokenIndexOff++;
        // Emit READ (SIN)
        emit(9, 0, 2);
        // Emit STO (M = table[symIdx].addr)
        emit(4, curLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr);
        return;
    }

    // If token is writesym
    if (tokensOff[tokenIndexOff].tokenValue == writesym)
    {
        // Get next token
        tokenIndexOff++;
        expression();
        // Emit WRITE (SOU)
        emit(9, 0, 1);
        return;
    }
}

// Function to parse condition
void condition()
{
    if (tokensOff[tokenIndexOff].tokenValue == procsym)
    {
        error(21); // Expression must not contain a procedure identifer.
    }
    if (tokensOff[tokenIndexOff].tokenValue == oddsym)
    {
        // Get next token
        tokenIndexOff++;
        expression();
        // Emit ODD
        emit(2, 0, 11); // ODD
    }

    else
    {
        expression();

        if (tokensOff[tokenIndexOff].tokenValue == eqsym)
        {
            // Get next token
            tokenIndexOff++;
            expression();
            emit(2, 0, 5); // EQL
        }
        else if (tokensOff[tokenIndexOff].tokenValue == neqsym)
        {
            // Get next token
            tokenIndexOff++;
            expression();
            emit(2, 0, 6); // NEQ
        }
        else if (tokensOff[tokenIndexOff].tokenValue == lessym)
        {
            // Get next token
            tokenIndexOff++;
            expression();
            emit(2, 0, 7); // LSS
        }
        else if (tokensOff[tokenIndexOff].tokenValue == leqsym)
        {
            // Get next token
            tokenIndexOff++;
            expression();
            emit(2, 0, 8); // LEQ
        }
        else if (tokensOff[tokenIndexOff].tokenValue == gtrsym)
        {
            // Get next token
            tokenIndexOff++;
            expression();
            emit(2, 0, 9); // GTR
        }
        else if (tokensOff[tokenIndexOff].tokenValue == geqsym)
        {
            // Get next token
            tokenIndexOff++;
            expression();
            emit(2, 0, 10); // GEQ
        }
        else
        {
            error(13); // comparison error
        }
    }
}

// Function to parse expression
void expression()
{
    //  Parse the TERM
    term();

    // Check for additional plussym or minussym
    while (tokensOff[tokenIndexOff].tokenValue == plussym || tokensOff[tokenIndexOff].tokenValue == minussym)
    {
        // Check if token is plussym
        if (tokensOff[tokenIndexOff].tokenValue == plussym)
        {
            // Get next token
            tokenIndexOff++;

            // Parse the TERM
            term();

            emit(2, 0, 1); // ADD
        }
        else
        {
            // Get next token
            tokenIndexOff++;

            // Parse the TERM
            term();

            emit(2, 0, 2); // SUB
        }
    }
}

// Function to parse term
void term()
{
    // Parse the factor
    factor();

    // Check for additional multsym, slashsym, or modsym
    while (tokensOff[tokenIndexOff].tokenValue == multsym || tokensOff[tokenIndexOff].tokenValue == slashsym)
    {
        // Check if token is multsym
        if (tokensOff[tokenIndexOff].tokenValue == multsym)
        {
            // Get next token
            tokenIndexOff++;

            // Parse the FACTOR
            factor();

            emit(2, 0, 3); // MUL
        }
        else if (tokensOff[tokenIndexOff].tokenValue == slashsym)
        {
            // Get next token
            tokenIndexOff++;

            // Parse the FACTOR
            factor();

            emit(2, 0, 4); // DIV
        }
    }
}

// Function to parse factor
void factor()
{

    // If token is an identifier
    if (tokensOff[tokenIndexOff].tokenValue == identsym)
    {
        // Check symbol table for the identifier
        int symIdx = symbolTableCheck(tokensOff[tokenIndexOff].tokenName);

        // Get the identifier
        char identifier[15];
        strcpy(identifier, tokensOff[tokenIndexOff].tokenName);

        if (symIdx == -1)
        {
            error(7); // Undeclared identifier
        }

        // implement HW4 Error: Expression must not contain a procedure identifier
        if (symbolTable[symIdx].kind == 3)
        {
            error(21);
        }

        // If it's a constant
        if (symbolTable[symIdx].kind == 1)
        {
            // Emit LIT (M = table[symIdx].val)
            emit(1, 0, symbolTable[symIdx].val);
        }
        else
        {
            // Emit LOD (M = table[symIdx].addr)
            emit(3, curLevel - symbolTable[symIdx].level, symbolTable[symIdx].addr);
        }
        // Get next token
        tokenIndexOff++;
    }
    // If token is a number
    else if (tokensOff[tokenIndexOff].tokenValue == numbersym)
    {
        // Emit LIT
        emit(1, 0, atoi(tokensOff[tokenIndexOff].tokenName)); // atoi should convert ascii to integer?

        // Get next token
        tokenIndexOff++;
    }
    // If token is left parenthesis
    else if (tokensOff[tokenIndexOff].tokenValue == lparentsym)
    {
        // Get next token
        tokenIndexOff++;

        // Parse the expression
        expression();

        // Check for right parenthesis
        if (tokensOff[tokenIndexOff].tokenValue != rparentsym)
        {
            error(14); // Right parenthesis must follow left parenthesis
        }
        // Get next token
        tokenIndexOff++;
    }
    // If none of the above, it's an error
    else
    {
        error(15); // Arithmetic error
    }
}

int symbolTablePosition(char *identifier) // Hw4
{
    // Start searching from the current procedure's level (ptx) and move towards the base level
    for (int i = symbolIndex - 1; i >= 0; i--)
    {
        // Check if the identifier matches and it's in the current or outer scope (level <= ptx)
        if (strcmp(symbolTable[i].name, identifier) == 0)
        {
            return i; // Return the position of the identifier in the symbol table
        }
    }

    return -1; // Undeclared Identifer
}

// Bonus: Symbol table check but account for level this time
int symbolTableCheckWithLevel(char *string)
{
    // start at the end for searching to find symbols of the latest level. Not sure if its supposed to be SymbolCount - 1 or symbolCount.
    for (int i = symbolIndex - 1; i >= 0; i--)
    {
        // == curLevel makes sure things arent redeclared in the same level
        if (strcmp(string, symbolTable[i].name) == 0 && symbolTable[i].level == curLevel && symbolTable[i].kind != 3)
        {
            return i;
        }
    }
    return -1;
}

void printCleanList()
{

    printf("\nPRINT THE CLEAN LIST\n");

    for (int i = 0; i < tokenCountOff; i++)
    {

        printf("%d ", tokens[i]->tokenValue);

        // we print the names if its a identifier OR num
        if (tokens[i]->tokenValue == identsym || tokens[i]->tokenValue == numbersym)
        {
            printf("%s ", tokens[i]->tokenName);
        }
    }
    printf("\n\n");
}

void updateMark()
{
    // if its a var set mark to 1
    for (int i = 0; i < symbolCount; i++)
    {
        if (symbolTable[i].kind == 2 || symbolTable[i].kind == 1)
        {
            symbolTable[i].mark = 1;
        }
    }
}

void printSymbolTable()
{
    // print out symbol table
    printf("\nSymbol Table: \n");
    printf("Kind | Name\t | Value | Level | Address | Mark \n");
    printf("---------------------------------------------------\n");
    // Print each line of the table
    for (int i = 0; i < symbolCount; i++)
    {
        printf("   %d | \t\t%s | \t%d | \t%d | \t%d | \t%d ", symbolTable[i].kind, symbolTable[i].name,
               symbolTable[i].val, symbolTable[i].level, symbolTable[i].addr, symbolTable[i].mark);
        printf("\n");
    }
}

void printTokenList()
{
    printf("\n Token List:\n");

    for (int i = 0; i < tokenCount; i++)
    {
        // Print out token list if no error
        if (tokens[i]->tokenValue != -1 && tokens[i]->errorHandle == 0)
        {
            printf("%d ", tokens[i]->tokenValue);

            // we print the names if its a identifier OR num
            if (tokens[i]->tokenValue == identsym || tokens[i]->tokenValue == numbersym)
            {
                printf("%s ", tokens[i]->tokenName);
            }
        }
    }
    printf("\n");
}

void printGeneratedCode()
{
    // Print generated code
    printf("\nLine\tOP\tL\tM");
    for (int i = 0; i < codeCount; i++)
    {
        // Print code information
        printCode(code[i].op, code[i].l, code[i].m, i);
    }
    // New line
    printf("\n\n");
}

void printHw1Input() // hw4
{
    // File ptr for elf.txt
    FILE *elf = fopen("elf.txt", "w");
    if (elf == NULL)
    {
        printf("Error opening output file.\n");
        return;
    }

    printf("\nPrinting op codes to elf.txt...\n");
    for (int i = 0; i < codeCount; i++)
    {
        //printf("%d %d %d \n", code[i].op, code[i].l, code[i].m);
        fprintf(elf, "%d %d %d \n", code[i].op, code[i].l, code[i].m);
    }
    fclose(elf);
}

// Function to print generated code // hw4 moved this to the bottom for organization
void printCode(int op, int l, int m, int i)
{
    // new line for formatting
    printf("\n%d ", i);

    // switch case for each op code
    switch (op)
    {
    // cases for different op codes passed in
    case 1:
        printf("\tLIT\t%d\t%d\t", l, m);
        break;

    case 2:
        switch (m)
        {
        case 0: // RTN (0, 0) Remove activation record
            printf("\tRTN\t%d\t%d\t", l, m);
            break;

        case 1: // ADD
            printf("\tADD\t%d\t%d\t", l, m);
            break;

        case 2: // SUB
            printf("\tSUB\t%d\t%d\t", l, m);
            break;

        case 3: // MUL
            printf("\tMUL\t%d\t%d\t", l, m);
            break;

        case 4: // DIV
            printf("\tDIV\t%d\t%d\t", l, m);
            break;

        case 5: // EQL
            printf("\tEQL\t%d\t%d\t", l, m);
            break;

        case 6: // NEQ
            printf("\tNEQ\t%d\t%d\t", l, m);
            break;

        case 7: // LSS
            printf("\tLSS\t%d\t%d\t", l, m);
            break;

        case 8: // LEQ
            printf("\tLEQ\t%d\t%d\t", l, m);
            break;

        case 9: // GTR
            printf("\tGTR\t%d\t%d\t", l, m);
            break;

        case 10: // GEQ
            printf("\tGEQ\t%d\t%d\t", l, m);
            break;

        case 11: // ODD
            printf("\tODD\t%d\t%d\t", l, m);
            break;
        }
        break; // Case 2 break; (OPRs switch)

    case 3: // LOD (L, M)
        printf("\tLOD\t%d\t%d\t", l, m);
        break;

    case 4: // STO (L, M)
        printf("\tSTO\t%d\t%d\t", l, m);
        break;

    case 5:
        printf("\tCAL\t%d\t%d\t", l, m);
        break;

    case 6:
        printf("\tINC\t%d\t%d\t", l, m);
        break;

    case 7:
        printf("\tJMP\t%d\t%d\t", l, m);
        break;

    case 8:
        printf("\tJPC\t%d\t%d\t", l, m);
        break;

    case 9:
        switch (m)
        {
        case 1: // SYS (0, 1)
            printf("\tSOU\t%d\t%d\t", l, m);
            break;

        case 2: // SYS (0, 2)
            printf("\tSIN\t%d\t%d\t", l, m);
            break;

        case 3: // SYS (0, 3)
            printf("\tEOP\t%d\t%d\t", l, m);
            break;
        }
        break; // Case 9 break; (SYS switch)
    }
}