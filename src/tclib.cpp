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

// database compatible layer
#include "yamlite.hpp"
#include "tclib.hpp"

using namespace std;

bool Tclib::open(const string& file) {
    Yamlite yamlite;

    if (!yamlite.load(file))
        return false;
    for (int key_i = 0; key_i < yamlite.key_num(); key_i++) {
        var_vec.emplace_back();
        if (yamlite.get_key(key_i)[0] != '_' && !isalpha(yamlite.get_key(key_i)[0]))
            return false;
        var_vec.back().assign(yamlite.get_key(key_i));
        var_lut[var_vec.back()] = key_i;
        for (int value_i = 0; value_i < yamlite.get_key(key_i).value_num(); value_i++) {
            var_vec.back().val_vec.emplace_back();
            var_vec.back().val_vec.back().assign(yamlite.get_key(key_i).get_value(value_i));
            var_vec.back().val_lut[var_vec.back().val_vec.back()] = value_i;
        }
    }
    for (int key_i = 0; key_i < yamlite.key_num(); key_i++) {
        if (yamlite.get_key(key_i).get_key_tag().length()) {
            if (!parse_cond(yamlite.get_key(key_i).get_key_tag(), var_vec[key_i].cond)) {
                Tclib();
                return false;
            }
        }
        for (int value_i = 0; value_i < yamlite.get_key(key_i).value_num(); value_i++) {
            if (yamlite.get_key(key_i).get_value(value_i).get_value_tag().length()) {
                if (!parse_cond(yamlite.get_key(key_i).get_value(value_i).get_value_tag(), var_vec[key_i].val_vec[value_i].cond)) {
                    Tclib();
                    return false;
                }
            }
        }
    }
    return true;
}
