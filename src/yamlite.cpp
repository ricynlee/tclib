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

#include <fstream>
#include <string>
#include <regex>
#include "yamlite.hpp"

using namespace std;

const string& YamlValue::get_value_tag(void) const{
    return (const string&)value_tag;
}

YamlKey::YamlKey(size_t reserved_length){
    value_vec.reserve(reserved_length);
}

size_t YamlKey::find_value(const string& value_string) const{
    for(size_t i = 0; i < value_vec.size(); i++){
        if(value_vec[i]==value_string){
            return i;
        }
    }
    return YAML_INVALID_INDEX;
}

const YamlValue& YamlKey::get_value(size_t index) const{
    return value_vec[index];
}

const string& YamlKey::get_key_tag(void) const{
    return (const string&)key_tag;
}

size_t YamlKey::value_num() const{
    return value_vec.size();
}

YamlValue& YamlKey::append_value(const string& value, const string& value_tag){
    value_vec.emplace_back();
    value_vec.back().assign(value);
    value_vec.back().value_tag.assign(value_tag);
    return value_vec.back();
}

Yamlite::Yamlite(const std::string& filename, size_t reserved_length){
    key_vec.reserve(reserved_length);
    if(filename.size()) // !empty
        load(filename);
}

YamlKey& Yamlite::append_key(const string& name, const string& key_tag){
    key_vec.emplace_back();
    key_vec.back().assign(name);
    key_vec.back().key_tag.assign(key_tag);
    return key_vec.back();
}

const YamlKey& Yamlite::get_key(size_t index) const{
    return key_vec[index];
}

size_t Yamlite::find_key(const string& key_string) const{
    for(size_t i = 0; i < key_vec.size(); i++){
        if(key_vec[i]==key_string){
            return i;
        }
    }
    return YAML_INVALID_INDEX;
}

size_t Yamlite::key_num() const{
    return key_vec.size();
}

bool Yamlite::load(const string& filename){
    key_vec.clear();
    ifstream ifs(filename);
    if(!ifs.is_open())
        return false;
    string line;
    smatch group;
    while(getline(ifs, line)){
        if(regex_match(line, group, regex("^(\\w+) *\\: *(.*?) *$"))){ // Key
            if(key_vec.size() && key_vec.back().value_num()==0){
                // empty-valued option
                key_vec.clear();
                return false;
            }

            size_t key_index = find_key(group[1].str());
            if(key_index != YAML_INVALID_INDEX){
                // fail on duplicate key
                key_vec.clear();
                return false;
            }

            append_key(group[1].str(), group[2].str());
        } else if(regex_match(line, group, regex("^ *- *(\\w+) *$")) ||
                  regex_match(line, group, regex("^ *- *(\\w+) *\\: *(.*?) *$"))){ // Value
            if(key_vec.size()==0){
                // floating value
                key_vec.clear();
                return false;
            }

            if(key_vec.back().find_value(group[1].str()) != YAML_INVALID_INDEX)
                // ignore duplicate value
                continue;
            key_vec.back().append_value(group[1].str(), group.size()>2 ? group[2].str() : "");
        } else if(regex_match(line, group, regex("^ *(#.*)?$"))){
            // skip empty lines
        } else {
            // unrecognized line
            key_vec.clear();
            return false;
        }
    }
    if(key_vec.size()==0 || key_vec.back().value_num()==0){
        // empty-optioned yaml, or the last option is empty-valued
        key_vec.clear();
        return false;
    }
    return true;
}
