#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <map>

#include "utils.cpp"
#include "sjson.cpp"
#include "minipost.hpp"
#include "colors.h"
#include "terminal.hpp"

#ifdef __WIN32__
    #include <windows.h>
#endif

#include <ctime>

using namespace std;

bool stopCompletionFlag = false;
bool completionStream = true;
std::string completionBuffer;

//////////////////////////////////////////////

struct parameters_t {
    int     mirostat = 0;
    int     mirostat_tau = 5;
    float   mirostat_eta = 0.1;
    int     frecuency_penalty = 0;
    int     n_probs = 0;
    int     presence_penalty = 0;
    int     top_k = 40;
    float   top_p = 0.5;
    int     typical_p = 1;
    int     tfz_z = 1;
    int     repeat_last_n = 256;
    float   repeat_penalty = 1.18;
    int     slot_id = -1;
    float   temperature = 0.8;
    int     n_predict = 100;
    bool    stream = true;
    std::vector<std::string> stop;
    std::string grammar = "";
    std::string prompt;
};

struct prompt_style_t {
    std::string prompt_type;

    std::string begin_system;
    std::string end_system;

    std::string begin_user;
    std::string end_user;

    std::string begin_assistant;
    std::string eos;
};

struct actor_t {
    std::string alias;
    std::string name;
    std::string tag_color;
    std::string role;
};

struct chat_entry_t {
    actor_t *actor;
    std::string message_content;
};

struct actor_list_t {
    std::string actor_name;
    std::string actor_color;
};

struct ChatContext {
    std::string system_prompt;
    std::map<std::string, actor_t> actor_list;
};