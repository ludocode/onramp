// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // signed arithmetic single pointers
    if ((int)((char*)100 + 1) != 101) {return 1;}
    if ((int)((short*)100 + 1) != 102) {return 1;}
    if ((int)((int*)100 + 1) != 104) {return 1;}
    if ((int)((long*)100 + 1) != 104) {return 1;}

    // unsigned arithmetic single pointers
    if ((int)((unsigned char*)100 + 1) != 101) {return 1;}
    if ((int)((unsigned short*)100 + 1) != 102) {return 1;}
    if ((int)((unsigned int*)100 + 1) != 104) {return 1;}
    if ((int)((unsigned long*)100 + 1) != 104) {return 1;}

    // signed arithmetic double pointers
    if ((int)((char**)100 + 1) != 104) {return 1;}
    if ((int)((short**)100 + 1) != 104) {return 1;}
    if ((int)((int**)100 + 1) != 104) {return 1;}
    if ((int)((long**)100 + 1) != 104) {return 1;}

    // unsigned arithmetic double pointers
    if ((int)((unsigned char**)100 + 1) != 104) {return 1;}
    if ((int)((unsigned short**)100 + 1) != 104) {return 1;}
    if ((int)((unsigned int**)100 + 1) != 104) {return 1;}
    if ((int)((unsigned long**)100 + 1) != 104) {return 1;}

    return 0;
}
