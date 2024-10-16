#include "chat.hpp"

int main(int argc, char *argv[]) {
    std::string user_prompt =       "default";  // prompts.json
    std::string prompt_template =   "empty";    // tempaltes.json
    std::string param_profile =     "creative"; // params.json

    std::string ipaddr = DEFAULT_IP;
    int16_t port = DEFAULT_PORT;

    std::string save_folder_path = "saved_chats/";
    bool chat_guards = true;
    const char *input_arg = nullptr;

    // Handle args
    if ((input_arg = get_arg_value(argc, argv, "--prompt")) != NULL)
        user_prompt = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--param-profile")) != NULL)
        param_profile = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--prompt-template")) != NULL)
        prompt_template = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--ip")) != NULL)
        ipaddr = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--port")) != NULL)
        port = std::stoi(input_arg);

    if ((input_arg = get_arg_value(argc, argv, "--no-chat-guards")) != NULL)
        chat_guards = false;

    if ((input_arg = get_arg_value(argc, argv, "--help")) != NULL) {
        printCmdHelp();
        exit(0);
    }
    
    // Setup terminal behaviours
    Terminal::setupEncoding();

    // Init chat context
    Chat chatContext;
    chatContext.chat_guards = chat_guards;
    std::string userInput, director_input;

    // Load prompt template
    if(prompt_template == "empty"){
        if(!chatContext.loadPromptTemplates(prompt_template)){
            logging::error("Failed to load the template '%s' ! Using default empty.", prompt_template);
            Terminal::pause();
        }
    }

    // Load user prompt
    if(user_prompt == "default"){
        chatContext.setupDefaultActors();
    }else{
        if(!chatContext.loadUserPrompt(user_prompt))
            exit(1);   
    }

    // Load params profile
    if(!chatContext.loadParametersSettings(param_profile)){
        logging::error("Failed to load the param profile '%s' !", param_profile);
        Terminal::pause();
        exit(1);
    }

    chatContext.setupChatStopWords();

    // Control actor talk round
    bool onceAct = false;
    std::string previousActingActor;
    std::string currentActor = chatContext.getAssistantName();
    
    Terminal::resetColor();
    Terminal::clear();
    while (true) {
        chatContext.completionBuffer.buffer.clear();
        chatContext.draw();

        if(onceAct){
            currentActor = previousActingActor;
            onceAct = false;
        }else{
            previousActingActor = currentActor;
        }

        // Print user chat tag
        chatContext.printActorChaTag(chatContext.getUserName());
        Terminal::resetColor();
        
        signal(SIGINT, completionSignalHandler);

        std::getline(std::cin, userInput);

        // reset cin state during ctrl+c
        if (std::cin.fail() || std::cin.eof()) {
            std::cin.clear(); 
            continue;
        }

        // Command processing
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
            if (cmd == "/help" || cmd == "/h") {
                printChatHelp();
                Terminal::pause();
                continue;

            }else if (cmd == "/redraw") {
                Terminal::clear();
                continue;

                // custom actor 
            } else if (cmd == "/actor" || cmd == "/now" || cmd == "/a") {
                chatContext.addActor(arg, "assistant", ANSIColors::getRandColor());
                currentActor = arg;

                // switch between instruct mode (without chat tags)
            } else if (cmd == "/instruct" || cmd == "/a") {
                if(arg == "on"){
                    chatContext.instruct_mode = true;
                }
                if(arg == "off"){
                    chatContext.instruct_mode = false;
                }
                logging::success("Instruct mode %s", chatContext.instruct_mode?"ON":"OFF");
                Terminal::pause();
                continue;

                // act like an actor
            } else if (cmd == "/as") {
                chatContext.addActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(arg);
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(arg, director_input);

                // talk to an specific actor
            } else if (cmd == "/talkto") {
                chatContext.addActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(chatContext.getUserName());
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(chatContext.getUserName(), director_input);
                currentActor = arg;

                // lets narrator describes the context
            } else if (cmd == "/narrator") {
                chatContext.addActor("Narrator", "system", "yellow");
                currentActor = "Narrator";
                onceAct = true;

                // multiline insertion mode
            } else if (cmd == "/insert" || cmd == "/i") {
                userInput.clear();
                Terminal::setTitle("Multiline mode");
                std::string line;
                while(1){
                    std::getline(std::cin, line);
                    if(line == "EOL" or line == "eol") break;
                    userInput+= line + "\n";
                };
                chatContext.addNewMessage(chatContext.getUserName(), userInput);
            
                // as director you can put your prompt
            } else if (cmd == "/director" || cmd == "dir") {
                chatContext.addActor("Director", "system", "yellow");
                chatContext.printActorChaTag("Director");
                std::getline(std::cin, director_input);
                chatContext.addNewMessage("Director", director_input);

                // save chat in a file
            } else if (cmd == "/save") {
                if(!std::filesystem::exists(save_folder_path))
                    std::filesystem::create_directory(save_folder_path);

                std::string filename = arg.empty()? "/chat_" + getDate(): arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==std::string::npos) filename+=DEFAULT_FILE_EXTENSION;

                if(chatContext.saveConversation(save_folder_path + filename))
                    logging::success("Conversation saved as '%s'.", filename.c_str());
                else
                    logging::error("Problem saving the conversation.");       
                Terminal::pause();
                continue;

                // load from file
            } else if (cmd == "/load") {
                std::string filename = arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==std::string::npos) 
                    filename+=DEFAULT_FILE_EXTENSION;
                if (chatContext.loadSavedConversation(std::string(save_folder_path + filename)))
                    logging::success("Conversation from '%s' loaded.", filename.c_str());
                else
                    logging::error("Failed to load conversation!");
                currentActor = chatContext.getAssistantName();
                Terminal::pause();
                continue;

                // load prompt template in runtime
            } else if (cmd == "/stemplate") {
                if(chatContext.loadPromptTemplates(arg))
                    logging::success("Prompt template '%s' loaded.", arg);
                else
                    logging::error("Failed to load template!");
                Terminal::pause();
                continue;

                // load prompt template in runtime
            } else if (cmd == "/sprompt") {
                if(chatContext.loadUserPrompt(arg)){
                    logging::success("User prompt '%s' loaded.", arg);
                    user_prompt = arg;
                }else{
                    logging::error("Failed to user prompt!");
                }
                currentActor = chatContext.getAssistantName();
                Terminal::pause();
                continue;

                // load param profile in runtime
            } else if (cmd == "/sparam") {
                if(chatContext.loadParametersSettings(arg)){
                    chatContext.setupChatStopWords(); 
                    logging::success("Param profile '%s' loaded", arg.c_str());
                    param_profile = arg;
                }else{
                    logging::error("Failed to load param profile!");
                }
                Terminal::pause();
                continue;

                // undo only last message
            } else if (cmd == "/undolast") {
                chatContext.removeLastMessage(1);
                continue;

                // undo last completion and user message
            } else if (cmd == "/undo" || cmd == "/u") {
                chatContext.removeLastMessage(2);
                continue;

                // reset all chat historial
            } else if (cmd == "/reset" || cmd == "/clear" || cmd == "/cls") {
                chatContext.resetChatHistory();
                currentActor = chatContext.getAssistantName();
                continue;

                // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);

                // show current prompt
            } else if (cmd == "/lprompt" || cmd == "/history") {
                std::cout << ">Used prompt profile:" << user_prompt << std::endl;
                std::cout << ">Current prompt content:\n" << chatContext.dumpLegacyPrompt() << "\n";
                Terminal::pause();
                continue;
                
                // list current parameters
            } else if (cmd == "/lparams") {
                std::cout << ">Used param profile:" << param_profile << std::endl;
                std::cout << chatContext.dumpJsonPayload() << "\n\n";
                Terminal::pause();
                continue;

                // view actors 
            } else if (cmd == "/lactors") {
                std::cout << "> Current actors:\n";
                chatContext.listCurrentActors();
                Terminal::pause();
                continue;

                // reload params from file
            } else if (cmd == "/rparams") {
                chatContext.loadParametersSettings(param_profile);
                chatContext.setupChatStopWords(); 
                logging::success("Params reloaded!");Terminal::pause();
                continue;

                // reload template
            } else if (cmd == "/rtemplate") {
                chatContext.loadPromptTemplates(prompt_template);
                logging::success("Template reloaded");Terminal::pause();
                continue;

                // retry last completion
            } else if (cmd == "/retry" || cmd == "/r") {
                chatContext.removeLastMessage(1);
                chatContext.draw();

                // just continue the completation
            } else if (cmd == "/continue") {
                // ...
            } else {
                logging::error("Wrong command! Use '/help' for chat commands usage.");
                Terminal::pause();
                continue;
            }
        } else {
            chatContext.addNewMessage(chatContext.getUserName(), userInput);
        }

        // Print current actor chat tag
        chatContext.printActorChaTag(currentActor);

        // to complete
        chatContext.addNewMessage(currentActor, ""); 

        // Send for completion
        #ifdef __WIN32__
        Terminal::setTitle("Completing...");
        #endif
        Response res = chatContext.requestCompletion(ipaddr.c_str(), port);
        if (res.Status != HTTP_OK) {
            if (res.Status == HTTP_INTERNAL_ERROR)
                logging::critical("500 Internal server error!");
            if (res.Status == -1)
                logging::error("Error to connect, please check that server is running and try again.");
            if(!res.body.empty()) std::cout << "Server response body:" << res.body << std::endl;
            Terminal::setTitle("Completion in error.");
            Terminal::pause();
            chatContext.removeLastMessage(2);
        }else{
            chatContext.cureCompletionForChat();
            chatContext.updateMessageContent(chatContext.messages.size()-1, chatContext.completionBuffer.buffer);
        }
    }

    atexit(Terminal::resetColor);
    return 0;
}