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

#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <regex>
#include <map>
#include "yamlite.hpp"
#include "tclib.hpp"
using namespace std;

#include <iostream>
// (-1)=>n:(i+n+1)%(n+1)
// n=>(-1):(i+1)%(n+1)-1

Node::~Node(){
    for(size_t i=0; i<node_vec.size(); i++)
        if(node_vec[i] != nullptr)
           delete node_vec[i];
}

Library::COND Library::judge(const string& expr){
    if(expr.empty())
        return COND_TRUE;

    // filter out '?' - valid internally only
    string e(expr);
    if(e.find('?')!=string::npos)
        return COND_ERROR;

    // replace elements
    smatch group;
    size_t option_index, value_index;
    while(regex_search(e, group, regex(" *(\\w+) *(\\!=|==) *(\\w+) *"))){
        option_index = yaml.find_key(group[1].str());
        value_index = yaml.get_key(yaml.find_key(group[1].str())).find_value(group[3].str());
        if(option_index==YAML_INVALID_INDEX || value_index==YAML_INVALID_INDEX){
            return COND_ERROR;
        } else if(option_index>=var.size()){
            e.replace(group.position(0), group.length(0), string(1, '?')); // uncertain
        } else if(group[2].str()==string("==")){
            e.replace(group.position(0), group.length(0), string(1, var[option_index]==value_index ? '1' : '0'));
        } else if(group[2].str()==string("!=")){
            e.replace(group.position(0), group.length(0), string(1, var[option_index]==value_index ? '0' : '1'));
        }
    }
    while(regex_search(e, group, regex("&&"))){
        e.replace(group.position(0), group.length(0), string(1, '&'));
    }
    while(regex_search(e, group, regex("\\|\\|"))){
        e.replace(group.position(0), group.length(0), string(1, '|'));
    }

    // generate polish expr
    queue<char> q;
    stack<char> s;
    for(int i=(int)e.size()-1; i>=0; i--){
        if(e[i]=='1' || e[i]=='0' || e[i]=='?'){
            q.push(e[i]);
        }else if(e[i]==')'){
            s.push(e[i]);
        }else if(e[i]=='('){
            while(s.size() && s.top()!=')'){
                q.push(s.top());
                s.pop();
            }
            if(s.empty()){ // no matching bracket
                return COND_ERROR;
            }
            s.pop(); // ')'
        }else if(e[i]=='!'){
            s.push(e[i]);
        }else if(e[i]=='&'){
            while(s.size() && s.top()=='!'){
                q.push(s.top());
                s.pop();
            }
            s.push(e[i]);
        }else if(e[i]=='|'){
            while(s.size() && (s.top()=='!' || s.top()=='&')){
                q.push(s.top());
                s.pop();
            }
            s.push(e[i]);
        }else if(e[i]!=' '){
            return COND_ERROR;
        }
    }
    while(s.size()){
        q.push(s.top());
        s.pop();
    }

    // calc polish expr
    char a, b;
    while(q.size()){
        if(q.front()=='0' || q.front()=='1' || q.front()=='?'){
            s.push(q.front());
        }else if(q.front()=='&'){
            if(s.empty())
                return COND_ERROR;
            a = s.top(); s.pop();
            if(s.empty())
                return COND_ERROR;
            b = s.top(); s.pop();
            if(a=='0' || b=='0') // 00 01 10 ?0 0?
                s.push('0');
            else if(a=='?' || b=='?') // ?? ?1 1?
                s.push('?');
            else // 11
                s.push('1');
        }else if(q.front()=='|'){
            if(s.empty())
                return COND_ERROR;
            a = s.top(); s.pop();
            if(s.empty())
                return COND_ERROR;
            b = s.top(); s.pop();
            if(a=='1' || b=='1') // 11 10 01 1u u1
                s.push('1');
            else if(a=='?' || b=='?') // uu u0 0u
                s.push('?');
            else // 00
                s.push('0');
        }else if(q.front()=='!'){
            if(s.empty())
                return COND_ERROR;
            a = s.top(); s.pop();
            if(a=='?')
                s.push('?');
            else // invert bit 0
                s.push(a^0x1);
        }
        q.pop();
    }
    if(s.size()!=1)
        return COND_ERROR;

    // result
    if(s.top()=='1')
        return COND_TRUE;
    else if(s.top()=='0')
        return COND_FALSE;
    else
        return COND_UNCERTAIN;
}

