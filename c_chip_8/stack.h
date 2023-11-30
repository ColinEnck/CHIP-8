typedef struct Stack
{
    unsigned short* array;
    unsigned int size;
} Stack;

short pop(Stack* stack)
{
    short ret = stack->array[stack->size-1];
    stack->array[stack->size-1] = 0;
    stack->size--;
    return ret;
}

void push(Stack* stack, unsigned short val)
{
    printf("PUSHING: %d\n", val);
    stack->array[stack->size] = val;
    stack->size++;
}

short top(Stack stack)
{
    return stack.array[stack.size-1];
}