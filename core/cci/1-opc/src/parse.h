#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

void parse_init(void);
void parse_destroy(void);

/**
 * Parses the entire input, compiling it to the output.
 */
void parse(void);

#endif
