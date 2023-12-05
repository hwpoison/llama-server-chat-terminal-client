#include "completion.hpp"

bool stopCompletionFlag = false;

void Completion::loadParametersSettings(const char *profile_name) {
    // Retrieve parsed parameters from parameters.json
    sjson parameters = sjson(PARAMS_FILENAME);

    // Choice the profile entry and work with it
    const char *PARAMETER_PROFILE_[] = {"params_profiles", profile_name, "\0"};
    yyjson_val *parameter_profile = parameters.get_value(PARAMETER_PROFILE_);

    parameters_t Parameters;
    parameters.set_to_member(parameter_profile, "mirostat",           Parameters,     &parameters_t::mirostat);
    parameters.set_to_member(parameter_profile, "mirostat_tau",       Parameters,     &parameters_t::mirostat_tau);
    parameters.set_to_member(parameter_profile, "mirostat_eta",       Parameters,     &parameters_t::mirostat_eta);
    parameters.set_to_member(parameter_profile, "stop",               Parameters,     &parameters_t::stop);
    parameters.set_to_member(parameter_profile, "frecuency_penalty",  Parameters,     &parameters_t::frecuency_penalty);
    parameters.set_to_member(parameter_profile, "top_k",              Parameters,     &parameters_t::top_k);
    parameters.set_to_member(parameter_profile, "top_p",              Parameters,     &parameters_t::top_p);
    parameters.set_to_member(parameter_profile, "n_probs",            Parameters,     &parameters_t::n_probs);
    parameters.set_to_member(parameter_profile, "grammar",            Parameters,     &parameters_t::grammar);
    parameters.set_to_member(parameter_profile, "presence_penalty",   Parameters,     &parameters_t::presence_penalty);
    parameters.set_to_member(parameter_profile, "typical_p",          Parameters,     &parameters_t::typical_p);
    parameters.set_to_member(parameter_profile, "tfz_z",              Parameters,     &parameters_t::tfz_z);
    parameters.set_to_member(parameter_profile, "repeat_last_n",      Parameters,     &parameters_t::repeat_last_n);
    parameters.set_to_member(parameter_profile, "repeat_penalty",     Parameters,     &parameters_t::repeat_penalty);
    parameters.set_to_member(parameter_profile, "min_p",              Parameters,     &parameters_t::min_p);
    parameters.set_to_member(parameter_profile, "slot_id",            Parameters,     &parameters_t::slot_id); // add
    parameters.set_to_member(parameter_profile, "temperature",        Parameters,     &parameters_t::temperature);
    parameters.set_to_member(parameter_profile, "n_predict",          Parameters,     &parameters_t::n_predict);
    parameters.set_to_member(parameter_profile, "stream",             Parameters,     &parameters_t::stream);
    parameters.set_to_member(parameter_profile, "ignore_eos",         Parameters,     &parameters_t::ignore_eos);
    parameters.set_to_member(parameter_profile, "penalize_nl",        Parameters,    &parameters_t::penalize_nl);

    completionBuffer.stream = Parameters.stream;

    this->parameters = Parameters;
}

std::string Completion::dumpJsonPayload() { // converts parameters_t to json string to sent
    // We dont need create a mutable object if treats about a simple json
    std::string json = "{";
    std::string is_stream = parameters.stream ? "true" : "false";
    json += std::string("\"stream\":") +  is_stream + ",";
    json += "\"slot_id\":" +       std::to_string(parameters.slot_id) + ",";
    json += "\"mirostat\":" +       std::to_string(parameters.mirostat) + ",";
    json += "\"mirostat_tau\":" +   std::to_string(parameters.mirostat_tau) + ",";
    json += "\"mirostat_eta\":" +   std::to_string(parameters.mirostat_eta) + ",";
    json += "\"frecuency_penalty\":" +  std::to_string(parameters.frecuency_penalty) + ",";
    json += "\"n_probs\":" +        std::to_string(parameters.n_probs) + ",";
    json += "\"grammar\":\"" +      normalizeText(parameters.grammar) + "\",";
    json += "\"presence_penalty\":" +   std::to_string(parameters.presence_penalty) + ",";
    json += "\"top_k\":" +          std::to_string(parameters.top_k) + ",";
    json += "\"top_p\":" +          std::to_string(parameters.top_p) + ",";
    json += "\"typical_p\":" +      std::to_string(parameters.typical_p) + ",";
    json += "\"tfz_z\":" +          std::to_string(parameters.tfz_z) + ",";
    json += "\"repeat_last_n\":" +  std::to_string(parameters.repeat_last_n) + ",";
    json += "\"repeat_penalty\":" + std::to_string(parameters.repeat_penalty) + ",";
    json += "\"temperature\":" +    std::to_string(parameters.temperature) + ",";
    json += "\"n_predict\":" +      std::to_string(parameters.n_predict) + ",";
    json += "\"stop\":[";
    for (size_t i = 0; i < parameters.stop.size(); ++i) {
        json += "\"" + parameters.stop[i] + "\"";
        if (i < parameters.stop.size() - 1) {
            json += ",";
        }
    }
    json += "],";
    json += "\"prompt\":\"" +       normalizeText(parameters.prompt) + "\"";


    json += "}";

    return json;
}

Response Completion::requestCompletion(
    const char* ipaddr, const char* completion_endpoint, const int16_t port) 
{
    #ifdef __WIN32__
    Terminal::setTitle("Completing...");
    #endif
    std::string json = dumpJsonPayload();
    Response res = Req.post(ipaddr, port, completion_endpoint, json, completionCallback, &completionBuffer);
    return res;
}

void Completion::setPrompt(std::string content){
    parameters.prompt = content;
}

std::string Completion::getPrompt(){
    return parameters.prompt;
}

void Completion::addStopWord(std::string word){
    parameters.stop.push_back(word);
}