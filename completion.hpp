#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <iostream>
#include <string>
#include <csignal>

#include "sjson.hpp"
#include "utils.hpp"
#include "minipost.hpp"
#include "terminal.hpp"

#define DEFAULT_COMPLETION_ENDPOINT "http://localhost:8080/completion"

static bool stopCompletion = false;

struct parameters_t {
    int mirostat = 0;
    int mirostat_tau = 5;
    float mirostat_eta = 0.1;
    int frecuency_penalty = 0;
    int n_probs = 0;
    int presence_penalty = 0;
    int top_k = 40;
    float top_p = 0.5;
    int typical_p = 1;
    int tfz_z = 1;
    int repeat_last_n = 256;
    float repeat_penalty = 1.18;
    int slot_id = -1;
    float temperature = 0.8;
    int n_predict = 100;
    bool stream = true;
    std::vector<std::string> stop;
    std::string grammar = "";
    std::string prompt;
};

static bool completionCallback(const std::string &chunck, const CallbackBus *bus) {
    if (stopCompletion) {
        std::cout << std::endl << "Completion stopped by the user!..." << std::endl;
        stopCompletion = false;
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

    std::string jsonPayload();

    bool requestCompletion();

    void setPrompt(std::string content);

    void addPrompt(std::string content);

    std::string getPrompt(){
        return parameters.prompt;
    }
    std::string getCurrentPrompt();

    void addStopWord(std::string word);

    static void completionSignalHandler(int signum) {
        stopCompletion = false;
        signal(SIGINT, SIG_DFL);
    }

    CallbackBus completionBuffer = {"", true, false};
private:
    std::string endpoint_url = DEFAULT_COMPLETION_ENDPOINT;
    parameters_t parameters;
};
#endif
