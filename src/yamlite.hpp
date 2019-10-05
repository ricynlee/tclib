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

#ifndef _YAMLITE_H_
#define _YAMLITE_H_

#include <vector>
#include <string>

#define YAML_INVALID_INDEX ((size_t)(-1))

class YamlValue: public std::string{
private:
    std::string value_tag;
public:
    const std::string& get_value_tag(void) const;
public:
    friend class YamlKey;
};

class YamlKey: public std::string{
private:
    std::vector<YamlValue>   value_vec;
    YamlValue& append_value(const std::string& value, const std::string& value_tag);
    std::string  key_tag;
public:
    YamlKey(size_t reserved_length=16);
    size_t find_value(const std::string& value_string) const;
    const YamlValue& get_value(size_t index) const;
    const std::string& get_key_tag(void) const;
    size_t value_num() const;
public:
    friend class Yamlite;
};

class Yamlite{
private:
    std::vector<YamlKey> key_vec;
    YamlKey& append_key(const std::string& name, const std::string& key_tag);
public:
    Yamlite(const std::string& filename=std::string(""), size_t reserved_length=16);
    bool load(const std::string& filename);
    size_t find_key(const std::string& key_string) const;
    const YamlKey& get_key(size_t index) const;
    size_t key_num() const;
};

#endif // _YAMLITE_H_
