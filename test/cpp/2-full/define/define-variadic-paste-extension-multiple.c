// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define FOO(x,...) before,x,##__VA_ARGS__,after
FOO(1)
FOO(1,2)
FOO(1,2,3)
FOO(1,(2,3),4)
