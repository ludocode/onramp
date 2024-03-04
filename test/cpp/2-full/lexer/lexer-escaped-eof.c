// This backslash is at the very end of the file. It is not followed by a line
// feed.
// GCC, Clang and TinyCC (correctly, I believe) emit it as backslash.
// MSVC and chibicc remove it.
// Kefir diagnoses an error in the preprocessor tokenizer.
\