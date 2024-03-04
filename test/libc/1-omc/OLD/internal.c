// hack hack hack to make test suite work

typedef void FILE;

int fprintf(FILE* restrict stream, const char* restrict format, ...);
int fflush(FILE* stream);
extern FILE* stdout;
extern FILE* stderr;
void abort();

void __assert_fail(const char* expression, const char* file,
        int line, const char* function)
{
    /* The exact string suggested by the C99 spec */
    fprintf(stderr, "Assertion failed: %s, function %s, file %s, line %i.\n",
            expression, function, file, line);
    fflush(stderr);
    abort();
}

void putd(int v) {
    fprintf(stdout, "%i", v);
}
