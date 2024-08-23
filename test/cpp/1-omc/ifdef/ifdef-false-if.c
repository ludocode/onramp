// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#ifndef FOO
    #ifdef BAR
        // all of this should be ignored
        #if @#@#$%@$#%@^&
            #if $!$!$!$!
                #if ----
                    error error
                #endif
            #endif
        #endif
        #define BAZ(x) $$$$$$
        #if 0
            #if 1
                #if
                #endif
            #endif
        #endif
    #endif
#endif

int main(void) {
}
