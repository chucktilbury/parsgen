#ifndef _SCANNER_H_
#define _SCANNER_H_

typedef enum {
    END_OF_INPUT,
    PIPE,
    ONE_OR_MORE,
    ZERO_OR_MORE,
    ZERO_OR_ONE,
    OPAREN,
    CPAREN,
    OCURLY,
    CCURLY,
    NON_TERMINAL,
    TERMINAL_SYMBOL,
    TERMINAL_OPER,
    TERMINAL_NAME,
    TERMINAL,
} token_type_t;

typedef struct {
    token_type_t type;
    const char* text;
    const char* name;
    int line_no;
    int col_no;
} token_t;

void init_scanner(const char* file_name);
void uninit_scanner(void);
token_t* get_token(void);
token_t* consume_token(void);
int post_token_queue(void);
void reset_token_queue(int post);
const char* tok_to_str(token_t*);
const char* tok_type_to_str(token_t*);
const char* get_file_name();
int get_line_no(void);

#endif /* _SCANNER_H_ */
