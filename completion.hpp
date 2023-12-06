#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <iostream>
#include <string>
#include <signal.h>
#include <map>

#include "sjson.hpp"
#include "utils.hpp"
#include "minipost.hpp"
#include "terminal.hpp"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080
#define DEFAULT_COMPLETION_ENDPOINT "/completion"
#define PARAMS_FILENAME "params.json"

extern bool stopCompletionFlag;

static void completionSignalHandler(int signum) {
    stopCompletionFlag =  true;
    signal(SIGINT, SIG_DFL);
}

static bool completionCallback(const std::string &chunck, const CallbackBus *bus) {
    if (stopCompletionFlag) {
        std::cout << std::endl << "Completion stopped by the user!..." << std::endl;
        Terminal::pause();
        stopCompletionFlag = false;
        return false;
    }

    // Extract and parse completion response data
    std::string completionData = chunck;
    if (const_cast<CallbackBus*>(bus)->stream) {
        size_t dataPos = chunck.find("data: ");
        if (dataPos == std::string::npos) return true;
        completionData = chunck.substr(dataPos + 6);
    } else {
        size_t dataPos = chunck.find("\"content\":");
        if (dataPos == std::string::npos) return true;
    }

    // Parse result
    sjson result(sjson::read(completionData.c_str()));
    
    // Check std::end of completion
    const char *content_[] = {"content", "\0"};
    const char *end_of_c[] = {"stop", "\0"};
    std::string token;
    token = result.get_str(content_);
    if (result.get_bool(end_of_c)) {
        if (!const_cast<CallbackBus*>(bus)->stream){
            std::cout << token;
            const_cast<CallbackBus*>(bus)->buffer+=token;
        }
        const char *timing[] = {"timings", "predicted_per_second", "\0"};
        double tokensPerSecond = result.get_real(timing);
        // show token speed in the terminal title
        char windowTitle[30];
        std::snprintf(windowTitle, sizeof(windowTitle), "%.1f t/s\n",
                      tokensPerSecond);
        Terminal::setTitle(windowTitle);
        return false;
    }

    std::cout << token << std::flush;  // write the token in terminal

    const_cast<CallbackBus*>(bus)->buffer+=token;
    return true;
}

class Completion {
public:

    bool loadParametersSettings(const char *profile_name);

    std::string dumpJsonPayload();

    Response requestCompletion(const char* ipaddr, const char* completion_endpoint, const int16_t port);

    void setPrompt(std::string content);

    std::string getPrompt();

    void addStopWord(std::string word);

    CallbackBus completionBuffer = {"", true};

private:
    httpRequest Req;
    std::map<std::string, std::string>  parameters = {{"prompt",""}};
    std::vector<std::string> stop_words;
};
#endif
