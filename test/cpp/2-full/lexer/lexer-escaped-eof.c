// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This backslash is at the very end of the file. It is not followed by a line
// feed.
// GCC, Clang and TinyCC (correctly, I believe) emit it as backslash.
// MSVC and chibicc remove it.
// Kefir diagnoses an error in the preprocessor tokenizer.
\