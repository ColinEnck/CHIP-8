typedef struct Stack
{
    unsigned short* array;
    unsigned int size;
} Stack;

unsigned short pop(Stack* stack)
{
    unsigned short ret = stack->array[stack->size-1];
    stack->array[stack->size-1] = 0;
    stack->size--;
    return ret;
}

void push(Stack* stack, unsigned short val)
{
    stack->array[stack->size] = val;
    stack->size++;
}

char top(Stack stack)
{
    return stack.array[stack.size-1];
}