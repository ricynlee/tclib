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

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "tclib.hpp"
#include "tclib-engine.hpp"
using namespace std;

ostream* out = &cout;
void tc_print(const map<string, string>& tc_dict){

    (*out) << tc_dict.cbegin()->first;
    (*out) << '=';
    (*out) << tc_dict.cbegin()->second;

    for(map<string, string>::const_iterator it=next(tc_dict.cbegin()); it!=tc_dict.cend(); it++){
        (*out) << ',';
        (*out) << it->first;
        (*out) << '=';
        (*out) << it->second;
    }

    (*out) << '\n';
}

void help(void){
    cout<<
    "tclib (C) 2019~2020 ricynlee" "\n"
    "Built on " __DATE__ " " __TIME__ "\n"
    "\n"
    "# Usage" "\n"
    "tclib yamlite-in [testcases-out]" "\n"
    <<endl;
}

int main(int argc, char* argv[]){
    int status = 0;
    ofstream ofs;

    do{
        if(argc<2 || argc>3){
            help();
            status = 0;
            break;
        }

        if(argc==3){
            ofs.open(argv[2]);
            if(ofs.is_open())
                out = &ofs;
            else
                cerr<<"[WARNING] Failed to access output file "<<'\"'<<argv[2]<<'\"'<<". Output is redirected to stdout."<<endl;
        }

        Tclib lib;
        if(!lib.load(argv[1])){
            cerr << "[ERROR] Failed to load input file " << '\"' << argv[1] << '\"' << endl;
            cerr << lib.log;
            status = (-1);
            break;
        }
        TclibEnumEngine eng(lib);
        if(!eng.enumerate(tc_print)){
            cerr << "[ERROR] Failed to generate test cases out of file " << '\"' << argv[1] << '\"' << endl;
            cerr << eng.log;
            status = (-1);
            break;
        }
    }while(0);

    if(ofs.is_open())
        ofs.close();
    out = nullptr;

    return status;
}
