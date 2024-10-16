#ifndef __CHAT_H__
#define __CHAT_H__
#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <filesystem>
#include <algorithm>

#include "sjson.hpp" 
#include "completion.hpp"
#include "colors.h"
#include "logging.hpp"

#define DEFAULT_FILE_EXTENSION ".json"
#define MY_PROMPT_FILENAME "prompts.json"
#define TEMPLATES_FILENAME "templates.json"

//////////////////////////////////////////////


/* All related about the chat prompt is on this calss */
class Chat : public Completion {
public:
    Chat(){};

    void addNewMessage(std::string_view actor_name, std::string_view content) {
        message_entry_t newEntry(&participants[actor_name.data()], content.data());
        messages.push_back(newEntry);
    }

    bool removeLastMessage(int nmessages = 1) {
        size_t message_count = messages.size();

        if(instruct_mode && message_count == 1){
            messages.pop_back();
            return true;
        }

        if (message_count == 2) {
            if(using_system_prompt){
                messages.pop_back();
            }else{
                messages.clear();
            }
            return true;
        };

        if (message_count > 2) {
            nmessages = std::min(nmessages, static_cast<int>(message_count));
            for (int i = 0; i < nmessages; i++) messages.pop_back();
            return true;
        }
        return false;
    }


    void updateMessageContent(int number, const std::string& newContent){
        messages[number].content = newContent;
    }

    void resetChatHistory(){ 
        if(instruct_mode or !using_system_prompt){ // remove all messages
            messages.clear();
        }else{             // reset all except the system prompt
            if (messages.size() > 1)
                messages.erase(messages.begin() + 1, messages.end());
        }
    }

    void draw() {
        Terminal::clear();

        for (const message_entry_t& entry : messages) {
            printActorChaTag(entry.participant_info->name);
            std::cout << entry.content << "\n";
            
            if (entry.participant_info->name == "System") {
                std::cout << "\n";
            }
        }
    }


    bool loadUserPrompt(std::string_view prompt_name) {
        sjson prompt_file = sjson(MY_PROMPT_FILENAME);
        if(!prompt_file.is_opened())
            return false;

        const char* prompt_[] = {prompt_name.data(), "\0"}; // file -> daryl
        yyjson_val *my_prompt = prompt_file.get_value(prompt_);
        if(my_prompt==NULL){
            logging::error("Prompt \"%s\" not found.", prompt_name.data());
            return false;
        }

        participants.clear();
        messages.clear();

        yyjson_val *system = yyjson_obj_get(my_prompt, "system");  // daryl->system
        if(system != NULL){
            addActor("System", "system", "green_ul");
            addNewMessage("System", yyjson_get_str(system));
            using_system_prompt = true;
        }else{
            using_system_prompt = false;
        }
        
        // load actors list
        yyjson_val *actors = yyjson_obj_get(my_prompt, "actors");  // daryl->actors
        if(actors == NULL){ // default actors
            setupDefaultActors();
            return true;
        }

        size_t actor_count = yyjson_arr_size(actors);
        for (size_t i = 0; i < actor_count; ++i) {
            yyjson_val *actor = yyjson_arr_get(actors, i);
            if (actor && yyjson_get_type(actor) == YYJSON_TYPE_OBJ) {
                yyjson_val *name = yyjson_obj_get(actor, "name");
                yyjson_val *role = yyjson_obj_get(actor, "role");
                yyjson_val *color = yyjson_obj_get(actor, "color");
                if (name && role && color) {
                    const char *name_str = yyjson_get_str(name);
                    const char *role_str = yyjson_get_str(role);
                    const char *color_str = yyjson_get_str(color);
                    addActor(name_str, role_str, color_str);
                    if(!strcmp(role_str,"user")){
                        user_name = name_str;
                    }else if(!strcmp(role_str,"assistant")){
                        assistant_name = name_str;
                    }
                }
            }
        }

        return true;
    }

    void setupDefaultActors(){
        addActor("User", "user", "blue");
        addActor("Assistant", "assistant", "pink");
        user_name = "User";
        assistant_name = "Assistant";
    }

