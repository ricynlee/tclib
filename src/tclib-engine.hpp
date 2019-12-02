/*
MIT License

Copyright (c) 2019 ricynlee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _TCLIB_ENG_H_
#define _TCLIB_ENG_H_

#include <vector>
#include <string>
#include <map>
#include "tclib.hpp"

#define INVALID_INDEX ((size_t)(-1))

class TclibEnumNode{
private:
    size_t        parent_var_index;
    size_t        parent_val_index;
public:
    friend class TclibEnumEngine;
};

typedef void (*tc_callback)(const std::map<std::string, std::string>& tc_dict);

class TclibEnumEngine{
private:
    typedef enum{
        COND_FALSE = 0,
        COND_TRUE = 1,
        COND_UNCERTAIN = 2,
        COND_ERROR = (-1),
    } cond_t;
private:
    Tclib& tclib;
    std::vector<size_t>                 var;
    std::vector<std::list<cond_elem_t>> cond;
    cond_t judge(std::list<cond_elem_t> expr);
public:
    TclibEnumEngine(Tclib& lib) :tclib(lib) {
        var.clear();
        var.reserve(lib.size());
        cond.clear();
    }
    bool enumerate(tc_callback tc_handler=nullptr);
};

#endif // _TCLIB_ENG_H_
