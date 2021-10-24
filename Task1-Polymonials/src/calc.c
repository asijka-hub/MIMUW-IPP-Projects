#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <strings.h>
#include "poly.h"
#include "calc.h"

#define INIT_CAPACITY 30

int lineNumber = 0;

static void* SafeMalloc(size_t size)
{
    void* p = malloc(size);
    if (!p)
        exit(1);
    return p;
}

static void* SafeRealloc(void* ptr, size_t size)
{
    void* p = realloc(ptr, size);
    if (!p)
        exit(1);
    return p;
}

static Stack* CreateStack(size_t capacity)
{
    assert(capacity > 0);
    Stack* stack = (Stack*) SafeMalloc(sizeof (Stack));
    stack->capacity = capacity;
    stack->top = 0;
    stack->array = (Poly*)SafeMalloc(stack->capacity * sizeof(Poly));
    return stack;
}

static size_t IsFull(Stack* stack)
{
    return stack->top == stack->capacity;
}

static size_t IsEmpty(Stack* stack)
{
    return stack->top == 0;
}

static void Resize(Stack* stack, int capacity) {
    stack->capacity = capacity;
    stack->array = (Poly*) SafeRealloc(stack->array, capacity * sizeof (Poly));
}

static size_t Size(Stack* stack) {
    return stack->top;
}

static void Push(Stack* stack, Poly item)
{
    if (IsFull(stack)) {
        Resize(stack, stack->capacity * 2);
    }
    stack->array[stack->top] = item;
    stack->top++;
}

static Poly Pop(Stack* stack)
{
    assert(!IsEmpty(stack));
    stack->top--;
    return stack->array[stack->top];
}

static Poly Peek(Stack* stack)
{
    assert(!IsEmpty(stack));
    return stack->array[stack->top-1];
}

static Poly PeekOneDeeper(Stack* stack)
{
    assert(!IsEmpty(stack));
    return stack->array[stack->top - 2];
}

static Vector* CreateVec(size_t capacity)
{
    assert(capacity > 0);
    Vector* stack = (Vector*) SafeMalloc(sizeof (Vector));
    stack->capacity = capacity;
    stack->top = 0;
    stack->array = (Mono*)SafeMalloc(stack->capacity * sizeof(Mono));
    return stack;
}

static size_t IsFullVec(Vector* vector)
{
    return vector->top == vector->capacity;
}

static void ResizeVec(Vector* vector, int capacity) {
    vector->capacity = capacity;
    vector->array = (Mono*) SafeRealloc(vector->array, capacity * sizeof (Mono));
}

static size_t SizeVec(Vector* vector) {
    return vector->top;
}

static void PushVec(Vector* vector, Mono item)
{
    if (IsFullVec(vector))
        ResizeVec(vector, vector->capacity * 2);

    vector->array[vector->top] = item;
    vector->top++;
}

static Mono PopVec(Vector* vector)
{
    vector->top--;
    return vector->array[vector->top];
}

static void PrintPoly(Poly* poly) {
    if (PolyIsCoeff(poly)) {
        printf("%ld", poly->coeff);
        return;
    }

    for (size_t i = 0; i < poly->size; ++i) {
        if (i > 0)
            printf("+");

        printf("(");
        PrintPoly(&poly->arr[poly->size-i-1].p);
        printf(",");
        printf("%d",poly->arr[poly->size-i-1].exp);
        printf(")");
    }
}

static bool CompareStrings(char* str1, char* str2 ) {
    return strcmp (str1, str2) == 0 ? true : false;
}

static bool CharIsInStr(char* chr, char* str) {
    bool res = false;

    for (size_t i = 0; i < strlen(str); ++i) {
        if (*chr == *(str + i))
            res = true;
    }

    return res;
}

static int CountWhitespaceCharacters(char* input) {
    int count = 0;
    for (size_t i = 0; i < strlen(input); ++i) {
        if (CharIsInStr(input+i, " \n\r\t\v\f"))
            count++;
    }

    return count;
}

static bool CheckPolymonialCharacters(char* input) {
    for (size_t i = 0; i < strlen(input); ++i) {
        char* chr = input + i;
        if ( ((*chr > 57 || *chr < 48) && !CharIsInStr(chr, "(),+-\n")) || CharIsInStr(chr, " \t\v\r\f"))
            return false;
    }

    return true;
}

