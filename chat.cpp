#include "chat.hpp"

int main(int argc, char *argv[]) {
    std::string myprompt;
    std::string endpoint_url;
    const char* param_profile = nullptr;
    std::string prompt_template_profile;
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
        prompt_template_profile = input_arg;
    } else {
        prompt_template_profile = "empty";
    }

    if ((input_arg = get_arg_value(argc, argv, "--url")) != NULL) {
        endpoint_url = input_arg;
    } else {
        endpoint_url = DEFAULT_COMPLETION_ENDPOINT;
    }

    if ((input_arg = get_arg_value(argc, argv, "--help")) != NULL) {
        printCmdHelp();
        exit(0);
    }

    if (myprompt.empty() || !param_profile || prompt_template_profile.empty()) {
        cout << "Wrong params" << endl;
        printCmdHelp();
        exit(1);
    }

    // Setup terminal behaviours
    Terminal::setupEncoding();
    
    Chat chatContext;
    chatContext.loadUserPromptProfile(myprompt.c_str());
    chatContext.loadPromptTemplates(prompt_template_profile.c_str());
    chatContext.loadParametersSettings(param_profile);
    chatContext.setupStopWords();

    std::string userInput;
    std::string director_input;
    std::string save_folder = "saved_chats/";

    signal(SIGINT, chatContext.completionSignalHandler);

    // Initialize narrator and director actors
    chatContext.addNewActor({"Narrator", "yellow", "narrator"});
    chatContext.addNewActor({"Director", "yellow", "director"});

    // Set up the system prompt
    chatContext.addNewMessage("system", chatContext.getPromptSystem()+"\n");

    Terminal::resetColor();
    Terminal::clear();

    while (true) {
        chatContext.completionBuffer.buffer = "";
        chatContext.draw();

        // User input
        chatContext.printActorChaTag(chatContext.getUserName());
        Terminal::resetColor();
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
                // custom actor 
            } else if (cmd == "/actor" || cmd == "/a") {
                chatContext.addNewActor((actor_t){arg, ANSIColors::getRandColor(), "actor"});
                chatContext.printActorChaTag(arg.c_str());
                chatContext.setPrompt(chatContext.craftChatPromp());
                chatContext.addPrompt(arg + ":");
                if (chatContext.requestCompletion())
                    chatContext.addNewMessage(arg.c_str(), chatContext.completionBuffer.buffer);
                continue;
            } else if (cmd == "/iam") {
                chatContext.addNewActor((actor_t){arg, ANSIColors::getRandColor(), "actor"});
                chatContext.printActorChaTag(arg.c_str());
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(arg.c_str(), director_input);
                // lets narrator describes the context
            } else if (cmd == "/narrator") {
                chatContext.printActorChaTag("Narrator");
                chatContext.setPrompt(chatContext.craftChatPromp());
                chatContext.addPrompt(std::string("Narrator")+":");

                if (chatContext.requestCompletion()) {
                    chatContext.addNewMessage("Narrator", chatContext.completionBuffer.buffer);
                    cout << endl; // chage by eos!
                }else{
                    chatContext.removeMessage();
                }
                continue;
                // as director you can put your prompt
            } else if (cmd == "/director" || cmd == "dir") {
                chatContext.printActorChaTag("Director");
                std::getline(std::cin, director_input);
                chatContext.addNewMessage("director", director_input);
                // save chat in a .txt
            } else if (cmd == "/save") {
                if (!arg.empty())
                    chatContext.saveConversation(std::string(save_folder + arg + ".json"));
                else
                    chatContext.saveConversation(std::string(save_folder + "/chat_" + getDate() + ".json"));

                Terminal::setTitle("Saved!");
                Terminal::clear();
                continue;
            } else if (cmd == "/load") {
                if (chatContext.loadSavedConversation(std::string(save_folder + arg + ".json"))) {
                    Terminal::setTitle("Conversation loaded!");
                    Terminal::clear();
                    continue;
                } else {
                    Terminal::setTitle("Failed to load conversation!");
                    Terminal::pause();
                    continue;
                }
                // undo only last message
            } else if (cmd == "/undolast") {
                chatContext.removeMessage(1);
                continue;
                // undo last completion and user message
            } else if (cmd == "/undo" || cmd == "/u") {
                chatContext.removeMessage(2);
                continue;
                // reset all chat historial
            } else if (cmd == "/reset") {
                chatContext.resetChatHistory();
                continue;
                // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);
            } else if (cmd == "/history") {
                std::cout << "Current history:"
                          << chatContext.craftChatPromp()
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;
                // show current prompt
            } else if (cmd == "/lprompt") {
                std::cout << "Current prompt:" << chatContext.getCurrentPrompt()
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;
                // list current parameters
            } else if (cmd == "/lparams") {
                std::cout << "Current params:" << chatContext.jsonPayload() << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;
            // view actors 
            } else if (cmd == "/lactors") {
                chatContext.listCurrentActors();
                Terminal::pause();
                continue;
                // reload params from file
            } else if (cmd == "/rparams") {
                std::string prevPrompt = chatContext.getPrompt();
                chatContext.loadParametersSettings(param_profile);
                chatContext.setupStopWords(); 
                chatContext.setPrompt(prevPrompt);
                Terminal::setTitle("Params reloaded");
                continue;
            } else if (cmd == "/rtemplate") {
                chatContext.loadPromptTemplates(prompt_template_profile.c_str());
                // setupChatGuards(chatContext);
                Terminal::setTitle("Template reloaded");
                continue;
                // retry last completion
            } else if (cmd == "/retry" || cmd == "/r") {
                chatContext.removeMessage(1);
                chatContext.draw();
            } else if (cmd == "/continue") {
                // ...

            } else {
                Terminal::setTitle("Wrong cmd!");
                continue;
            }
        } else {
            chatContext.addNewMessage(chatContext.getUserName(), userInput);
        }

        // Default Assistant completion
        chatContext.printActorChaTag(chatContext.getAssistantName());
        chatContext.setPrompt(chatContext.craftChatPromp());
        chatContext.addPrompt(std::string(chatContext.getAssistantName()) + ":");

        if (chatContext.requestCompletion()) {
            chatContext.addNewMessage(chatContext.getAssistantName(), chatContext.completionBuffer.buffer);
            cout << endl; // chage by eos!
        }else{
            chatContext.removeMessage();
        }
    }

    atexit(Terminal::resetColor);
    return 0;
}