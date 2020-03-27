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

#include <cctype>
#include <fstream>
#include <string>
#include "tclib.hpp"

using namespace std;

bool Tclib::parse_cond(const std::string& src, std::list<cond_elem_t>& dst) {
    // TODO: save polish expr from the beginning?
    int i = 0;
    while (true) {
        if (src[i] == '_' || isalpha(src[i])) {
            std::string var, val;
            var.assign(1, src[i++]);
            while (src[i] == '_' || isalnum(src[i]))
                var.append(1, src[i++]);
            dst.emplace_back();
            try {
                dst.back().type = 'L';
                dst.back().index = var_lut.at(var);
            }
            catch (std::out_of_range) {
                return false;
            }
            while (src[i] == ' ')
                i++;
            if (src[i] == '=' && src[i + 1] == '=') {
                dst.emplace_back();
                dst.back().type = '=';
                i += 2;
            }
            else if (src[i] == '!' && src[i + 1] == '=') {
                dst.emplace_back();
                dst.back().type = '~';
                i += 2;
            }
            else
                return false;
            while (src[i] == ' ')
                i++;
            if (src[i] == '_' || isalnum(src[i])) {
                val.assign(1, src[i++]);
                while (src[i] == '_' || isalnum(src[i]))
                    val.append(1, src[i++]);
                dst.emplace_back();
                try {
                    dst.back().type = 'R';
                    dst.back().index = var_vec[var_lut.at(var)].val_lut.at(val);
                }
                catch (std::out_of_range) {
                    return false;
                }
            }
            else
                return false;
        }
        else if (src[i] == '|') {
            if (src[++i] != '|') {
                return false;
            }
            dst.emplace_back();
            dst.back().type = '|';
            i++;
        }
        else if (src[i] == '&') {
            if (src[++i] != '&') {
                return false;
            }
            dst.emplace_back();
            dst.back().type = '&';
            i++;
        }
        else if (src[i] == '!') {
            dst.emplace_back();
            dst.back().type = '!';
            i++;
        }
        else if (src[i] == '(') {
            dst.emplace_back();
            dst.back().type = '(';
            i++;
        }
        else if (src[i] == ')') {
            dst.emplace_back();
            dst.back().type = ')';
            i++;
        }
        else if (src[i] == '0') {
            dst.emplace_back();
            dst.back().type = '0';
            i++;
        }
        else if (src[i] == '1') {
            dst.emplace_back();
            dst.back().type = '1';
            i++;
        }
        else if (src[i] == '\0') {
            return true;
        }
        else if (src[i] == ' ') {
            i++;
        }
        else
            return false;
    }
    return false; // should never reach this
}

static bool retrieve_elem(const string& line, string& elem, string& tag) {
    size_t i = 0;
    elem.clear();
    tag.clear();
    if (line.length() == 0 || (line[0] != '_' && !isalnum(line[0]))) {
        return false;
    }
    // derive var name
    elem.assign(1, line[0]);
    for (i = 1; i < line.length(); i++) {
        if (line[i] == '_' || isalnum(line[i])) {
            elem.append(1, line[i]);
        }
        else if (line[i] == ':' || line[i] == ' ') {
            break;
        }
        else {
            return false;
        }
    }
    if (i >= line.length()) {
        return true;
    }
    // skip whitespaces (if any) following var name
    for (; i < line.length(); i++) {
        if (line[i] != ' ')
            break;
    }
    if (i >= line.length()) {
        return true;
    }
    // check for colon
    if (line[i] == ':') {
        // store tag
        tag.clear();
        for (i += 1; i < line.length(); i++) {
            // exit upon comment mark
            if (line[i] == '#') {
                break;
            }
            else{
                tag.append(1, line[i]);
            }
        }
        // strip heading/ending whitespace
        tag.erase(0, tag.find_first_not_of(' '));
        tag.erase(tag.find_last_not_of(' ') + 1);
    }
    else if (line[i] != '#') {
        return false;
    }
    return true;
}

static bool is_empty_line(const string& line) {
    // empty line or comment
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '#') {
            break;
        }
        else if (line[i] != ' ') {
            return false;
        }
    }
    return true;
}

