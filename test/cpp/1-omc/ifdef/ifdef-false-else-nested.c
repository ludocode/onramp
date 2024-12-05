// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    #ifdef UNDEFINED
        #error
        #ifdef NESTED
            #error
        #elif NESTED_ELIF
            // nested #elif is allowed
            #error
        #elifdef NESTED_ELIFDEF
            // nested #elifdef is allowed
            #error
        #elifndef NESTED_ELIFNDEF
            // nested #elifndef is allowed
            #error
        #else
            // nested #else is allowed
            #error
        #endif
    #endif
}
