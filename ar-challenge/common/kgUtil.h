#if !defined(UTIL_H_)
#define UTIL_H_
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>

//throw runtime_error with msg if bval is false
void validate(bool bval, const std::string& msg);



void check_gl_error(const char *file, int line);
#define CHK_GL_ERR()
#define CHK_GL_ERR2() check_gl_error(__FILE__,__LINE__)

//angle of the corner firmed by  points p0, p1 and p2, in that order
double angle(const cv::Point &p0, const cv::Point &p1, const cv::Point &p2 );

#endif //UTIL_H_
