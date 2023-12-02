#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <iostream>
#include <string>
#include <signal.h>
#include <atomic>

#include "sjson.hpp"
#include "utils.hpp"
#include "minipost.hpp"
#include "terminal.hpp"

#define DEFAULT_COMPLETION_ENDPOINT "http://localhost:8080/completion"
#define PARAMS_FILENAME "params.json"

extern std::atomic<bool> stopCompletionFlag;

struct parameters_t {
    int mirostat = 0;
    int mirostat_tau = 5;
    float mirostat_eta = 0.1;
    int frecuency_penalty = 0;
    int n_probs = 0;
    int presence_penalty = 0;
    int top_k = 40;
    float top_p = 0.95;
    float min_p = 0.05;
    int typical_p = 1;
    int tfz_z = 1;
    int repeat_last_n = 256;
    float repeat_penalty = 1.18;
    int slot_id = 0;
    float temperature = 0.8;
    int n_predict = 100;
    bool stream = true;
    bool ignore_eos = false;
    bool penalize_nl = true;
    std::vector<std::string> stop;
    std::string grammar = "";
    std::string prompt;
};

static void completionSignalHandler(int signum) {
    stopCompletionFlag =  true;
    //signal(SIGINT, SIG_DFL);
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
    std::cout << token;  // write the token in terminal
    const_cast<CallbackBus*>(bus)->buffer+=token;
    return true;
}

class Completion {
public:

    void loadParametersSettings(const char *profile_name);

    std::string dumpJsonPayload();

    bool requestCompletion(std::string endpoit_url);

    void setPrompt(std::string content);

    void addPrompt(std::string content);

    std::string getPrompt(){
        return parameters.prompt;
    }
    std::string getCurrentPrompt();

    void addStopWord(std::string word);

    CallbackBus completionBuffer = {"", true, false};
private:
    parameters_t parameters;
};
#endif
