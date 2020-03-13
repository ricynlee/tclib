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
#include <map>
#include "tclib-engine.hpp"
using namespace std;

// (-1)=>n:(i+n+1)%(n+1)
// n=>(-1):(i+1)%(n+1)-1

TclibEnumEngine::cond_t TclibEnumEngine::judge(list<cond_elem_t> expr){
    if(expr.empty())
        return COND_TRUE;

    for (list<cond_elem_t>::iterator it = next(expr.begin()); it != expr.end() && next(it) != expr.end(); it++) {
        if (it->type == '=') {
            if (prev(it)->type != 'L' || next(it)->type != 'R') {
                return COND_ERROR;
            }
            if (prev(it)->index >= (int)var.size()) {
                it->type = '?';
            }
            else {
                it->index = (var[prev(it)->index] == next(it)->index);
                it->type = it->index ? '1' : '0';
            }
            expr.erase(prev(it));
            expr.erase(next(it));
        }
        else if (it->type == '~') {
            if (prev(it)->type != 'L' || next(it)->type != 'R') {
                return COND_ERROR;
            }
            if (prev(it)->index >= (int)var.size()) {
                it->type = '?';
            }
            else {
                it->index = (var[prev(it)->index] != next(it)->index);
                it->type = it->index ? '1' : '0';
            }
            expr.erase(prev(it));
            expr.erase(next(it));
        }
    }

    // generate polish expr
    queue<char> q;
    stack<char> s;
    for (list<cond_elem_t>::reverse_iterator it = expr.rbegin(); it != expr.rend(); it++) {
        if(it->type=='1' || it->type=='0' || it->type=='?'){
            q.push(it->type);
        }else if(it->type ==')'){
            s.push(it->type);
        }else if(it->type=='('){
            while(s.size() && s.top()!=')'){
                q.push(s.top());
                s.pop();
            }
            if(s.empty()){ // no matching bracket
                return COND_ERROR;
            }
            s.pop(); // ')'
        }else if(it->type=='!'){
            s.push(it->type);
        }else if(it->type=='&'){
            while(s.size() && s.top()=='!'){
                q.push(s.top());
                s.pop();
            }
            s.push(it->type);
        }else if(it->type=='|'){
            while(s.size() && (s.top()=='!' || s.top()=='&')){
                q.push(s.top());
                s.pop();
            }
            s.push(it->type);
        }else if(it->type!=' '){
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

bool TclibEnumEngine::enumerate(tc_callback tc_handler){
    if(tclib.size()==0){
        enumerate_log.append("[ENUM ERROR] Empty test case library");
        enumerate_log.append(1, '\n');
        return false;
    }

    map<string, string> tc_dict;

    // configure root
    TclibEnumNode n;
    n.parent_var_index = INVALID_INDEX;
    n.parent_val_index = INVALID_INDEX;

    // initialization for traverse
    size_t i;
    stack<TclibEnumNode> lifo;
    size_t current_var_index;
    lifo.push(n);

    // initialization for vars/conditions
    var.clear();
    var.reserve(tclib.size());
    cond.clear();
    cond.reserve(tclib.size());
    cond_t judge_result=COND_ERROR;
    list<cond_elem_t> tmp_cond;
    cond_elem_t tmp_cond_elem;

    // traverse
    while(lifo.size()){
        n=lifo.top(); lifo.pop();
        current_var_index = n.parent_var_index+1;

        // update & process vars/conditions
        if(n.parent_var_index != INVALID_INDEX){
            tmp_cond.clear();
            if(var.size()<current_var_index){
                var.emplace_back();
                cond.emplace_back();
            }else while(var.size()>current_var_index){
                var.pop_back();
                cond.pop_back();
            }
            var.back() = n.parent_val_index;
            if(n.parent_val_index != INVALID_INDEX){
                if(tclib[n.parent_var_index][n.parent_val_index].constraint().size()){
                    tmp_cond_elem.type = '(';
                    tmp_cond.push_back(tmp_cond_elem);
                    tmp_cond.insert(tmp_cond.end(), tclib[n.parent_var_index][n.parent_val_index].constraint().begin(), tclib[n.parent_var_index][n.parent_val_index].constraint().end());
                    tmp_cond_elem.type = ')';
                    tmp_cond.push_back(tmp_cond_elem);
                }
                if(tmp_cond.size() && tclib[n.parent_var_index].dependency().size()){
                    tmp_cond_elem.type = '&';
                    tmp_cond.push_back(tmp_cond_elem);
                }
                if(tclib[n.parent_var_index].dependency().size()){
                    tmp_cond_elem.type = '(';
                    tmp_cond.push_back(tmp_cond_elem);
                    tmp_cond.insert(tmp_cond.end(), tclib[n.parent_var_index].dependency().begin(), tclib[n.parent_var_index].dependency().end());
                    tmp_cond_elem.type = ')';
                    tmp_cond.push_back(tmp_cond_elem);
                }
            }else{ // nil node
                tmp_cond_elem.type = '!';
                tmp_cond.push_back(tmp_cond_elem);
                tmp_cond_elem.type = '(';
                tmp_cond.push_back(tmp_cond_elem);
                tmp_cond.insert(tmp_cond.end(), tclib[n.parent_var_index].dependency().begin(), tclib[n.parent_var_index].dependency().end());
                tmp_cond_elem.type = ')';
                tmp_cond.push_back(tmp_cond_elem);
            }
            cond.back() = tmp_cond;

            judge_result = judge(cond.back());
            if(judge_result==COND_FALSE){
                continue;
            }else if(judge_result==COND_ERROR){
                enumerate_log.append("[ENUM ERROR] There are logical error(s) with a dependency/constraint.\n");
                enumerate_log.append("Check ").append(tclib[n.parent_var_index]).append(": ").append(tclib[n.parent_var_index].tag).append(1, '\n');
                enumerate_log.append("Check ").append(tclib[n.parent_var_index][n.parent_val_index]).append(": ").append(tclib[n.parent_var_index][n.parent_val_index]).append(1, '\n');
                break;
            }else{ // TRUE or UNCERTAIN
                if(judge_result==COND_TRUE){
                    cond.back().clear();
                }else{ // UNCERTAIN
                    // TODO: store a simplified expression?
                }
                for(i=0; i<cond.size()-1; i++){
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
        if(n.parent_var_index != INVALID_INDEX){
            if(n.parent_val_index != INVALID_INDEX){
                tc_dict[tclib[n.parent_var_index]] = tclib[n.parent_var_index][n.parent_val_index];
            }else if(tc_dict.find(tclib[n.parent_var_index])!=tc_dict.end()){
                // remove var from dict as (n.parent_val_index != INVALID_INDEX)
                tc_dict.erase(tclib[n.parent_var_index]);
            }
            if(current_var_index==tclib.size() && tc_dict.size()){
                if(tc_handler != nullptr){
                    tc_handler(tc_dict);
                }
            }
        }

        // add sub-nodes
        if(current_var_index == tclib.size())
            continue;

        for(i=tclib[current_var_index].size()-(tclib[current_var_index].dependency().empty() ? 1 : 0); i!=INVALID_INDEX; i--){ // Add an extra value
            lifo.emplace();
            lifo.top().parent_var_index = current_var_index;
            lifo.top().parent_val_index = (i + 1) % (tclib[current_var_index].size() + 1) - 1;
        }
    }

    // return
    if(judge_result == COND_ERROR)
        return false;
    else
        return true;
}
