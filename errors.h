

#ifndef _ERRORS_H_
#define _ERRORS_H_

void fatal_error(const char* fmt, ...);
void syntax_error(const char* file, int line, const char* fmt, ...);
void misc_error(const char* fmt, ...);
int get_errors(void);

#endif /* _ERRORS_H_ */
