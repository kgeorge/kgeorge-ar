#if !defined(UTIL_H_)
#define UTIL_H_
#include <string>


void validate(bool bval, const std::string& msg);
void check_gl_error(const char *file, int line);
#define CHK_GL_ERR()
#define CHK_GL_ERR2() check_gl_error(__FILE__,__LINE__)
#define CHK_FIRST() void check_gl_error_first(const char *file, int line)


#endif //UTIL_H_