static void FreeVec(Vector * vec) {
    while (SizeVec(vec) > 0) {
        Mono mono = PopVec(vec);
        MonoDestroy(&mono);
    }

    free(vec->array);
    free(vec);
}

static void PrintfErrDegBy(bool* commandIsProcessed) {
    fprintf(stderr,"ERROR %d DEG BY WRONG VARIABLE\n", lineNumber);
    *commandIsProcessed = true;
}

static void PrintfErrWrongCommand(bool* commandIsProcessed) {
    fprintf(stderr,"ERROR %d WRONG COMMAND\n", lineNumber);
    *commandIsProcessed = true;
}

static void PrintfErrStackUnderflow(bool* commandIsProcessed) {
    fprintf(stderr,"ERROR %d STACK UNDERFLOW\n", lineNumber);
    *commandIsProcessed = true;
}

static void PrintfErrAt(bool* commandIsProcessed) {
    fprintf(stderr,"ERROR %d AT WRONG VALUE\n", lineNumber);
    *commandIsProcessed = true;
}

static void PrintfErrCompose(bool* commandIsProcessed) {
    fprintf(stderr,"ERROR %d COMPOSE WRONG PARAMETER\n", lineNumber);
    *commandIsProcessed = true;
}

static void PrintfErrPoly() {
    fprintf(stderr,"ERROR %d WRONG POLY\n", lineNumber);
}

static bool CheckIfNumberOrMinus(char c) {
    int chr = (int) c;
    return (chr >= 48 && chr <= 57) || chr == 45;
}

static void MoveOverByNumber(char* input, size_t* curr) {
    while (CheckIfNumberOrMinus(input[*curr])) {
        (*curr)++;
    }
}

static bool PolyIsCorrect(char* input) {
    int n = CountWhitespaceCharacters(input);
    if (n > 1 || !CheckPolymonialCharacters(input) || (input[0] != '(' && !CheckIfNumberOrMinus(input[0])))
        return false;

    size_t i = 0, openBrackets = 0, closeBrackets = 0, commas = 0;

    while (input[i] != '\n' && input[i] != '\0') {
        char curr = input[i], next = input[i + 1];

        if (CheckIfNumberOrMinus(curr)) {
            while (CheckIfNumberOrMinus(input[i + 1])) {
                i++;
            }

            if(!CharIsInStr(input + i + 1,",)\n\0"))
                return false;
        }

        if (curr == ',') {
            commas++;
            if (!CheckIfNumberOrMinus(next))
                return false;
        }

        if ((curr == '+' && next != '(') || (curr == '-' && !CheckIfNumberOrMinus(next)) || (curr == '-' && next == '-'))
            return false;

        if (curr == '(') {
            openBrackets++;
            if (!(CheckIfNumberOrMinus(next) || next == '-' || next == '('))
                return false;
        }

        if (curr == ')') {
            closeBrackets++;

            if(!CharIsInStr(input + i + 1,"+,\n\0"))
                return false;
        }

        i++;
    }

    if ((openBrackets != commas) || (openBrackets != closeBrackets))
        return false;

    return true;
}

static Poly ReturnPolyZeroHandleErrErrnoVec(Vector** pVector, bool printfErr) {
    errno = ERANGE;

    if (printfErr)
        PrintfErrPoly();

    if (pVector != NULL)
        FreeVec(*pVector);

    return PolyZero();
}

static Poly ParsePoly(char* input, size_t* curr) {
    if (CheckIfNumberOrMinus(input[*curr])) {
        errno = 0;

        long long convLL = strtoll(input + *curr, NULL, 10);
        if (errno == ERANGE)
            return ReturnPolyZeroHandleErrErrnoVec(NULL, true);

        MoveOverByNumber(input, curr);

        return PolyFromCoeff(convLL);
    }
    else if (input[*curr] == '(') {
        Vector* vec = CreateVec(INIT_CAPACITY);
        bool polyEnded = false;

        while (!polyEnded) {
            (*curr)++;

            Poly p = ParsePoly(input, curr);
            if (errno == ERANGE)
                return ReturnPolyZeroHandleErrErrnoVec(&vec, false);

            (*curr)++;

            long convL = strtol(input + *curr, NULL, 10);
            if (errno == ERANGE || convL < 0 || convL > 2147483647)
                return ReturnPolyZeroHandleErrErrnoVec(&vec, true);

            MoveOverByNumber(input, curr);

            if (convL == 0 || !PolyIsZero(&p)) {
                Mono m = MonoFromPoly(&p, (poly_exp_t) convL);
                PushVec(vec, m);
            }

            (*curr)++;

            if (input[*curr] == '+')
                (*curr)++;
            else
                polyEnded = true;
        }
        Poly res = PolyAddMonos(SizeVec(vec), vec->array);
        free(vec->array);
        free(vec);
        return res;
    }
    else
        return ReturnPolyZeroHandleErrErrnoVec(NULL, true);
}

