#include "chat.hpp"


void Chat::addNewMessage(std::string_view actor_name, std::string_view content) {
    message_entry_t newEntry(&actors[actor_name.data()], content.data());
    messages.push_back(newEntry);
}

bool Chat::removeLastMessage(int nmessages = 1) {
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
        messages.erase(messages.end() - nmessages, messages.end());
        return true;
    }
    return false;
}

void Chat::updateMessageContent(int number, std::string_view newContent) {
    messages[number].content.assign(newContent);
}

void Chat::resetChatHistory(){ 
    if(instruct_mode or !using_system_prompt){ // remove all messages
        messages.clear();
    }else{             // reset all except the system prompt
        if (messages.size() > 1)
            messages.erase(messages.begin() + 1, messages.end());
    }
}

void Chat::draw() {
    Terminal::clear();

    for (const message_entry_t& entry : messages) {
        printActorChaTag(entry.actor_info->name);
        std::cout << entry.content << "\n";
        
        if (entry.actor_info->name == "System") {
            std::cout << "\n";
        }
    }
}

bool Chat::loadUserPrompt(std::string_view prompt_name) {
    sjson prompt_file = sjson(MY_PROMPT_FILENAME);
    if(!prompt_file.is_opened())
        return false;

    const char* prompt_[] = {prompt_name.data(), "\0"}; // file -> daryl
    yyjson_val *my_prompt = prompt_file.get_value(prompt_);
    if(my_prompt==NULL){
        logging::error("Prompt \"%s\" not found.", prompt_name.data());
        return false;
    }

    actors.clear();
    messages.clear();

    yyjson_val *system = yyjson_obj_get(my_prompt, "system");  // daryl->system
    if(system != NULL){
        setupSystemPrompt(yyjson_get_str(system));
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
            yyjson_val *msg_preffix = yyjson_obj_get(actor, "msg_preffix");
            if (name && role) {
                const char *name_str = yyjson_get_str(name);
                const char *role_str = yyjson_get_str(role);
                const char *color_str = "";
                const char *msg_preffix_str = "";

                if (msg_preffix != NULL) {
                    msg_preffix_str = yyjson_get_str(msg_preffix);
                }
                if (color != NULL){
                    color_str = yyjson_get_str(color);
                }else{
                    color_str = ANSIColors::getRandColor();
                }

                addActor(name_str, role_str, color_str, msg_preffix_str);
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

void Chat::setupSystemPrompt(std::string prompt){
    addActor("System", "system", "green_ul");
    addNewMessage("System", prompt);
    using_system_prompt = true;
}

void Chat::updateSystemPrompt(std::string new_system_prompt) {
    updateMessageContent(0, new_system_prompt);
}

void Chat::setupDefaultActors(){
    addActor("User", "user", "blue");
    addActor("Assistant", "assistant", "pink");
    user_name = "User";
    assistant_name = "Assistant";
}

bool Chat::loadSavedConversation(std::string file_path) {
    sjson saved_file = sjson(file_path.c_str());
    if(!saved_file.is_opened())
        return false;

    actors.clear();
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

bool Chat::saveConversation(std::string filename) {
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    // History array
    yyjson_mut_val *historyArray = yyjson_mut_arr(doc);
    int msg_id = 0;
    for (const message_entry_t &entry : messages) {
        yyjson_mut_val *historyEntry = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_int(doc, historyEntry, "id",
                               msg_id++);
        yyjson_mut_obj_add_str(doc, historyEntry, "name",
                               entry.actor_info->name.c_str());
        yyjson_mut_obj_add_str(doc, historyEntry, "role",
                               entry.actor_info->role.c_str());
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

void Chat::printActorChaTag(std::string_view actor_name) {
    actor_t actor = actors[actor_name.data()];
    std::cout << ANSIColors::getColorCode(actor.tag_color) << actor.name
              << ANSI_COLOR_RESET << ":";
}

void Chat::addActorStopWords(std::string actor_name){
    addStopWord(actor_name + ":");
    addStopWord(toUpperCase(actor_name + ":"));
    addStopWord(toLowerCase(actor_name + ":"));
}

bool Chat::addActor(std::string name, const char* role, const char* tag_color, std::string preffix){
    if(actors.find(name.data()) != actors.end() ? true : false)
        return false;
    actor_t new_actor = { name, role, tag_color , preffix};
    actors[new_actor.name] = new_actor;
    if(chat_guards) addActorStopWords(name);
    return true;
}

std::string& Chat::getUserName(){
    return user_name;
}

std::string& Chat::getAssistantName(){
    return assistant_name;
}

void Chat::removeAllActors(){
    actors.clear();
}

// Delete double breakline and space at the start
void Chat::cureCompletionForChat(){
    if(chat_guards){
        completionBuffer.buffer.erase(0, completionBuffer.buffer.find_first_not_of(" "));
        while (!completionBuffer.buffer.empty() && completionBuffer.buffer.back() == '\n') {
            completionBuffer.buffer.erase(completionBuffer.buffer.size() - 1);
        }
    }
}

// Set all possible variations related to a chat context
void Chat::setupChatStopWords() {
    if(chat_guards){
        for (const auto &[key, actor] : actors) {
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

int Chat::messagesCount(){
    return messages.size();
}

void Chat::listCurrentActors(){
    for (const auto &[key, value] : actors) {
        std::cout << key << "\n";
    }
}

// Return legacy prompt with applied prompt template
std::string Chat::composePrompt(){
    std::string newPrompt;
    prompt_template_t &prompt_template = getTemplates();
    newPrompt+= prompt_template.bos;
    for (size_t i = 0; i < messages.size(); ++i) {
        const message_entry_t& entry = messages[i];
        bool is_last = i == messages.size() - 1;
        std::string tag = entry.actor_info->name + ":";
        if (entry.actor_info->role == "user") {
            newPrompt += prompt_template.begin_user;
            if(!instruct_mode)newPrompt += tag;
            newPrompt += entry.actor_info->msg_preffix;
            newPrompt += entry.content;
            if(!is_last)newPrompt += prompt_template.end_user;
        } else if (entry.actor_info->role == "system") {
            newPrompt += prompt_template.begin_system;
            newPrompt += entry.actor_info->msg_preffix;
            newPrompt += entry.content;
            newPrompt += prompt_template.end_system;
        }else{
            newPrompt += prompt_template.begin_assistant;
            if(!instruct_mode)newPrompt += tag;
            newPrompt += entry.actor_info->msg_preffix;
            newPrompt += entry.content;
            if(!is_last)newPrompt += prompt_template.end_assistant;
            if(!is_last)newPrompt += prompt_template.eos;
        }
    };

    return newPrompt;
}

// ignore actor tags in chat
bool Chat::setInstructMode(bool value){
    instruct_mode = value;
    return instruct_mode;
}

bool Chat::isInstructMode(){
    return instruct_mode;
}

bool Chat::setChatGuards(bool value){
    chat_guards = value;
    return chat_guards;
}