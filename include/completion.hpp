#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <iostream>
#include <string>

#include "sjson.hpp"
#include "utils.hpp"
#include "minipost.hpp"
#include "terminal.hpp"
#include <csignal>

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080
#define DEFAULT_COMPLETION_ENDPOINT "/completion"
#define PARAMS_FILENAME "params.json"
#define TEMPLATES_FILENAME "templates.json"

extern bool stopCompletionFlag;
extern bool completionInProgress;

struct prompt_template_t {
    std::string prompt_type;

    std::string bos; // Beging of string
    std::string begin_system;
    std::string end_system;

    std::string begin_user;
    std::string end_user;

    std::string begin_assistant;
    std::string end_assistant;
    std::string eos; // End of string
};


static void completionSignalHandler(int signum) {
    if(completionInProgress){
        stopCompletionFlag =  true;
        signal(SIGINT, SIG_DFL); // reset the signal
    }
}

static bool completionCallback(const std::string &chunk, const CallbackBus *bus) {
    if (stopCompletionFlag) {
        Terminal::setTitle("Completion interrupted by user");
        stopCompletionFlag = false;
        completionInProgress = false;
        return false;
    }

    // response keys
    static const char *contentKey[] = {"content", "\0"};
    static const char *endOfCompletationKey[] = {"stop", "\0"};
    static const char *tokenPerSecondsKey[] = {"timings", "predicted_per_second", "\0"};

    // Extract json fragment from chunk
    std::string completionData = chunk;
    if (const_cast<CallbackBus*>(bus)->stream) {
        size_t dataKeyPos = chunk.find("data: ");
        if (dataKeyPos == std::string::npos) return true;
        completionData = chunk.substr(dataKeyPos + 6);
    } else {
        size_t contentKeyPos = chunk.find("\"content\":");
        if (contentKeyPos == std::string::npos) return true;
    }

    // Parse the json
    sjson result(sjson::read(completionData.c_str()));

    // Get current token under 'content' key
    std::string token = result.get_str(contentKey);

    // Check end of completation
    if (result.get_bool(endOfCompletationKey)) {
        if (!const_cast<CallbackBus*>(bus)->stream){
            std::cout << token;
            const_cast<CallbackBus*>(bus)->buffer+=token;
        }

        // show token/s in the terminal title
        char windowTitle[30];
        std::snprintf(windowTitle, sizeof(windowTitle), "%.1f t/s\n",
                      result.get_real(tokenPerSecondsKey));
        Terminal::setTitle(windowTitle);

        completionInProgress = false;
        return false;
    }

    // Print the token in terminal
    std::cout << token << std::flush;

    const_cast<CallbackBus*>(bus)->buffer+=token;
    return true;
}

class Completion {
public:
    Completion(){};

    bool loadParametersSettings(std::string_view profile_name);

    std::string dumpJsonPayload(std::string prompt);

    Response requestCompletion(const char* ipaddr, const int16_t port, std::string prompt);

    prompt_template_t& getTemplates();

    void addStopWord(std::string word);

    bool loadPromptTemplates(std::string_view prompt_template_name);

    CallbackBus completionBuffer = {"", true};

private:

    httpRequest Req;

    prompt_template_t prompt_template = {"", "\n", "", "\n", "", "\n"};

    Dict<std::string> parameters;

    std::vector<std::string> stop_words;

};
#endif