static void ProcessPolymonial(char* input, Stack* stack) {
    if (PolyIsCorrect(input)) {
        char *str;
        str = strtok(input, "\n");

        size_t curr = 0;
        Poly poly = ParsePoly(str, &curr);

        if (errno != ERANGE)
            Push(stack, poly);
    }
    else
        PrintfErrPoly();
}

static bool is_10(char* number) {
    for (size_t i = 0; i < strlen(number); ++i) {
        char chr = *(number + i);
        if ((chr < 48 || chr > 57) && chr != '-')
            return false;
    }
    return true;
}

static void ProcessDegByParameter(char* parameter, Stack* stack, bool* commandIsProcessed) {
    unsigned long length = strlen(parameter);

    if (length < 6) {
        return;
    }
    else if (length == 6) {
        if (CompareStrings(parameter,"DEG_BY"))
            PrintfErrDegBy(commandIsProcessed);
    }
    else {
        if (strncmp (parameter, "DEG_BY ", 7) != 0) {
            if (strncmp (parameter, "DEG_BY", 6) == 0)
                return PrintfErrDegBy(commandIsProcessed);
            return;
        }

        int count = CountWhitespaceCharacters(parameter);

        if (count > 2)
            return PrintfErrDegBy(commandIsProcessed);

        char* str = strtok (parameter," \n");
        str = strtok(NULL, " \n");

        if (str == NULL)
            return PrintfErrAt(commandIsProcessed);

        char possible0 = *str;

        char* pEnd;

        unsigned long long convULL;
        convULL = strtoull(str, &pEnd, 10);

        if ((convULL == 0 && possible0 != '0') || possible0 == '+' || errno == ERANGE || !is_10(str))
            return PrintfErrDegBy(commandIsProcessed);

        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly polyTmp = Peek(stack);

        *commandIsProcessed = true;

        printf("%d\n",PolyDegBy(&polyTmp, convULL));
    }
}

static void ProcessAtParameter(char* parameter, Stack* stack, bool* commandIsProcessed) {
    unsigned long length = strlen(parameter);

    if (length < 2) {
        return;
    }
    else if (length == 2) {
        if (CompareStrings(parameter,"AT"))
            PrintfErrAt(commandIsProcessed);
    }
    else {
        if (strncmp (parameter, "AT ", 3) != 0) {
            if (strncmp (parameter, "AT", 2) == 0)
                return PrintfErrAt(commandIsProcessed);
            return;
        }

        int count = CountWhitespaceCharacters(parameter);
        if (count > 2)
            return PrintfErrAt(commandIsProcessed);

        char* str = strtok (parameter," \n");
        str = strtok(NULL, " \n");

        if (str == NULL)
            return PrintfErrAt(commandIsProcessed);

        char possible0 = *str;

        char* pEnd;

        long long convLL;
        convLL = strtoll(str, &pEnd, 10);

        if ((convLL == 0 && possible0 != '0') || possible0 == '+'  || errno == ERANGE || !is_10(str))
            return PrintfErrAt(commandIsProcessed);

        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly polyTmp = Pop(stack);
        Push(stack, PolyAt(&polyTmp, convLL));
        PolyDestroy(&polyTmp);

        *commandIsProcessed = true;
    }
}

