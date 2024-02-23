#include "completion.hpp"

bool stopCompletionFlag = false;
bool completionInProgress = false;

// Read parameters from params.json file. [params_profiles][profile_name]
bool Completion::loadParametersSettings(std::string_view profile_name) {
    sjson parameters_file = sjson(PARAMS_FILENAME);
    if(!parameters_file.is_opened())
        return false;

    // Choice the profile entry and work with it
    const char *PARAMETER_PROFILE_[] = {"params_profiles", profile_name.data(), "\0"};
    yyjson_val *parameter_profile = parameters_file.get_value(PARAMETER_PROFILE_);
    if(parameter_profile == NULL) return false;
    
    yyjson_val *key, *val;
    yyjson_obj_iter iter = yyjson_obj_iter_with(parameter_profile);

    parameters.clear();

    // Map the json content to parameters
    while ((key = yyjson_obj_iter_next(&iter))) {
        const char* param_key = yyjson_get_str(key);
        yyjson_val *param_value = yyjson_obj_iter_get_val(key);
        std::string value_type = yyjson_get_type_desc(param_value);

        if(value_type == "uint"){
            parameters.set<int>(param_key, yyjson_get_int(param_value));
        }else if(value_type == "real"){
            parameters.set<double>(param_key, yyjson_get_real(param_value));
        }else if(value_type == "string"){
            parameters.set<std::string>(param_key, "\"" + std::string(yyjson_get_str(param_value)) + "\"");
        }else if(value_type == "true"){
            parameters.set<bool>(param_key, true);
        }else if(value_type == "false"){
            parameters.set<bool>(param_key, false);
        }else if(value_type == "array"){
            if(!strcmp(param_key, "stop")){
                size_t idx, max;
                yyjson_val *hit;
                yyjson_arr_foreach(param_value, idx, max, hit)
                        stop_words.push_back(std::string(yyjson_get_str(hit)));
            }
        }
    }

    if(this->parameters.get<bool>("stream") == false){
        completionBuffer.stream = false;
    }

    return true;
}

// load parameters from params.json
bool Completion::loadPromptTemplates(const char* prompt_template_name) {
    sjson templates_fp = sjson(TEMPLATES_FILENAME);
    if(!templates_fp.is_opened())
        return false;

    const char *main_key = "prompt_templates";
    
    const auto loadSeqToken = [&](const char *key, const char *field_key) {
        const char *keys[] = {main_key, prompt_template_name, key, field_key, "\0"};
        const char* key_value = templates_fp.get_str(keys);
        return key_value?key_value:"";
    };

    prompt_template.bos =                loadSeqToken("SEQ", "BOS");
    prompt_template.begin_system =       loadSeqToken("SEQ", "B_SYS");
    prompt_template.end_system =         loadSeqToken("SEQ", "E_SYS");
    prompt_template.begin_user =         loadSeqToken("SEQ", "B_USER");
    prompt_template.end_user =           loadSeqToken("SEQ", "E_USER");
    prompt_template.begin_assistant =    loadSeqToken("SEQ", "B_ASSISTANT");
    prompt_template.end_assistant =      loadSeqToken("SEQ", "E_ASSISTANT");
    prompt_template.eos =                loadSeqToken("SEQ", "EOS");

    return true;
}

// compose json to send
std::string Completion::dumpJsonPayload() {
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    // prompt content
    std::string legacy_prompt = dumpLegacyPrompt();
    yyjson_mut_obj_add_str(doc, root, "prompt", legacy_prompt.c_str());
 
    // stop word list
    const char **stop_word_arr = new const char *[stop_words.size()];
    for(int i = 0; i < stop_words.size(); i++)
        stop_word_arr[i] = stop_words[i].c_str();
    yyjson_mut_val *hits = yyjson_mut_arr_with_str(doc, stop_word_arr, stop_words.size());
    yyjson_mut_obj_add_val(doc, root, "stop", hits);

    // others params
    parameters.forEach([&doc, &root](const std::string& key, const std::any& value) {
        if (value.type() == typeid(int)) {
            yyjson_mut_obj_add_int(doc, root, key.c_str(), std::any_cast<int>(value));
        } else if (value.type() == typeid(double)) {
            yyjson_mut_obj_add_real(doc, root, key.c_str(), std::any_cast<double>(value));
        } else if (value.type() == typeid(bool)) {
            yyjson_mut_obj_add_bool(doc, root, key.c_str(), std::any_cast<bool>(value));
        } else if (value.type() == typeid(std::string)) {
            yyjson_mut_obj_add_str(doc, root, key.c_str(), std::any_cast<std::string>(value).c_str());
        }
    }); 

    const char *json = yyjson_mut_write(doc, 0, NULL);
    std::string result(json);

    delete[] stop_word_arr;
    yyjson_mut_doc_free(doc);
    free(const_cast<char*>(json));
    return result;
}


Response Completion::requestCompletion(
    const char* ipaddr, const int16_t port) 
{
    completionInProgress = true;
    return Req.post(ipaddr, port, DEFAULT_COMPLETION_ENDPOINT, dumpJsonPayload(), completionCallback, &completionBuffer);
}

prompt_template_t& Completion::getTemplates(){
    return prompt_template;
}

void Completion::addStopWord(std::string word){
    stop_words.push_back(word);
}

// Return legacy prompt with applied prompt template
std::string Completion::dumpLegacyPrompt(){
    std::string newPrompt;
    newPrompt+= prompt_template.bos;
    for (size_t i = 0; i < messages.size(); ++i) {
        const message_entry_t& entry = messages[i];
        bool is_last = i == messages.size() - 1;
        std::string tag = entry.participant_info->name + ":";
        if (entry.participant_info->role == "user") {
            newPrompt += prompt_template.begin_user;
            if(!instruct_mode)newPrompt += tag;
            newPrompt += entry.content;
            if(!is_last)newPrompt += prompt_template.end_user;
        } else if (entry.participant_info->role == "system") {
            newPrompt += prompt_template.begin_system;
            newPrompt += entry.content;
            newPrompt += prompt_template.end_system;
        }else{
            newPrompt += prompt_template.begin_assistant;
            if(!instruct_mode)newPrompt += tag;
            newPrompt += entry.content;
            if(!is_last)newPrompt += prompt_template.end_assistant;
            if(!is_last)newPrompt += prompt_template.eos;
        }
    };

    return newPrompt;
}
