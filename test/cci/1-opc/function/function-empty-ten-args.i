int foo(int a, int* b, int** c, int*** d, int**** e,
        char f, char* g, char** h, char*** i, char**** j)
{
    return 0;
}

int main(void) {
    return foo(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