static void ProcessCompose(char* parameter, Stack* stack, bool* commandIsProcessed) {
    unsigned long length = strlen(parameter);

    if (length < 7) {
        return;
    }
    else if (length == 7) {
        if (CompareStrings(parameter,"COMPOSE"))
            PrintfErrCompose(commandIsProcessed);
    }
    else {
        if (strncmp (parameter, "COMPOSE ", 8) != 0) {
            if (CompareStrings(parameter,"COMPOSE") || CompareStrings(parameter,"COMPOSE\n"))
                return PrintfErrCompose(commandIsProcessed);
            return;
        }

        int count = CountWhitespaceCharacters(parameter);
        if (count > 2)
            return PrintfErrCompose(commandIsProcessed);

        char* str = strtok (parameter," \n");
        str = strtok(NULL, " \n");

        if (str == NULL)
            return PrintfErrCompose(commandIsProcessed);

        char possible0 = *str;

        char* pEnd;

        unsigned long long convULL;
        convULL = strtoull(str, &pEnd, 10);

        if ((convULL == 0 && possible0 != '0') || possible0 == '+' || possible0 == '-'  || errno == ERANGE || !is_10(str))
            return PrintfErrCompose(commandIsProcessed);

        size_t k = convULL;

        if (Size(stack) < k + 1 || k + 1 == 0)
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly p = Pop(stack);
        Poly *polyTmp = (Poly*) SafeMalloc(sizeof (Poly) * k);

        for (size_t i = 0; i < k; ++i) {
            polyTmp[i] = Pop(stack);
        }
        Poly res = PolyCompose(&p, k, polyTmp);
        Push(stack, res);
        PolyDestroy(&p);

        for (size_t i = 0; i < k; ++i) {
            PolyDestroy(&polyTmp[i]);
        }
        free(polyTmp);

        *commandIsProcessed = true;
    }
}

static void PrintBool(bool isTrue) {
    if (isTrue)
        printf("%d\n", 1);
    else
        printf("%d\n", 0);
}

static void HandleBoolResult(bool* commandIsProcessed, Stack* stack, bool (*op)(const Poly *)) {
    Poly polyTmp = Peek(stack);
    PrintBool(op(&polyTmp));
    *commandIsProcessed = true;
}

static void HandleTwoArgs(Stack *stack, Poly (*operation)(const Poly *, const Poly *), bool* commandIsProcessed) {
        Poly poly1 = Pop(stack);
        Poly poly2 = Pop(stack);
        Poly res = operation(&poly1, &poly2);
        PolyDestroy(&poly1);
        PolyDestroy(&poly2);
        Push(stack, res);
        *commandIsProcessed = true;
}

static void HandleZero(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "ZERO") || CompareStrings(input, "ZERO\n")) && !*commandIsProcessed) {
        Push(stack, PolyZero());
        *commandIsProcessed = true;
    }
}

static void HandleIsCoeff(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "IS_COEFF") || CompareStrings(input, "IS_COEFF\n")) && !*commandIsProcessed) {
        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);
        HandleBoolResult(commandIsProcessed, stack, PolyIsCoeff);
    }
}

static void HandleIsZero(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "IS_ZERO") || CompareStrings(input, "IS_ZERO\n")) && !*commandIsProcessed) {
        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);
        HandleBoolResult(commandIsProcessed, stack, PolyIsZero);
    }
}

static void HandleClone(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "CLONE") || CompareStrings(input, "CLONE\n")) && !*commandIsProcessed) {
        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly polyTmp = Peek(stack);
        Push(stack, PolyClone(&polyTmp));

        *commandIsProcessed = true;
    }
}

static void HandlePop(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "POP") || CompareStrings(input, "POP\n")) && !*commandIsProcessed) {
        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly poly = Pop(stack);
        PolyDestroy(&poly);

        *commandIsProcessed = true;
    }
}

static void HandleAdd(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "ADD") || CompareStrings(input, "ADD\n")) && !*commandIsProcessed) {
        if (Size(stack) < 2)
            return PrintfErrStackUnderflow(commandIsProcessed);
        HandleTwoArgs(stack, PolyAdd, commandIsProcessed);
    }
}

static void HandleMul(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "MUL") || CompareStrings(input, "MUL\n")) && !*commandIsProcessed) {
        if (Size(stack) < 2)
            return PrintfErrStackUnderflow(commandIsProcessed);
        HandleTwoArgs(stack, PolyMul, commandIsProcessed);
    }
}

static void HandleNeg(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "NEG") || CompareStrings(input, "NEG\n")) && !*commandIsProcessed) {
        if (IsEmpty(stack))
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly poly1 = Pop(stack);
        Push(stack, PolyNeg(&poly1));
        PolyDestroy(&poly1);

        *commandIsProcessed = true;
    }
}

