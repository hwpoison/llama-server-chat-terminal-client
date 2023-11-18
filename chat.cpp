#include "chat.hpp"

void completionSignalHandler(int signum) {
    stopCompletionFlag = true;
    signal(SIGINT, SIG_DFL);
}

std::string craftToSendPrompt(chat_history_t &chatHistory,
                              prompt_template_t &promptTemplate) {
    std::string newString;
    for (const chat_entry_t &entry : chatHistory) {
        if (entry.actor->role == "user") {
            newString += promptTemplate.begin_user;
            newString += entry.actor->name + ":";
            newString += entry.message_content;
            newString += promptTemplate.end_user;
        } else if (entry.actor->role == "system") {
            newString += promptTemplate.begin_system;
            newString += entry.message_content;
            newString += promptTemplate.end_system;
        } else if (entry.actor->role == "assistant") {
            newString += promptTemplate.begin_assistant;
            newString += entry.actor->name + ":";
            newString += entry.message_content;
            newString += promptTemplate.eos;
        } else {
            newString += entry.actor->name + ":";
            newString += entry.message_content;
        }
    }
    return newString;
}

bool completionCallback(const std::string &chunck) {
    if (stopCompletionFlag) {
        cout << endl << "Completion stopped by the user!..." << endl;
        stopCompletionFlag = false;
        return false;
    }
    const char *content_[] = {"content", "\0"};
    // Extract and parse completion response data
    std::string completionData = chunck;
    if (completionStream) {
        size_t dataPos = chunck.find("data: ");
        if (dataPos != string::npos) {
            completionData = chunck.substr(dataPos + 6);
        } else {
            return true;
        }
    } else {
        size_t dataPos = chunck.find("\"content\":");
        if (dataPos == string::npos) return true;
    }

    // Parse result
    simplejson::json result(simplejson::json::read(completionData.c_str()));

    // Check end of completion
    const char *end_of_c[] = {"stop", "\0"};
    if (result.get_bool(end_of_c)) {
        if (!completionStream) {
            std::cout << result.get_str(content_);
        }
        const char *timing[] = {"timings", "predicted_per_second", "\0"};
        double tokensPerSecond = result.get_real(timing);
        // show token speed in the terminal title
        char windowTitle[100];
        std::snprintf(windowTitle, sizeof(windowTitle), "%.1f t/s\n",
                      tokensPerSecond);
        Terminal::setTitle(windowTitle);
        return false;
    }

    std::string token = result.get_str(content_);
    std::cout << token;  // write the token in terminal
    completionBuffer.append(token);
    return true;
}

std::string craftPayload(const parameters_t &params) {
    // We dont need create a mutable object if treats about a simple json
    std::string json = "{";

    json += "\"stream\":"s +        (params.stream ? "true" : "false") + ",";
    json += "\"mirostat\":" +       std::to_string(params.mirostat) + ",";
    json += "\"mirostat_tau\":" +   std::to_string(params.mirostat_tau) + ",";
    json += "\"mirostat_eta\":" +   std::to_string(params.mirostat_eta) + ",";
    json +=
        "\"frecuency_penalty\":" +  std::to_string(params.frecuency_penalty) + ",";
    json += "\"n_probs\":" +        std::to_string(params.n_probs) + ",";
    json += "\"grammar\":\"" +      normalizeText(params.grammar) + "\",";
    json +=
        "\"presence_penalty\":" +   std::to_string(params.presence_penalty) + ",";
    json += "\"top_k\":" +          std::to_string(params.top_k) + ",";
    json += "\"top_p\":" +          std::to_string(params.top_p) + ",";
    json += "\"typical_p\":" +      std::to_string(params.typical_p) + ",";
    json += "\"tfz_z\":" +          std::to_string(params.tfz_z) + ",";
    json += "\"repeat_last_n\":" +  std::to_string(params.repeat_last_n) + ",";
    json += "\"repeat_penalty\":" + std::to_string(params.repeat_penalty) + ",";
    json += "\"temperature\":" +    std::to_string(params.temperature) + ",";
    json += "\"n_predict\":" +      std::to_string(params.n_predict) + ",";
    json += "\"stop\":[";
    for (size_t i = 0; i < params.stop.size(); ++i) {
        json += "\"" + params.stop[i] + "\"";
        if (i < params.stop.size() - 1) {
            json += ",";
        }
    }
    json += "],";
    json += "\"prompt\":\"" +       normalizeText(params.prompt) + "\"";
    
    json += "}";

    return json;
}

parameters_t loadParametersSettings(const char *profile_name) {
    // Retrieve parsed parameters from parameters.json
    simplejson::json parameters = simplejson::json("params.json");

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
    parameters.set_to_member(parameter_profile, "slot_id",            Parameters,     &parameters_t::slot_id); // add
    parameters.set_to_member(parameter_profile, "temperature",        Parameters,     &parameters_t::temperature);
    parameters.set_to_member(parameter_profile, "n_predict",          Parameters,     &parameters_t::n_predict);
    parameters.set_to_member(parameter_profile, "stream",             Parameters,     &parameters_t::stream);

    return Parameters;
}

