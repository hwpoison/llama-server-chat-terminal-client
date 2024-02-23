#ifndef __UTILS_H__
#define __UTILS_H__
#include <iostream>
#include <algorithm>
#include <cstring>
#include <any>
#include <map> 

std::string normalizeText(const std::string& text);

std::string toUpperCase(const std::string& input);

std::string toLowerCase(const std::string& input);

const std::string getDate();

const char *get_arg_value(int argc, char **argv, const char *target_arg);

void printCmdHelp();

// a dict like python
template<typename keyT>
class AnyMap {
public:
    template<typename valueT>
    void set(keyT key, valueT value){
        vals[key] = value;
    }
    
    template<typename valueT>
    valueT get(keyT key){
        return std::any_cast<valueT>(vals[key]);
    }

    template<typename Function>
    void forEach(Function func){
        for (auto& pair : vals) {
            func(pair.first, pair.second);
        }
    }

    void clear(){
        vals.clear();
    }

private:
    std::map<keyT, std::any> vals;
};

#endif