// yaml-like data descriptor is supported
bool Tclib::load(const string& file_name) {
    load_log.clear();

    ifstream ifs(file_name);
    if (!ifs.is_open()){
        load_log.append("[LOAD ERROR] Cannot open ");
        load_log.append(file_name);
        load_log.append(1, '\n');
        return false;
    }

    string line;
    int line_num = 0;
    while (getline(ifs, line)){
        line_num++;
        // remove heading whitespace
        line.erase(0, line.find_first_not_of(' '));

        // skip empty lines
        if (is_empty_line(line)) {
            continue;
        }

        // search for var
        else if (line[0] == '_' || isalpha(line[0])) {
            string var, tag;
            bool status = retrieve_elem(line, var, tag);
            if (!status) {
                load_log.append("[LOAD ERROR] Cannot retrieve variable @line ").append(to_string(line_num)).append(": \n");
                load_log.append(line);
                load_log.append(1, '\n');
                return false;
            }
            // err if previous var has no val
            if (var_vec.size() && var_vec.back().size() == 0) {
                load_log.append("[LOAD ERROR] Variable w/o possible value: ");
                load_log.append(var_vec.back());
                load_log.append(1, '\n');
                return false;
            }
            // err if duplicate vars found
            if (var_lut.find(var) != var_lut.end()) {
                load_log.append("[LOAD ERROR] Duplicate variable @line ").append(to_string(line_num)).append(": \n");
                load_log.append(line);
                load_log.append(1, '\n');
                return false;
            }
            // insert var
            var_vec.emplace_back();
            var_vec.back().assign(var);
            var_vec.back().var_tag.assign(tag);
            var_lut[var] = (int)var_vec.size() - 1;
        }
        // search for val
        else if (line[0] == '-') {
            // remove heading '-' and following whitespace
            line.erase(0, line.find_first_not_of("- "));
            string val, tag;
            bool status = retrieve_elem(line, val, tag);
            if (!status) {
                load_log.append("[LOAD ERROR] Cannot retrieve value @line ").append(to_string(line_num)).append(": \n");
                load_log.append(line);
                load_log.append(1, '\n');
                return false;
            }
            // err if no previous var created
            if (var_vec.size() == 0) {
                load_log.append("[LOAD ERROR] Value w/o matching variable @line ").append(to_string(line_num)).append(": \n");
                load_log.append(line);
                load_log.append(1, '\n');
                return false;
            }
            // ignore duplicate val
            if (var_vec.back().val_lut.find(val) != var_vec.back().val_lut.end()) {
                load_log.append("[LOAD WARNING] Ignored duplicate value @line ").append(to_string(line_num)).append(": \n");
                load_log.append(line);
                load_log.append(1, '\n');
                continue;
            }
            // insert value
            var_vec.back().val_vec.emplace_back();
            var_vec.back().val_vec.back().assign(val);
            var_vec.back().val_vec.back().val_tag.assign(tag);
            var_vec.back().val_lut[val] = (int)var_vec.back().val_vec.size() - 1;
        }
        // error
        else {
            load_log.append("[LOAD ERROR] Cannot recognize @line ").append(to_string(line_num)).append(": \n");
            load_log.append(line);
            load_log.append(1, '\n');
            return false;
        }
    }

    //parse dependencies/constraints
    for (size_t var_i = 0; var_i < var_vec.size(); var_i++) {
        if (var_vec[var_i].tag.length()) {
            if (!parse_cond(var_vec[var_i].tag, var_vec[var_i].cond)) {
                load_log.append("[LOAD ERROR] Invalid dependency: ");
                load_log.append(var_vec[var_i].tag);
                load_log.append(1, '\n');
                return false;
            }
        }
        for (size_t val_i = 0; val_i < var_vec[var_i].val_vec.size(); val_i++) {
            if (var_vec[var_i][val_i].tag.length()) {
                if (!parse_cond(var_vec[var_i][val_i].tag, var_vec[var_i].val_vec[val_i].cond)) {
                    load_log.append("[LOAD ERROR] Invalid constraint: ");
                    load_log.append(var_vec[var_i][val_i].tag);
                    load_log.append(1, '\n');
                    return false;
                }
            }
        }
    }

    return true;
}
