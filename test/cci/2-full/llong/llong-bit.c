// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if ((0xF0F0F0F0F0F0F0F0u & 0x0F0F0F0F0F0F0F0Fu) != 0ull) return 1;
    if ((0xF0F0F0F0F0F0F0F0u | 0x0F0F0F0F0F0F0F0Fu) != 0xFFFFFFFFFFFFFFFFull) return 2;
    if ((0xF0F0F0F0F0F0F0F0u ^ 0x0F0F0F0F0F0F0F0Fu) != 0xFFFFFFFFFFFFFFFFull) return 3;
    if (~0xF0F0F0F0F0F0F0F0u != 0x0F0F0F0F0F0F0F0Fu) return 4;

    if ((0x5959595959595959u & 0x9595959595959595u) != 0x1111111111111111ull) return 5;
    if ((0x5959595959595959u | 0x9595959595959595u) != 0xDDDDDDDDDDDDDDDDull) return 6;
    if ((0x5959595959595959u ^ 0x9595959595959595u) != 0xCCCCCCCCCCCCCCCCull) return 7;
    if (~0x5959595959595959u != 0xA6A6A6A6A6A6A6A6u) return 8;

    long long a = 0x5959595959595959u ;
    a &= 0x9595959595959595u;
    if (a != 0x1111111111111111ull) return 9;

    long long b = 0x5959595959595959u ;
    b |= 0x9595959595959595u;
    if (b != 0xDDDDDDDDDDDDDDDDull) return 10;

    long long c = 0x5959595959595959u ;
    c ^= 0x9595959595959595u;
    if (c != 0xCCCCCCCCCCCCCCCCull) return 11;
}
