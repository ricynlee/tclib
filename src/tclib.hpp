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

#ifndef _TCLIB_H_
#define _TCLIB_H_

#include <list>
#include <map>
#include <vector>
#include <stdexcept>
#include <cctype>

typedef struct{
    int index; // var/val
    char type; // = ~ & | ! ( ) L R 1 0 ?
    // =: equal
    // ~: not equal
    // &: logical and
    // |: logical or
    // !: logical not
    // (: left bracket
    // ): right bracket
    // L: left item of =
    // R: right item of =
    // 1: logical true
    // 0: logical false
    // ?: logical uncertain, unacceptable in input
}cond_elem_t;

class TclibVal:public std::string{
private:
    std::list<cond_elem_t>  cond;
public:
    friend class Tclib;
public:
    TclibVal():tag(val_tag){
        cond.clear();
    }
    const std::list<cond_elem_t>& constraint(void)const {
        return cond;
    }
private:
    std::string        val_tag;
public:
    const std::string& tag;
};

class TclibVar:public std::string{
private:
    std::list<cond_elem_t>      cond;
    std::map<std::string, int>  val_lut;
    std::vector<TclibVal>       val_vec;
public:
    friend class Tclib;
public:
    TclibVar():tag(var_tag) {
        cond.clear();
        val_lut.clear();
        val_vec.clear();
        val_vec.reserve(16);
    }
    const std::list<cond_elem_t>& dependency(void)const {
        return cond;
    }
    const TclibVal& operator[](size_t index)const {
        if (index>val_vec.size())
            throw std::out_of_range("cannot find val");
        return val_vec[index];
    }
    size_t size()const {
        return val_vec.size();
    }
private:
    std::string        var_tag;
public:
    const std::string& tag;
};

class Tclib{
private:
    std::vector<TclibVar>       var_vec;
    std::map<std::string, int>  var_lut;
    std::string                 load_log;
public:
    Tclib():log(load_log) {
        var_lut.clear();
        var_vec.clear();
        var_vec.reserve(32);
        load_log.clear();
    }
    const TclibVar& operator[](size_t index)const {
        if (index>var_vec.size())
            throw std::out_of_range("cannot find var");
        return var_vec[index];
    }
    size_t size()const {
        return var_vec.size();
    }
private:
    bool parse_cond(const std::string& src, std::list<cond_elem_t>& dst);
public:
    const std::string& log;
    bool load(const std::string& file_name);
};

#endif // _TCLIB_H_