    bool loadSavedConversation(std::string file_path) {
        sjson saved_file = sjson(file_path.c_str());
        if(!saved_file.is_opened())
            return false;

        participants.clear();
        messages.clear();

        yyjson_val *hits = yyjson_obj_get(saved_file.get_current_root(), "history");
        size_t idx, max;
        yyjson_val *hit;
        yyjson_arr_foreach(hits, idx, max, hit) {
            const char *name_str = yyjson_get_str(yyjson_obj_get(hit, "name"));
            const char *role_str = yyjson_get_str(yyjson_obj_get(hit, "role"));
            const char *content = yyjson_get_str(yyjson_obj_get(hit, "content"));

            addActor(name_str, role_str, ANSIColors::getRandColor());
            addNewMessage(name_str, content);
            if(!strcmp(role_str, "user")){
                user_name = name_str;
            }else if(!strcmp(role_str,"assistant")){
                assistant_name = name_str;
            }
        }

        return true;
    }

    bool saveConversation(std::string filename) {
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);

        // History array
        yyjson_mut_val *historyArray = yyjson_mut_arr(doc);
        for (const message_entry_t &entry : messages) {
            yyjson_mut_val *historyEntry = yyjson_mut_obj(doc);
            yyjson_mut_obj_add_str(doc, historyEntry, "name",
                                   entry.participant_info->name.c_str());
            yyjson_mut_obj_add_str(doc, historyEntry, "role",
                                   entry.participant_info->role.c_str());
            yyjson_mut_obj_add_str(doc, historyEntry, "content",
                                   entry.content.c_str());
            yyjson_mut_arr_add_val(historyArray, historyEntry);
        }
        yyjson_mut_obj_add_val(doc, root, "history", historyArray);

        yyjson_write_flag flg =
            YYJSON_WRITE_PRETTY;  // | YYJSON_WRITE_ESCAPE_UNICODE;
        yyjson_write_err err;
        yyjson_mut_write_file(filename.c_str(), doc, flg, NULL, &err);
        yyjson_mut_doc_free(doc);
        if (err.code) {
            logging::error("Error writing the file \"%s\": %s", filename.c_str(), err.msg);
            return false;
        }
        return true;
    }
    
    void printActorChaTag(std::string_view actor_name) {
        participant_t actor = participants[actor_name.data()];
        std::cout << ANSIColors::getColorCode(actor.tag_color) << actor.name
                  << ANSI_COLOR_RESET << ":";
    }

    void addActorStopWords(std::string actor_name){
        addStopWord(actor_name + ":");
        addStopWord(toUpperCase(actor_name + ":"));
        addStopWord(toLowerCase(actor_name + ":"));
    }

    bool addActor(std::string name, const char* role, const char* tag_color){
        if(participants.find(name.data()) != participants.end() ? true : false)
            return false;
        participant_t newP = { name, role, tag_color };
        addParticipant(newP);
        if(chat_guards) addActorStopWords(name);
        return true;
    }

    std::string& getUserName(){
        return user_name;
    }

    std::string& getAssistantName(){
        return assistant_name;
    }

    void removeAllActors(){
        participants.clear();
    }

    // Delete double breakline and space at the start
    void cureCompletionForChat(){
        if(chat_guards){
            completionBuffer.buffer.erase(0, completionBuffer.buffer.find_first_not_of(" "));
            while (!completionBuffer.buffer.empty() && completionBuffer.buffer.back() == '\n') {
                completionBuffer.buffer.erase(completionBuffer.buffer.size() - 1);
            }
        }
    }

    // Set all possible variations related to a chat context
    void setupChatStopWords() {
        if(chat_guards){
            for (const auto &[key, actor] : participants) {
                addActorStopWords(actor.name);
            }

            prompt_template_t &templates = getTemplates(); 
            auto addStop = [this](std::string token) {
                if(token==""){}else{
                    if(token!="\n") addStopWord(normalizeText(token));
                }
            };
            
            addStop(templates.begin_user);
            addStop(templates.end_user);
            addStop(templates.begin_system);
            addStop(templates.end_system);
            addStop(templates.eos);

        }
    }

    void listCurrentActors(){
        for (const auto &[key, value] : participants) {
            std::cout << key << "\n";
        }
    }

    bool chat_guards = true;

private:
    bool using_system_prompt = false;
    std::string user_name;
    std::string assistant_name;
};
#endif