void registerChatEntry(chat_history_t &chatHistory, actor_t &actorInfo,
                       std::string content) {
    chat_entry_t newEntry;
    newEntry.actor = &actorInfo;
    newEntry.message_content = content;
    chatHistory.push_back(newEntry);
}

bool loadSavedChat(std::string file_path, chat_history_t &chatHistory,
                   my_prompt_t &myPrompt) {
    cout << "Loaded " << endl;
    simplejson::json saved_file = simplejson::json(file_path.c_str());

    chatHistory.clear();
    yyjson_val *hits = yyjson_obj_get(saved_file.get_current_root(), "history");
    size_t idx, max;
    yyjson_val *hit;
    yyjson_arr_foreach(hits, idx, max, hit) {
        const char *alias = yyjson_get_str(yyjson_obj_get(hit, "alias"));
        const char *actor_name = yyjson_get_str(yyjson_obj_get(hit, "actor_name"));
        const char *role = yyjson_get_str(yyjson_obj_get(hit, "role"));
        const char *content = yyjson_get_str(yyjson_obj_get(hit, "content"));

        if (myPrompt.actor_list.count(std::string(alias)) != 0) {
            registerChatEntry(chatHistory, myPrompt.actor_list[std::string(alias)],
                              content);
        } else {
            actor_t newActor;
            newActor.name = actor_name;
            newActor.tag_color = "blue";
            newActor.role = role;
            registerChatEntry(chatHistory, newActor, content);
        }
    }
    return true;
}

