// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;
    {
        defer x *= 3;
        x += 2;
        goto end;
    }
end:
    if (x != 6) return 1;
}
