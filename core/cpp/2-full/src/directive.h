#ifndef DIRECTIVE_H_INCLUDED
#define DIRECTIVE_H_INCLUDED

struct stream_t;
struct token_t;

void directive_setup(void);
void directive_teardown(void);

/**
 * Handles the current preprocessor directive.
 *
 * Note that this does not handle directives in untaken conditional branches.
 * See directive_skip_branch().
 */
void directive_parse(struct stream_t* stream, struct token_t* token);

#endif
