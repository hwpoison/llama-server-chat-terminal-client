#ifndef __CHAT_H__
#define __CHAT_H__
#include <iostream>
#include <string>
#include <vector>
#include <signal.h>
#include <filesystem>
#include <algorithm>
#include <map>

#include "sjson.hpp" 
#include "completion.hpp"
#include "colors.h"
#include "logging.hpp"

#define DEFAULT_FILE_EXTENSION ".json"
#define MY_PROMPT_FILENAME "my_prompts.json"

using namespace std;

//////////////////////////////////////////////

struct prompt_template_t {
    std::string prompt_type;

    std::string bos; // Beging of string
    std::string begin_system;
    std::string end_system;

    std::string begin_user;
    std::string end_user;

    std::string begin_assistant;
    std::string eos; // End of string
};

struct actor_t {
    std::string name;
    std::string role;
    std::string tag_color;
};

struct chat_entry_t {
    std::string actor_name;
    std::string message_content;
};

typedef std::map<std::string, actor_t> actor_list_t;

typedef std::vector<chat_entry_t> chat_history_t;


/* All related about the chat prompt is on this calss */
class Chat : public Completion {
public:
    Chat(){};

    bool actorExists(std::string actor_name){
        if(actors.find(actor_name) != actors.end()) // if doest not exists
            return true;
        return false;
    }

    void addNewMessage(std::string actor_name, std::string content) {
        chat_entry_t newEntry;
        newEntry.actor_name = actor_name;
        newEntry.message_content = content;
        history.push_back(newEntry);
    }

    bool removeLastMessage(int nmessages = 1) {
        if(instruct_mode and history.size() == 1){
            history.pop_back();
            return true;
        }
        if (history.size() == 2) {
            history.pop_back();
            return true;
        };
        if (history.size() > 2) {
            for (int i = 0; i < nmessages; i++) history.pop_back();
            return true;
        }
        return false;
    }

    // Delete double breakline and space at the start
    void cureCompletionForChat(){
        if(guards){
            completionBuffer.buffer.erase(0, completionBuffer.buffer.find_first_not_of(" "));
            while (!completionBuffer.buffer.empty() && completionBuffer.buffer.back() == '\n') {
                completionBuffer.buffer.erase(completionBuffer.buffer.size() - 1);
            }
        }
    }

    // Return chat history + prompt template
    std::string dumpFormatedPrompt() {
        std::string newPrompt;
        newPrompt+= prompt_template.bos;
        for (const chat_entry_t &entry : history) {
            actor_t actor = actors[entry.actor_name];
            if (actor.role == "user") {
                newPrompt += prompt_template.begin_user;
                if(!instruct_mode)newPrompt += actor.name + ":";
                newPrompt += entry.message_content;
                newPrompt += prompt_template.end_user;
            } else if (actor.role == "system") {
                newPrompt += prompt_template.begin_system;
                if(!instruct_mode) newPrompt += actor.name + ":";
                newPrompt += entry.message_content;
                newPrompt += prompt_template.end_system;
            }else{
                newPrompt += prompt_template.begin_assistant;
                if(!instruct_mode)newPrompt += actor.name + ":";
                newPrompt += entry.message_content;
                newPrompt += prompt_template.eos;
            }
        }
        return newPrompt;
    }

    // Append the prompt tail related to an actor
    std::string getActorTemplateFooter(std::string actor_name) {
        std::string footer;
        actor_t actor = actors[actor_name];
        if (actor.role == "user") {
            footer += prompt_template.begin_user;
            if(!instruct_mode)footer += actor.name + ":";
            footer += prompt_template.end_user;
        } else if (actor.role == "system") {
            footer += prompt_template.begin_system;
            if(!instruct_mode)footer += actor.name + ":";           
        } else {
            footer += prompt_template.begin_assistant;
            if(!instruct_mode)footer += actor.name + ":";
        }
        return footer;
    }

    void resetChatHistory(){
        if (history.size() > 1)
            history.erase(history.begin() + 1, history.end());
    }

    void draw() {
        Terminal::clear();
        for (const chat_entry_t &entry : history) {
            actor_t actor = actors[entry.actor_name];
            printActorChaTag(entry.actor_name.c_str());
            cout << entry.message_content << "\n";
            if (actors[entry.actor_name].name == "System")
                cout << "\n";          
        }
    }