static void HandleSub(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "SUB") || CompareStrings(input, "SUB\n")) && !*commandIsProcessed) {
        if (Size(stack) < 2)
            return PrintfErrStackUnderflow(commandIsProcessed);
        HandleTwoArgs(stack, PolySub, commandIsProcessed);
    }
}

static void HandleIsEq(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "IS_EQ") || CompareStrings(input, "IS_EQ\n")) && !*commandIsProcessed) {
        if (Size(stack) < 2)
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly poly1 = Peek(stack);
        Poly poly2 = PeekOneDeeper(stack);
        PrintBool(PolyIsEq(&poly1, &poly2));
        *commandIsProcessed = true;
    }
}

static void HandleDeg(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "DEG") || CompareStrings(input, "DEG\n")) && !*commandIsProcessed) {
        if (Size(stack) < 1)
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly Poly1 = Peek(stack);
        int deg = PolyDeg(&Poly1);
        printf("%d\n", deg);
        *commandIsProcessed = true;
    }
}

static void HandlePrint(char* input, Stack* stack, bool* commandIsProcessed) {
    if ((CompareStrings(input, "PRINT") || CompareStrings(input, "PRINT\n")) && !*commandIsProcessed) {
        if (Size(stack) < 1)
            return PrintfErrStackUnderflow(commandIsProcessed);

        Poly Poly1 = Peek(stack);
        PrintPoly(&Poly1);
        printf("\n");
        *commandIsProcessed = true;
    }
}

static void HandleDegBy(char* input, Stack* stack, bool* commandIsProcessed) {
    if (!*commandIsProcessed)
        ProcessDegByParameter(input, stack, commandIsProcessed);
}

static void HandleAt(char* input, Stack* stack, bool* commandIsProcessed) {
    if (!*commandIsProcessed)
        ProcessAtParameter(input, stack, commandIsProcessed);
}

static void HandleCompose(char* input, Stack* stack, bool* commandIsProcessed) {
    if (!*commandIsProcessed)
        ProcessCompose(input, stack, commandIsProcessed);
}

static void ProcessCommand(char* input, Stack* stack) {
    bool commandIsProcessed = false;

    HandleZero(input, stack, &commandIsProcessed);

    HandleIsCoeff(input, stack, &commandIsProcessed);

    HandleZero(input, stack, &commandIsProcessed);

    HandleIsZero(input, stack, &commandIsProcessed);

    HandleClone(input, stack, &commandIsProcessed);

    HandlePop(input, stack, &commandIsProcessed);

    HandleAdd(input, stack, &commandIsProcessed);

    HandleMul(input, stack, &commandIsProcessed);

    HandleNeg(input, stack, &commandIsProcessed);

    HandleSub(input, stack, &commandIsProcessed);

    HandleIsEq(input, stack, &commandIsProcessed);

    HandleDeg(input, stack, &commandIsProcessed);

    HandlePrint(input, stack, &commandIsProcessed);

    HandleDegBy(input, stack, &commandIsProcessed);

    HandleAt(input, stack, &commandIsProcessed);

    HandleCompose(input, stack, &commandIsProcessed);

    if (!commandIsProcessed)
        PrintfErrWrongCommand(&commandIsProcessed);
}

static void ProcessLine(char* inputLine, Stack* stack) {
    if (*inputLine == '#' || CompareStrings(inputLine,"\n"))
        return;

    if ((*inputLine >= 65 && *inputLine <= 90) || (*inputLine >= 97 && *inputLine <= 122))
        ProcessCommand(inputLine, stack);
    else
        ProcessPolymonial(inputLine, stack);
}

void useCalculator(Stack* stack) {
    char* input= NULL;
    size_t bufsize = 32;
    size_t characters;

    input = (char *)SafeMalloc(bufsize * sizeof(char));

    characters = getline(&input, &bufsize, stdin);

    while (characters != ((size_t) -1)) {
        lineNumber++;
        ProcessLine(input, stack);
        characters = getline(&input, &bufsize, stdin);
    }

    free(input);
}

static void FreeStack(Stack* stack) {
    while (Size(stack) > 0) {
        Poly poly = Pop(stack);
        PolyDestroy(&poly);
    }

    free(stack->array);
    free(stack);
}

int main() {
    Stack* stack = CreateStack(INIT_CAPACITY);
    useCalculator(stack);
    FreeStack(stack);
}