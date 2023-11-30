#ifndef __UTILS_H__
#define __UTILS_H__
#include <iostream>
#include <cstring>
#include <algorithm>

std::string normalizeText(const std::string& text);

std::string toUpperCase(const std::string& input);

std::string toLowerCase(const std::string& input);

const std::string getDate();

const char *get_arg_value(int argc, char **argv, const char *target_arg);

void printCmdHelp();

#endif