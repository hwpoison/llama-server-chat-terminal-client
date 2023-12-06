#include "completion.hpp"

bool stopCompletionFlag = false;


bool Completion::loadParametersSettings(const char *profile_name) {
    // Retrieve parsed parameters from parameters.json
    sjson parameters_file = sjson(PARAMS_FILENAME);
    if(!parameters_file.is_opened())
        return false;

    // Choice the profile entry and work with it
    const char *PARAMETER_PROFILE_[] = {"params_profiles", profile_name, "\0"};
    yyjson_val *parameter_profile = parameters_file.get_value(PARAMETER_PROFILE_);

    yyjson_val *key, *val;
    yyjson_obj_iter iter = yyjson_obj_iter_with(parameter_profile);

    parameters.clear();
    while ((key = yyjson_obj_iter_next(&iter))) {
        const char* param_key = yyjson_get_str(key);
        yyjson_val *param_value = yyjson_obj_iter_get_val(key);
        std::string value_type = yyjson_get_type_desc(param_value);

        if(value_type == "uint"){
            parameters[param_key] = std::to_string(yyjson_get_int(param_value));
        }else if(value_type == "real"){
            parameters[param_key] = std::to_string(yyjson_get_real(param_value));
        }else if(value_type == "array"){
            if(!strcmp(param_key, "stop")){
                size_t idx, max;
                yyjson_val *hit;
                yyjson_arr_foreach(param_value, idx, max, hit)
                        stop_words.push_back(std::string(yyjson_get_str(hit)));
            }
        }else if(value_type == "string"){
            parameters[param_key] = "\"" + std::string(yyjson_get_str(param_value)) + "\"";
        }else if(value_type == "true"){
            parameters[param_key] = "true";
        }else if(value_type == "false"){
            parameters[param_key] = "false";
        }
    }

    if(this->parameters["stream"] == "false"){
        completionBuffer.stream = false;
    }

    return true;
}

// converts parameters map to json string to sent
std::string Completion::dumpJsonPayload() {
    std::string json = "{";
    for (auto const& [key, val] : parameters) {
        if(key!="prompt")
            json+= "\"" + key + "\":" + val + ",";
    }
    json += "\"stop\":[";
    for (size_t i = 0; i < stop_words.size(); ++i) {
        json += "\"" + stop_words[i] + "\"";
        if (i < stop_words.size() - 1)
            json += ",";
    }
    json += "],";
    json+= "\"prompt\":\"" + normalizeText(parameters["prompt"]) + "\"";

    json += "}";
    return json;
}

Response Completion::requestCompletion(
    const char* ipaddr, const char* completion_endpoint, const int16_t port) 
{
    std::string json = dumpJsonPayload();
    Response res = Req.post(ipaddr, port, completion_endpoint, json, completionCallback, &completionBuffer);
    return res;
}

void Completion::setPrompt(std::string content){
    parameters["prompt"] = content;
}

std::string Completion::getPrompt(){
    return parameters["prompt"];
}

void Completion::addStopWord(std::string word){
    stop_words.push_back(word);
}