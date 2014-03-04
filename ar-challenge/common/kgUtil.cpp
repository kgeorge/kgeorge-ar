#include <iostream>
#include <fstream>
#include <strstream>
#include <exception>
#include "kgUtil.h"

using namespace std;
void validate(bool bval, const std::string& msg) {
    if(!bval) {
        throw runtime_error(msg.c_str());
    }
}