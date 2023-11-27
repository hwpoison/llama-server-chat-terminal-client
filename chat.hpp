#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <map>

#include "sjson.hpp" 
#include "completion.hpp"
#include "colors.h"

using namespace std;

bool stopCompletionFlag = false;
bool completionStream = true;
std::string completionBuffer;

//////////////////////////////////////////////

struct prompt_template_t {
    std::string prompt_type;

    std::string begin_system;
    std::string end_system;

    std::string begin_user;
    std::string end_user;

    std::string begin_assistant;
    std::string eos;
};

struct actor_t {
    std::string name;
    std::string tag_color;
    std::string role;
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

    void addNewMessage(const char* actor_name, std::string content) {
        if(actors.find(actor_name) == actors.end()){ // if doest not exists
            addNewActor((actor_t){actor_name, ANSIColors::getRandColor() , "actor"});
        }
        chat_entry_t newEntry;
        newEntry.actor_name = actor_name;
        newEntry.message_content = content;
        history.push_back(newEntry);
    }

    bool removeMessage(int num = 1) {
        if (history.size() == 2) {
            history.pop_back();
            return true;
        };
        if (history.size() > 2) {
            for (int i = 0; i < num; i++) history.pop_back();
            return true;
        }
        return false;
    }

    std::string craftChatPromp() {
        std::string newPrompt;
        for (const chat_entry_t &entry : history) {
            actor_t actor = actors[entry.actor_name];
            if (actor.role == "user") {
                newPrompt += prompt_template.begin_user;
                newPrompt += actor.name + ":";
                newPrompt += entry.message_content;
                newPrompt += prompt_template.end_user;
            } else if (actor.role == "system") {
                newPrompt += prompt_template.begin_system;
                newPrompt += entry.message_content;
                newPrompt += prompt_template.end_system;
            } else if (actor.role == "assistant") {
                newPrompt += prompt_template.begin_assistant;
                newPrompt += actor.name + ":";
                newPrompt += entry.message_content;
                newPrompt += prompt_template.eos;
            } else {
                newPrompt += actor.name + ":";
                newPrompt += entry.message_content;
            }
        }
        return newPrompt;
    }

    void resetChatHistory(){
        if (history.size() > 1)
            history.erase(history.begin() + 1, history.end());
    }

    void draw() {
        Terminal::clear();
        for (const chat_entry_t &entry : history) {
            printActorChaTag(entry.actor_name.c_str());
            cout << entry.message_content << endl;
            if (actors[entry.actor_name].role == "system") cout << endl;
        }
    }

    void loadUserPromptProfile(const char *prompt_name) {
        sjson prompt_file = sjson("my_prompts.json");
        yyjson_val *my_prompt =
            yyjson_obj_get(prompt_file.get_current_root(), prompt_name);  // daryl
        yyjson_val *system = yyjson_obj_get(my_prompt, "system");  // daryl->system

        // load system prompt and register system actor
        system_prompt = yyjson_get_str(system);
        addNewActor((actor_t){"System", "green_bc", "system"});
        

        // load actors list
        yyjson_val *actors = yyjson_obj_get(my_prompt, "actors");  // daryl->actors
        size_t actor_count = yyjson_arr_size(actors);
        cout << actor_count << endl;

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
                    addNewActor((actor_t){name_str, color_str, role_str});

                    if(!strcmp(role_str,"user")){
                        user_name = name_str;
                    }else if(!strcmp(role_str,"assistant")){
                        assistant_name = name_str;
                    }
                }
            }
        }
    }

    bool loadSavedConversation(std::string file_path) {
        sjson saved_file = sjson(file_path.c_str());
        if(!saved_file.is_opened()){
            return false;
        }
        history.clear();
        yyjson_val *hits = yyjson_obj_get(saved_file.get_current_root(), "history");
        size_t idx, max;
        yyjson_val *hit;
        yyjson_arr_foreach(hits, idx, max, hit) {
            const char *name_str = yyjson_get_str(yyjson_obj_get(hit, "actor_name"));
            const char *role_str = yyjson_get_str(yyjson_obj_get(hit, "role"));
            const char *content = yyjson_get_str(yyjson_obj_get(hit, "content"));
            if (actors.find(name_str) != actors.end()) {
                addNewMessage(name_str, content);
            } else {
                addNewActor((actor_t){name_str, ANSIColors::getRandColor(), role_str});
                addNewMessage(name_str, content);
            }
            if(!strcmp(role_str, "user")){
                user_name = name_str;
            }else if(!strcmp(role_str,"assistant")){
                assistant_name = name_str;
            }
        }

        return true;
    }

    void saveConversation(std::string filename) {
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
        if (err.code) {
            printf("write error (%u): %s\n", err.code, err.msg);
        }
        yyjson_mut_doc_free(doc);
    }

    void loadPromptTemplates(const char *prompt_style_name) {
        prompt_template_t promptTemplate;
        const char *key_name = "prompt_templates";
        sjson template_base = sjson("params.json");

        const char *PROMPT_TYPE_[] = {key_name, prompt_style_name, "TYPE", "\0"};
        promptTemplate.prompt_type = template_base.get_str(PROMPT_TYPE_);

        const auto loadSeqToken = [&](const char *key, const char *field_key) {
            const char *keys[] = {key_name, prompt_style_name, key, field_key, "\0"};
            return template_base.get_str(keys);
        };
        promptTemplate.begin_system =       loadSeqToken("SEQ", "B_SYS");
        promptTemplate.end_system =         loadSeqToken("SEQ", "E_SYS");
        promptTemplate.begin_user =         loadSeqToken("SEQ", "B_USER");
        promptTemplate.end_user =           loadSeqToken("SEQ", "E_USER");
        promptTemplate.begin_assistant =    loadSeqToken("SEQ", "B_ASSISTANT");
        promptTemplate.eos =                loadSeqToken("SEQ", "EOS");

        prompt_template = promptTemplate;
    }

    void printActorChaTag(const char* actor_name) {
        actor_t actor = actors[actor_name];
        std::cout << ANSIColors::getColorCode(actor.tag_color) << actor.name
                  << ANSI_COLOR_RESET << ":";
    }

    bool addNewActor(actor_t actorInfo){
        if(actors.find(actorInfo.name) != actors.end())
            return false;
        actors[actorInfo.name] = actorInfo;
        addStopWord(actorInfo.name + ":");
        addStopWord(toUpperCase(actorInfo.name + ":"));
        return true;
    }

    actor_t getActor(const char* actor_name){
        return actors[actor_name];
    }

    std::string getPromptSystem(){
        return system_prompt;
    }

    const char* getUserName(){
        return user_name.c_str();
    }

    const char* getAssistantName(){
        return assistant_name.c_str();
    }

    void setupStopWords(){
        // add chat guards
        for (const auto &[key, value] : actors) {
            addStopWord(value.name + ":");
            addStopWord(toUpperCase(value.name + ":"));
        }

        if (prompt_template.begin_user != "")
            addStopWord(normalizeText(prompt_template.begin_user));
        if (prompt_template.end_user != "")
            addStopWord(normalizeText(prompt_template.end_user));
    }

    void listCurrentActors(){
        for (const auto &[key, value] : actors) {
            std::cout << key << std::endl;
        }
    }

private:
    std::string user_name;
    std::string assistant_name;

    std::string system_prompt;
    prompt_template_t prompt_template;
    actor_list_t actors;
    chat_history_t history;
};