    bool loadUserPromptProfile(const char *prompt_name) {
        sjson prompt_file = sjson(MY_PROMPT_FILENAME);
        if(!prompt_file.is_opened())
            return false;

        const char* prompt_[] = {prompt_name, "\0"}; // file -> daryl
        yyjson_val *my_prompt = prompt_file.get_value(prompt_);
        if(my_prompt==NULL){
            logging::error("Prompt \"%s\" not found.", prompt_name);
            return false;
        }

        yyjson_val *system = yyjson_obj_get(my_prompt, "system");  // daryl->system
        if(system != NULL){
            createActor("System", "system", "green_ul");
            system_prompt = yyjson_get_str(system);
            addNewMessage("System", system_prompt);
        }
        
        // load actors list
        yyjson_val *actors = yyjson_obj_get(my_prompt, "actors");  // daryl->actors
        if(actors == NULL){
            createActor("User", "user", "blue");
            createActor("Assistant", "assistant", "pink");
            user_name = "User";
            assistant_name = "Assistant";
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
                    createActor(name_str, role_str, color_str);
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

    bool loadSavedConversation(std::string file_path) {
        sjson saved_file = sjson(file_path.c_str());
        if(!saved_file.is_opened())
            return false;

        history.clear();
        yyjson_val *hits = yyjson_obj_get(saved_file.get_current_root(), "history");
        size_t idx, max;
        yyjson_val *hit;
        yyjson_arr_foreach(hits, idx, max, hit) {
            const char *name_str = yyjson_get_str(yyjson_obj_get(hit, "actor_name"));
            const char *role_str = yyjson_get_str(yyjson_obj_get(hit, "role"));
            const char *content = yyjson_get_str(yyjson_obj_get(hit, "content"));

            createActor(name_str, role_str, ANSIColors::getRandColor());
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
        for (const chat_entry_t &entry : history) {
            yyjson_mut_val *historyEntry = yyjson_mut_obj(doc);
            yyjson_mut_obj_add_str(doc, historyEntry, "actor_name",
                                   actors[entry.actor_name].name.c_str());
            yyjson_mut_obj_add_str(doc, historyEntry, "role",
                                   actors[entry.actor_name].role.c_str());
            yyjson_mut_obj_add_str(doc, historyEntry, "content",
                                   entry.message_content.c_str());
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

    bool loadPromptTemplates(const char *prompt_template_name) {
        sjson template_file = sjson(PARAMS_FILENAME);
        if(!template_file.is_opened())
            return false;

        const char *main_key = "prompt_templates";
        const char *PROMPT_TYPE_[] = {main_key, prompt_template_name, "TYPE", "\0"};
        const char *prompt_type = template_file.get_str(PROMPT_TYPE_);
        if(prompt_type == NULL) return false;

        if(!strcmp(template_file.get_str(PROMPT_TYPE_), "instruct")){
            instruct_mode = true;
            guards = false;
        }

        const auto loadSeqToken = [&](const char *key, const char *field_key) {
            const char *keys[] = {main_key, prompt_template_name, key, field_key, "\0"};
            const char* key_value = template_file.get_str(keys);
            return key_value?key_value:"";
        };

        prompt_template.bos =                loadSeqToken("SEQ", "BOS");
        prompt_template.begin_system =       loadSeqToken("SEQ", "B_SYS");
        prompt_template.end_system =         loadSeqToken("SEQ", "E_SYS");
        prompt_template.begin_user =         loadSeqToken("SEQ", "B_USER");
        prompt_template.end_user =           loadSeqToken("SEQ", "E_USER");
        prompt_template.begin_assistant =    loadSeqToken("SEQ", "B_ASSISTANT");
        prompt_template.eos =                loadSeqToken("SEQ", "EOS");

        return true;
    }

    void printActorChaTag(std::string actor_name) {
        actor_t actor = actors[actor_name];
        std::cout << ANSIColors::getColorCode(actor.tag_color) << actor.name
                  << ANSI_COLOR_RESET << ":";
    }

    void addActorStopWords(std::string actor_name){
        addStopWord(actor_name + ":");
        addStopWord(toUpperCase(actor_name + ":"));
        addStopWord(toLowerCase(actor_name + ":"));
    }

    bool createActor(std::string name, const char* role, const char* tag_color){
        if(actorExists(name))
            return false;
        actor_t newActor = { name, role, tag_color };
        actors[newActor.name] = newActor;
        if(guards) addActorStopWords(name);
        return true;
    }

    actor_t getActor(const char* actor_name){
        return actors[actor_name];
    }

    std::string getSystemPrompt(){
        return system_prompt;
    }

    std::string& getUserName(){
        return user_name;
    }

    std::string& getAssistantName(){
        return assistant_name;
    }

    void removeAllActors(){
        actors.clear();
    }

    // add chat guards, all possible variations of stop words expected for preserve
    // a chat scheme conversation.
    void setupStopWords(){
        if(guards){
            for (const auto &[key, actor] : actors) {
                addActorStopWords(actor.name);
            }

            auto addStop = [this](std::string token) {
                if(token==""){}else{
                    if(token!="\n") addStopWord(normalizeText(token));
                }
            };

            addStop(prompt_template.begin_user);
            addStop(prompt_template.end_user);
            addStop(prompt_template.begin_system);
            addStop(prompt_template.end_system);
            addStop(prompt_template.eos);
        }
    }

    void listCurrentActors(){
        for (const auto &[key, value] : actors) {
            std::cout << key << "\n";
        }
    }

    bool guards = true;
    bool instruct_mode = false;
private:
    std::string user_name;
    std::string assistant_name;

    std::string system_prompt;
    prompt_template_t prompt_template = {"", "\n", "", "\n", "", "\n"};
    
    actor_list_t actors;
    chat_history_t history;
};
#endif