#include "utils.hpp"

std::string normalizeText(const std::string& text) {
    std::string result = text;
    std::string replacement = "\\n";

    size_t found = result.find("\n");
    while (found != std::string::npos) {
        result.replace(found, 1, replacement);
        found = result.find("\n", found + replacement.length());
    }

    found = result.find("\"");
    while (found != std::string::npos) {
        result.replace(found, 1, "\\\"");
        found = result.find("\"", found + replacement.length());
    }

    found = result.find('\t');
    while (found != std::string::npos) {
        result.replace(found, 1, "\\t");  // Escapa la tabulación
        found = result.find('\t', found + 2);
    }

    return result;
}

std::string toUpperCase(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string toLowerCase(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

const std::string getDate() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", &tstruct);

    return buf;
}

const char *get_arg_value(int argc, char **argv, const char *target_arg){
    for(int arg_idx = 0; arg_idx < argc; arg_idx++){
        if(!strcmp(argv[arg_idx], target_arg)) // <arg> <value> 
            return argv[arg_idx+1]==nullptr?"":argv[arg_idx+1];
    }
    return nullptr;
}

void printCmdHelp(){
    std::cout << "Usage:\n \
    --my-prompt <my_prompt_name> (default: default)\n \
    --param-profile <profile_name> (default: samantha)\n \
    --prompt-template <template_name> (default: empty)\n \
    --no-chat-guards (default: true)\n \
    --ip <ip address> (default: 127.0.0.1)\n \
    --port <port> (default: 8080)" << std::endl;
}