bool Library::enumerate(tc_callback tc_handler){
    if(yaml.key_num()==0){
        return false;
    }

    map<string, string> tc_dict;

    // configure root
    Node* root_p = new Node;
    root_p->parent_option_index = INVALID_INDEX;
    root_p->parent_value_index = INVALID_INDEX;

    // initialization for traverse
    size_t i;
    stack<Node*> lifo;
    size_t current_option_index;
    lifo.push(root_p);

    // initialization for vars/conditions
    var.clear();
    var.reserve(yaml.key_num());
    cond.clear();
    cond.reserve(yaml.key_num());
    COND judge_result;
    string tmp_tag;

    // traverse
    Node* p;
    while(lifo.size()){
        p=lifo.top(); lifo.pop();
        current_option_index = p->parent_option_index+1;

        // update & process vars/conditions
        if(p->parent_option_index != INVALID_INDEX){
            tmp_tag.clear();
            if(var.size()<current_option_index){
                var.emplace_back();
                cond.emplace_back();
            }else while(var.size()>current_option_index){
                var.pop_back();
                cond.pop_back();
            }
            var.back() = p->parent_value_index;
            if(p->parent_value_index != INVALID_INDEX){
                if(yaml.get_key(p->parent_option_index).get_value(p->parent_value_index).get_value_tag().size()){
                    tmp_tag += '(';
                    tmp_tag += yaml.get_key(p->parent_option_index).get_value(p->parent_value_index).get_value_tag();
                    tmp_tag += ')';
                }
                if(tmp_tag.size() && yaml.get_key(p->parent_option_index).get_key_tag().size()){
                    tmp_tag += '&';
                }
                if(yaml.get_key(p->parent_option_index).get_key_tag().size()){
                    tmp_tag += '(';
                    tmp_tag += yaml.get_key(p->parent_option_index).get_key_tag();
                    tmp_tag += ')';
                }
            }else{ // nil node
                tmp_tag += "!(";
                tmp_tag += yaml.get_key(p->parent_option_index).get_key_tag();
                tmp_tag += ')';
            }
            cond.back().assign(tmp_tag);

            judge_result = judge(cond.back());
            if(judge_result==COND_FALSE){
                continue;
            }else if(judge_result==COND_ERROR){
                break;
            }else{ // TRUE or UNCERTAIN
                if(judge_result==COND_TRUE){
                    cond.back().clear();
                }else{ // UNCERTAIN
                    // TODO: store a simplified expression
                }
                for(i=0; i<cond.size()-1; i++){ // TODO: if final result is true, current cond size can be recorded for i to start from in future
                    if(cond[i].size() && judge(cond[i]) == COND_FALSE){
                        judge_result = COND_FALSE;
                        break;
                    }
                }
                if(judge_result==COND_FALSE){
                    continue;
                }
            }
        }

        // output node
        if(p->parent_option_index != INVALID_INDEX){
            if(p->parent_value_index != INVALID_INDEX){
                tc_dict[yaml.get_key(p->parent_option_index)] = yaml.get_key(p->parent_option_index).get_value(p->parent_value_index);
            }else if(tc_dict.find(yaml.get_key(p->parent_option_index))!=tc_dict.end()){
                tc_dict.erase(yaml.get_key(p->parent_option_index));
            }
            if(current_option_index==yaml.key_num()){
                if(tc_handler != nullptr){
                    tc_handler(tc_dict);
                }
            }
        }

        // add sub-nodes
        if(current_option_index == yaml.key_num())
            continue;

        p->node_vec.reserve(yaml.get_key(current_option_index).value_num()+1);
        for(i=yaml.get_key(current_option_index).value_num()-(yaml.get_key(current_option_index).get_key_tag().empty() ? 1 : 0); i!=INVALID_INDEX; i--){ // Add an extra value
            p->node_vec.push_back(new Node);
            p->node_vec.back()->parent_option_index = current_option_index;
            p->node_vec.back()->parent_value_index = (i+1)%(yaml.get_key(current_option_index).value_num()+1)-1;
            lifo.push(p->node_vec.back());
        }
    }

    // release
    delete root_p;

    // return
    if(judge_result == COND_ERROR)
        return false;
    else
        return true;
}
