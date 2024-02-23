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

struct participant_t {
    std::string name;
    std::string role;
    std::string tag_color;
};

struct message_entry_t {
    message_entry_t(participant_t *participant_info, std::string content) : participant_info(participant_info), content(content) {}
    participant_t *participant_info;
    std::string content;
};

typedef std::map<std::string, participant_t> participants_t;
typedef std::vector<message_entry_t> messages_t;

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

static bool completionCallback(const std::string &chunck, const CallbackBus *bus) {
    if (stopCompletionFlag) {
        Terminal::setTitle("Interrupted by user");
        stopCompletionFlag = false;
        completionInProgress = false;
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
        completionInProgress = false;
        return false;
    }
    std::cout << token << std::flush;  // write the token in terminal

    const_cast<CallbackBus*>(bus)->buffer+=token;
    return true;
}

class Completion {
public:
    Completion(){};//setPrompt("");};

    bool loadParametersSettings(std::string_view profile_name);

    std::string dumpJsonPayload();

    Response requestCompletion(const char* ipaddr, const int16_t port);

    prompt_template_t& getTemplates();

    participants_t& getParticipants();

    void addStopWord(std::string word);

    void addParticipant(participant_t& info){
        participants[info.name] = info;
    }

    bool loadPromptTemplates(const char* prompt_template_name);

    std::string dumpLegacyPrompt();

    CallbackBus completionBuffer = {"", true};

    bool instruct_mode = false;
    messages_t messages;
    participants_t participants;

private:

    httpRequest Req;

    prompt_template_t prompt_template = {"", "\n", "", "\n", "", "\n"};

    AnyMap<std::string> parameters;

    std::vector<std::string> stop_words;

};
#endif
