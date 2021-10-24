
#ifndef DUZEZADANIECZESC2_CALC_H
#define DUZEZADANIECZESC2_CALC_H

typedef struct StackStruct {
    size_t top;
    size_t capacity;
    Poly* array;
}Stack;

typedef struct VectorStruct {
    size_t top;
    size_t capacity;
    Mono* array;
}Vector;

static Stack* CreateStack(size_t capacity);

static size_t IsFull(Stack* stack);

static size_t IsEmpty(Stack* stack);

static void Resize(Stack* stack, int capacity);

static size_t Size(Stack* stack);

static void Push(Stack* stack, Poly item);

static Poly Pop(Stack* stack);

static Poly Peek(Stack* stack);

static void PrintPoly(Poly* poly);

static bool CompareStrings(char* str1, char* str2 );

static int CountWhitespaceCharacters(char* input);

static bool CheckPolymonialCharacters(char* input);

static void FreeStack(Stack* stack);

static void ProcessPolymonial(char* input, Stack* stack);

static bool is_10(char* number);

static void ProcessDegByParameter(char* parameter, Stack* stack, bool* commandIsProcessed);

static void ProcessAtParameter(char* parameter, Stack* stack, bool* commandIsProcessed);

static void ProcessCommand(char* input, Stack* stack);

static void ProcessLine(char* inputLine, Stack* stack);

static void useCalculator(Stack* stack);

#endif //DUZEZADANIECZESC2_CALC_H
