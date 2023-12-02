#include "chat.hpp"

int main(int argc, char *argv[]) {
    std::string my_prompt_profile;
    std::string prompt_template_profile;
    std::string completion_url;
    const char* param_profile = nullptr;
    const char *input_arg = nullptr;
    
    // Handle args
    if ((input_arg = get_arg_value(argc, argv, "--my-prompt")) != NULL) {
        my_prompt_profile = input_arg;
    } else {
        my_prompt_profile = "samantha";
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
        completion_url = input_arg;
    } else {
        completion_url = DEFAULT_COMPLETION_ENDPOINT;
    }

    if ((input_arg = get_arg_value(argc, argv, "--help")) != NULL) {
        printCmdHelp();
        exit(0);
    }

    if (my_prompt_profile.empty() || !param_profile || prompt_template_profile.empty()) {
        cout << "Wrong params" << endl;
        printCmdHelp();
        exit(1);
    }

    // Setup terminal behaviours
    Terminal::setupEncoding();
    signal(SIGINT, completionSignalHandler);

    Chat chatContext;
    chatContext.loadUserPromptProfile(my_prompt_profile.c_str());
    chatContext.loadPromptTemplates(prompt_template_profile.c_str());
    chatContext.loadParametersSettings(param_profile);
    chatContext.setupStopWords();

    std::string userInput;
    std::string director_input;
    std::string save_folder = "saved_chats/";

    // Initialize narrator and director actors
    chatContext.addNewActor("Narrator", "system", "yellow");
    chatContext.addNewActor("Director", "system", "yellow");

    // Set up the system prompt
    chatContext.addNewMessage("System", chatContext.getSystemPrompt());

    Terminal::resetColor();
    Terminal::clear();

    bool onceTime = false;
    std::string previousActingActor;
    std::string currentActingActor = chatContext.getAssistantName();

    while (true) {
        chatContext.completionBuffer.buffer = "";
        chatContext.draw();

        if(onceTime){
            currentActingActor = previousActingActor;
            onceTime = false;
        }else{
            previousActingActor = currentActingActor;
        }

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
                chatContext.addNewActor(arg, "actor", ANSIColors::getRandColor());
                currentActingActor = arg;
                
                // act like an actor
            } else if (cmd == "/as") {
                chatContext.addNewActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(arg);
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(arg, director_input);

                // talk to an specific actor
            } else if (cmd == "/talkto") {
                chatContext.addNewActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(chatContext.getUserName());
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(chatContext.getUserName(), director_input);
                currentActingActor = arg;

                // lets narrator describes the context
            } else if (cmd == "/narrator") {
                currentActingActor = "Narrator";
                onceTime = true;

                // as director you can put your prompt
            } else if (cmd == "/director" || cmd == "dir") {
                chatContext.printActorChaTag("Director");
                std::getline(std::cin, director_input);
                chatContext.addNewMessage("Director", director_input);

                // save chat in a .json
            } else if (cmd == "/save") {
                if (!arg.empty())
                    chatContext.saveConversation(std::string(save_folder + arg + ".json"));
                else
                    chatContext.saveConversation(std::string(save_folder + "/chat_" + getDate() + ".json"));
                Terminal::setTitle("Saved!");
                Terminal::clear();
                continue;
                // load from json
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
                chatContext.removeLastMessage(1);
                continue;

                // undo last completion and user message
            } else if (cmd == "/undo" || cmd == "/u") {
                chatContext.removeLastMessage(2);
                continue;

                // reset all chat historial
            } else if (cmd == "/reset") {
                chatContext.resetChatHistory();
                currentActingActor = chatContext.getAssistantName();
                continue;

                // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);

            } else if (cmd == "/history") {
                std::cout << "Current history:"
                          << chatContext.composeChatPrompt()
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;

                // show current prompt
            } else if (cmd == "/lprompt") {
                std::cout << "Current prompt:" << chatContext.composeChatPrompt()
                          << std::endl;
                std::cout << " " << endl;
                Terminal::pause();
                continue;

                // list current parameters
            } else if (cmd == "/lparams") {
                std::cout << "Current params:" << chatContext.dumpJsonPayload() << std::endl;
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
                chatContext.removeLastMessage(1);
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

        // Start to composing the prompt
        chatContext.printActorChaTag(currentActingActor);
        chatContext.generateChatPrompt(currentActingActor);
        if (chatContext.requestCompletion(completion_url)) {
            chatContext.addNewMessage(currentActingActor, chatContext.completionBuffer.buffer);
            cout << endl;
        }else{
            chatContext.removeLastMessage();
        }
    }

    atexit(Terminal::resetColor);
    return 0;
}