void saveChatHistory(std::string filename, chat_history_t &chatHistory) {
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    // History array
    yyjson_mut_val *historyArray = yyjson_mut_arr(doc);
    for (const chat_entry_t &entry : chatHistory) {
        yyjson_mut_val *historyEntry = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_str(doc, historyEntry, "alias",
                               entry.actor->alias.c_str());
        yyjson_mut_obj_add_str(doc, historyEntry, "actor_name",
                               entry.actor->name.c_str());
        yyjson_mut_obj_add_str(doc, historyEntry, "role",
                               entry.actor->role.c_str());
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

prompt_template_t loadPromptTemplates(const char *prompt_style_name) {
    prompt_template_t promptTemplate;
    const char *key_name = "prompt_templates";
    simplejson::json template_base = simplejson::json("params.json");

    const char *PROMPT_TYPE_[] = {key_name, prompt_style_name, "TYPE", "\0"};
    promptTemplate.prompt_type = template_base.get_str(PROMPT_TYPE_);

    const auto loadSeqToken = [&](const char *key, const char *field_key) {
        const char *keys[] = {key_name, prompt_style_name, key, field_key, "\0"};
        return template_base.get_str(keys);
    };
    promptTemplate.begin_system = loadSeqToken("SEQ", "B_SYS");
    promptTemplate.end_system = loadSeqToken("SEQ", "E_SYS");
    promptTemplate.begin_user = loadSeqToken("SEQ", "B_USER");
    promptTemplate.end_user = loadSeqToken("SEQ", "E_USER");
    promptTemplate.begin_assistant = loadSeqToken("SEQ", "B_ASSISTANT");
    promptTemplate.eos = loadSeqToken("SEQ", "EOS");

    return promptTemplate;
}

my_prompt_t loadUserPrompts(const char *prompt_name) {
    my_prompt_t myPrompt;

    simplejson::json prompt_file = simplejson::json("my_prompts.json");
    yyjson_val *my_prompt =
        yyjson_obj_get(prompt_file.get_current_root(), prompt_name);  // daryl
    yyjson_val *system = yyjson_obj_get(my_prompt, "system");  // daryl->system

    // load system prompt
    myPrompt.system_prompt = yyjson_get_str(system);
    myPrompt.actor_list["system"] = {"system", "System", "green_bc", "system"};

    // load actors list
    yyjson_val *obj = yyjson_obj_get(my_prompt, "actors");  // daryl->actors

    yyjson_obj_iter iter;
    yyjson_obj_iter_init(obj, &iter);
    yyjson_val *key, *val;
    while ((key = yyjson_obj_iter_next(&iter))) {
        val = yyjson_obj_iter_get_val(key);
        myPrompt.actor_list[yyjson_get_str(key)] = {
            yyjson_get_str(key),                          // alias
            yyjson_get_str(yyjson_obj_get(val, "name")),  // name
            yyjson_get_str(yyjson_obj_get(val, "color")),
            yyjson_get_str(yyjson_obj_get(val, "role"))
        };
    }

    return myPrompt;
}

void printActorTag(actor_t &actorInfo) {
    std::cout << ANSIColors::getColorCode(actorInfo.tag_color) << actorInfo.name
              << ANSI_COLOR_RESET << ":";
}

bool popChatMessage(chat_history_t &chatHistory, int num = 1) {
    if (chatHistory.size() == 2) {
        chatHistory.pop_back();
        return true;
    };
    if (chatHistory.size() > 2) {
        for (int i = 0; i < num; i++) chatHistory.pop_back();
        return true;
    }
    return false;
}

void drawChatContent(chat_history_t &chatHistory) {
    Terminal::clear();
    for (const chat_entry_t &entry : chatHistory) {
        printActorTag(*entry.actor);
        cout << entry.message_content << endl;
        if (entry.actor->role == "system") cout << endl;
    }
}

bool requestCompletion(std::string endpoint_url, parameters_t &Parameters,
                       chat_history_t &chatHistory) {
    Terminal::setTitle("Completing...");
    std::string json = craftPayload(Parameters);
    httpRequest Req;
    Response res = Req.post(endpoint_url, json.c_str(), completionCallback);

    if (res.Status != 200) {
        if (res.Status == 500) {
            cout << endl << json << endl;
            cout << ANSI_RED_BC << "[Error] Server Internal error."
                 << ANSI_COLOR_RESET << endl;
        } else {
            cout << ANSI_RED_BC
                 << "[Error] Please check server connection and try again."
                 << ANSI_COLOR_RESET << endl;
        }
        chatHistory.pop_back();
        Terminal::pause();
        return false;
    }

    // delete double breakline and space at the start
    completionBuffer.erase(0, completionBuffer.find_first_not_of(" "));
    while (!completionBuffer.empty() && completionBuffer.back() == '\n') {
        completionBuffer.erase(completionBuffer.size() - 1);
    }
    return true;
}

void setupChatGuards(settings_t &chatContext) {
    // Setup stop words
    for (const auto &[key, value] : chatContext.my_prompt.actor_list) {
        chatContext.parameters.stop.push_back(value.name + ":");
        chatContext.parameters.stop.push_back(toUpperCase(value.name + ":"));
    }

    if (chatContext.prompt_template.begin_user != "")
        chatContext.parameters.stop.push_back(
            normalizeText(chatContext.prompt_template.begin_user));
    if (chatContext.prompt_template.end_user != "")
        chatContext.parameters.stop.push_back(
            normalizeText(chatContext.prompt_template.end_user));
}

int main(int argc, char *argv[]) {
    std::string myprompt;
    std::string endpoint_url;
    std::string param_profile;
    std::string prompt_template_t;
    const char *input_arg = nullptr;

    // Handle args
    if ((input_arg = get_arg_value(argc, argv, "--my-prompt")) != NULL) {
        myprompt = input_arg;
    } else {
        myprompt = "samantha";
    }

    if ((input_arg = get_arg_value(argc, argv, "--param-profile")) != NULL) {
        param_profile = input_arg;
    } else {
        param_profile = "default";
    }

    if ((input_arg = get_arg_value(argc, argv, "--prompt-template")) != NULL) {
        prompt_template_t = input_arg;
    } else {
        prompt_template_t = "empty";
    }

    if ((input_arg = get_arg_value(argc, argv, "--url")) != NULL) {
        endpoint_url = input_arg;
    } else {
        endpoint_url = "http://localhost:8080/completion";
    }

    if ((input_arg = get_arg_value(argc, argv, "--help")) != NULL) {
        printCmdHelp();
        exit(0);
    }

    if (myprompt.empty() || param_profile.empty() || prompt_template_t.empty()) {
        cout << "Wrong params" << endl;
        printCmdHelp();
        exit(1);
    }

    // Setup terminal behaviours
    Terminal::setupEncoding();
    signal(SIGINT, completionSignalHandler);

    settings_t chatContext;
    chat_history_t chatHistory;
    std::string userInput;
    std::string director_input;
    std::string save_folder = "saved_chats/";

    // Load params files
    chatContext.my_prompt = loadUserPrompts(myprompt.c_str());
    chatContext.parameters = loadParametersSettings(param_profile.c_str());
    chatContext.prompt_template = loadPromptTemplates(prompt_template_t.c_str());

    // Initialize narrator and director actors
    chatContext.my_prompt.actor_list["narrator"] = {"Narrator", "narrator",
                                                    "yellow"
                                                   };
    chatContext.my_prompt.actor_list["director"] = {"Director", "director",
                                                    "green_bc"
                                                   };

    setupChatGuards(chatContext);

    // Set up the system prompt
    registerChatEntry(chatHistory, chatContext.my_prompt.actor_list["system"],
                      chatContext.my_prompt.system_prompt);

    completionStream = chatContext.parameters.stream;

    Terminal::resetColor();
    Terminal::clear();

    while (true) {
        completionBuffer = "";
        drawChatContent(chatHistory);

        // User input
        printActorTag(chatContext.my_prompt.actor_list["user"]);
        std::getline(std::cin, userInput);

        // Check if there is some command
        if (userInput[0] == '/') {
            size_t space = userInput.find(" ");
            std::string cmd = "";
            std::string arg = "";
            if (space != std::string::npos) {
                cmd = userInput.substr(0, space);
                arg = userInput.substr(space + 1);
            } else {
                cmd = userInput;
            }
            if (cmd == "/redraw") {
                Terminal::clear();
                continue;
                // narrator mode, just generate the tag and wait for completion.
            } else if (cmd == "/narrator") {
                printActorTag(chatContext.my_prompt.actor_list["narrator"]);
                chatContext.parameters.prompt =
                    craftToSendPrompt(chatHistory, chatContext.prompt_template);
                chatContext.parameters.prompt +=
                    chatContext.my_prompt.actor_list["narrator"].name + ":";
                if (requestCompletion(endpoint_url, chatContext.parameters,
                                      chatHistory)) {
                    registerChatEntry(chatHistory,
                                      chatContext.my_prompt.actor_list["narrator"],
                                      completionBuffer);
                }
                continue;
                // director mode, user can write what happens.
            } else if (cmd == "/director" || cmd == "dir") {
                printActorTag(chatContext.my_prompt.actor_list["director"]);
                std::getline(std::cin, director_input);
                registerChatEntry(chatHistory,
                                  chatContext.my_prompt.actor_list["director"],
                                  director_input);
                // save chat in a .txt
            } else if (cmd == "/save") {
                if (!arg.empty())
                    saveChatHistory(std::string(save_folder + arg + ".json"),
                                    chatHistory);
                else
                    saveChatHistory(
                        std::string(save_folder + "/chat_" + getDate() + ".json"),
                        chatHistory);

                Terminal::setTitle("Saved!");
                Terminal::clear();
                continue;
            } else if (cmd == "/load") {
                if (loadSavedChat(std::string(save_folder + arg + ".json"), chatHistory,
                                  chatContext.my_prompt)) {
                    Terminal::setTitle("Loaded!");
                    Terminal::clear();
                    continue;
                } else {
                    Terminal::setTitle("Failed to load!");
                    Terminal::pause();
                    continue;
                }
                // undo only last message
            } else if (cmd == "/undolast") {
                popChatMessage(chatHistory, 1);
                continue;
                // undo last completion and user message
            } else if (cmd == "/undo" || cmd == "/u") {
                popChatMessage(chatHistory, 2);
                continue;
                // reset all chat historial
            } else if (cmd == "/reset") {
                if (chatHistory.size() > 1)
                    chatHistory.erase(chatHistory.begin() + 1, chatHistory.end());
                continue;
                // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);
            } else if (cmd == "/history") {
                std::cout << "Current history:"
                          << craftToSendPrompt(chatHistory, chatContext.prompt_template)
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;
                // show current prompt
            } else if (cmd == "/prompt") {
                std::cout << "Current prompt:" << chatContext.parameters.prompt
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;
                // list current parameters
            } else if (cmd == "/params") {
                std::cout << "Current params:" << craftPayload(chatContext.parameters)
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;
                // reload params from file
            } else if (cmd == "/rparams") {
                chatContext.parameters = loadParametersSettings(param_profile.c_str());
                setupChatGuards(chatContext);
                Terminal::setTitle("Params reloaded");
                continue;
            } else if (cmd == "/rtemplate") {
                chatContext.parameters = loadParametersSettings(param_profile.c_str());
                setupChatGuards(chatContext);
                Terminal::setTitle("Template reloaded");
                continue;
                // retry last completion
            } else if (cmd == "/retry" || cmd == "/r") {
                popChatMessage(chatHistory, 1);
                drawChatContent(chatHistory);
            } else if (cmd == "/continue") {
                // ...

            } else {
                Terminal::setTitle("Wrong cmd!");
                continue;
            }
        } else {
            registerChatEntry(chatHistory, chatContext.my_prompt.actor_list["user"],
                              userInput);
        }

        // Default Assistant completion
        printActorTag(chatContext.my_prompt.actor_list["assistant"]);
        chatContext.parameters.prompt =
            craftToSendPrompt(chatHistory, chatContext.prompt_template) +
            chatContext.my_prompt.actor_list["assistant"].name + ":";

        if (requestCompletion(endpoint_url, chatContext.parameters, chatHistory)) {
            registerChatEntry(chatHistory,
                              chatContext.my_prompt.actor_list["assistant"],
                              completionBuffer);
            cout << chatContext.prompt_template.eos;
        }
    }
    atexit(Terminal::resetColor);
    return 0;
}