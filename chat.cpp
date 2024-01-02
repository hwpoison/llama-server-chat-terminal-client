#include "chat.hpp"

int main(int argc, char *argv[]) {
    const char* my_prompt_profile = "default";
    const char* prompt_template_profile = "empty";
    const char* ipaddr = DEFAULT_IP;
    const char* param_profile = "default";
    bool chat_guards = true;
    int16_t port = DEFAULT_PORT;

    const char *input_arg = nullptr;
    
    // Handle args
    if ((input_arg = get_arg_value(argc, argv, "--my-prompt")) != NULL)
        my_prompt_profile = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--param-profile")) != NULL)
        param_profile = input_arg;

    if ((input_arg = get_arg_value(argc, argv, "--prompt-template")) != NULL)
        prompt_template_profile = input_arg;

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
    signal(SIGINT, completionSignalHandler);

    Chat chatContext;
    chatContext.guards = chat_guards;
    if(!chatContext.loadPromptTemplates(prompt_template_profile)){
        logging::error("Failed to load the template '%s' ! Using default empty.", prompt_template_profile);
        Terminal::pause();
    }

    if(!chatContext.loadUserPromptProfile(my_prompt_profile))
        exit(1);

    chatContext.loadParametersSettings(param_profile);
    chatContext.setupStopWords();

    std::string userInput, director_input;
    std::string save_folder_path = "saved_chats/";

    Terminal::resetColor();
    Terminal::clear();

    // Control actor talk round
    bool onceAct = false;
    std::string previousActingActor;
    std::string currentActingActor = chatContext.getAssistantName();

    while (true) {
        chatContext.completionBuffer.buffer.clear();
        chatContext.draw();

        if(onceAct){
            currentActingActor = previousActingActor;
            onceAct = false;
        }else{
            previousActingActor = currentActingActor;
        }

        // Print user chat tag
        chatContext.printActorChaTag(chatContext.getUserName());
        Terminal::resetColor();
        std::getline(std::cin, userInput);

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
            if (cmd == "/redraw") {
                Terminal::clear();
                continue;

                // custom actor 
            } else if (cmd == "/actor" || cmd == "/a") {
                chatContext.createActor(arg, "actor", ANSIColors::getRandColor());
                currentActingActor = arg;
                
                // act like an actor
            } else if (cmd == "/as") {
                chatContext.createActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(arg);
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(arg, director_input);

                // talk to an specific actor
            } else if (cmd == "/talkto") {
                chatContext.createActor(arg, "actor", ANSIColors::getRandColor());
                chatContext.printActorChaTag(chatContext.getUserName());
                std::getline(std::cin, director_input);
                chatContext.addNewMessage(chatContext.getUserName(), director_input);
                currentActingActor = arg;

                // lets narrator describes the context
            } else if (cmd == "/narrator") {
                chatContext.createActor("Narrator", "system", "yellow");
                currentActingActor = "Narrator";
                onceAct = true;

                // multiline insertion mode
            } else if (cmd == "/insert" || cmd == "/i") {
                userInput.clear();
                Terminal::setTitle("Multiline mode");
                std::string line;
                while(1){
                    std::getline(std::cin, line);
                    if(line == "EOF" or line == "eof") break;
                    userInput+= line + "\n";
                };
                chatContext.addNewMessage(chatContext.getUserName(), userInput);
            
                // as director you can put your prompt
            } else if (cmd == "/director" || cmd == "dir") {
                chatContext.createActor("Director", "system", "yellow");
                chatContext.printActorChaTag("Director");
                std::getline(std::cin, director_input);
                chatContext.addNewMessage("Director", director_input);

                // save chat in a file
            } else if (cmd == "/save") {
                if(!std::filesystem::exists(save_folder_path))
                    std::filesystem::create_directory(save_folder_path);

                std::string filename = arg.empty()? "/chat_" + getDate(): arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==string::npos) filename+=DEFAULT_FILE_EXTENSION;

                if(chatContext.saveConversation(save_folder_path + filename))
                    logging::success("Conversation saved as '%s'.", filename.c_str());
                else
                    logging::error("Problem saving the conversation.");       
                Terminal::pause();
                continue;

                // load from file
            } else if (cmd == "/load") {
                std::string filename = arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==string::npos) 
                    filename+=DEFAULT_FILE_EXTENSION;
                if (chatContext.loadSavedConversation(std::string(save_folder_path + filename)))
                    logging::success("Conversation from '%s' loaded.", filename.c_str());
                else
                    logging::error("Failed to load conversation!");
                Terminal::pause();
                continue;

                // load prompt template in runtime
            } else if (cmd == "/stemplate") {
                if(chatContext.loadPromptTemplates(arg.c_str()))
                    logging::success("Prompt template '%s' loaded.", arg.c_str());
                else
                    logging::error("Failed to load template!");
                Terminal::pause();
                continue;

                // load param profile in runtime
            } else if (cmd == "/sparam") {
                if(chatContext.loadParametersSettings(arg.c_str())){
                    std::string prevPrompt = chatContext.getPrompt();
                    chatContext.setupStopWords(); 
                    chatContext.setPrompt(prevPrompt);
                    logging::success("Param profile '%s' loaded", arg.c_str());
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
            } else if (cmd == "/reset" || cmd == "/clear") {
                chatContext.resetChatHistory();
                currentActingActor = chatContext.getAssistantName();
                continue;

                // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);
                
                // show current prompt
            } else if (cmd == "/lprompt" || cmd == "/history") {
                std::cout << ">Current prompt:\n\n" << chatContext.dumpFormatedPrompt()
                          << "\n";
                Terminal::pause();
                continue;

                // list current parameters
            } else if (cmd == "/lparams") {
                std::cout << "> Current params:\n\n";
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
                std::string prevPrompt = chatContext.getPrompt();
                chatContext.loadParametersSettings(param_profile);
                chatContext.setupStopWords(); 
                chatContext.setPrompt(prevPrompt);
                logging::success("Params reloaded!");Terminal::pause();
                continue;

                // reload template
            } else if (cmd == "/rtemplate") {
                chatContext.loadPromptTemplates(prompt_template_profile);
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
                logging::error("Wrong command!");
                Terminal::pause();
                continue;
            }
        } else {
            chatContext.addNewMessage(chatContext.getUserName(), userInput);
        }

        // Print current actor chat tag
        chatContext.printActorChaTag(currentActingActor);

        // Compose prompt
        chatContext.setPrompt(
            chatContext.dumpFormatedPrompt() + 
            chatContext.getActorTemplateFooter(currentActingActor)
        );

        // Send for completion
        #ifdef __WIN32__
        Terminal::setTitle("Completing...");
        #endif
        Response res = chatContext.requestCompletion(ipaddr, DEFAULT_COMPLETION_ENDPOINT, port);
        if (res.Status != 200) {
            if (res.Status == 500)
                logging::critical("500 Internal server error!");
            if (res.Status == -1)
                logging::error("Error to connect, please check server and try again.");
            chatContext.removeLastMessage();
            if(!res.body.empty()) cout << "Server response body:" << res.body << endl;
            Terminal::setTitle("Completion in error.");
            Terminal::pause();
        }else{
            chatContext.cureCompletionForChat();
            chatContext.addNewMessage(currentActingActor, chatContext.completionBuffer.buffer);
        }
    }

    atexit(Terminal::resetColor);
    return 0;
}