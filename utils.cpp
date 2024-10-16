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
    --prompt <my_prompt_name> (default: creative)\n \
    --param-profile <profile_name> (default: samantha)\n \
    --prompt-template <template_name> (default: empty)\n \
    --no-chat-guards (default: true)\n \
    --ip <ip address> (default: 127.0.0.1)\n \
    --port <port> (default: 8080)" << std::endl;
}


void printChatHelp() {
    std::cout << "Conversation manipulation:\n \
    /narrator: Lets the narrator generate a narration.\n \
    /director: Switch to director mode input.\n \
    /actor / now: Choose who will talk now. If the actor doesn't exist, it will be created. (e.g., /now Einstein)\n \
    /as: Pretend to be an actor and prompt it. (e.g., /as Einstein)\n \
    /talkto: Talk to a specific character. It will switch the current talking actor. (e.g., /talkto Monica)\n \
    /insert / i: Multiline mode insertion. To finish and submit, write \"EOL\" or \"eol\" and then press enter.\n \
    /retry / r: Retry the last completion.\n \
    /continue: Continue the completion without intervention. (The assistant will continue talking)\n \
    /undolast: Undo only the last completion.\n \
    /undo / u: Undo the last completion and user input.\n\n \
Conversation mode:\n \
    /instruct on/off: Turn on/off chat tags for instruction mode.\n\n \
Conversation saving:\n \
    /save (chatname): Save the chat (without extension).\n \
    /load (chatname): Load a previously saved chat.\n\n \
Manage configurations:\n \
    /redraw: Redraw the chat content.\n \
    /reset: Reset the entire chat.\n \
    /quit / q: Exit the program.\n \
    /lprompt: Print the current prompt.\n \
    /lactors: Print current actors.\n \
    /lparams: Print the current parameters.\n \
    /rparams: Reload the current parameter profile.\n \
    /rtemplate: Reload the current template profile.\n \
    /sparam (parameter profile name): Load and set param profile in runtime from param.json.\n \
    /stemplate (template name): Load and set prompt template in runtime from template.json.\n \
    /sprompt (prompt name): Load and set custom prompt in runtime from prompt.json.\n\n" << std::endl;
}
