extern int fputc(int c, void* file);
extern void* stdout;

int main(void) {
    fputc(0x61, stdout);
    return 0;
}
