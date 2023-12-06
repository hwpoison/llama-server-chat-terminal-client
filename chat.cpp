#include "chat.hpp"

int main(int argc, char *argv[]) {
    const char* my_prompt_profile = "samantha";
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
    if(!chatContext.loadUserPromptProfile(my_prompt_profile))
        exit(1);

    chatContext.loadPromptTemplates(prompt_template_profile);
    chatContext.loadParametersSettings(param_profile);
    chatContext.setupStopWords();

    std::string userInput, director_input;
    std::string save_folder = "saved_chats/";

    // Create the director and narrator actors
    chatContext.createActor("Director", "system", "yellow");
    chatContext.createActor("Narrator", "system", "yellow");

    // Set up the system prompt
    chatContext.addNewMessage("System", chatContext.getSystemPrompt());

    Terminal::resetColor();
    Terminal::clear();

    bool onceAct = false;
    std::string previousActingActor;
    std::string currentActingActor = chatContext.getAssistantName();

    while (true) {
        chatContext.completionBuffer.buffer = "";
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
                currentActingActor = "Narrator";
                onceAct = true;

                // as director you can put your prompt
            } else if (cmd == "/director" || cmd == "dir") {
                
                chatContext.printActorChaTag("Director");
                std::getline(std::cin, director_input);
                chatContext.addNewMessage("Director", director_input);

                // save chat in a file
            } else if (cmd == "/save") {
                std::string filename = arg.empty()? "/chat_" + getDate(): arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==string::npos) filename+=DEFAULT_FILE_EXTENSION;
                chatContext.saveConversation(save_folder + filename);
                logging::success("Conversation saved as \"%s\".", filename.c_str());Terminal::pause();
                Terminal::clear();
                continue;

                // load from file
            } else if (cmd == "/load") {
                std::string filename = arg;
                if(arg.find(DEFAULT_FILE_EXTENSION)==string::npos) filename+=DEFAULT_FILE_EXTENSION;
                if (chatContext.loadSavedConversation(std::string(save_folder + filename))) {
                    logging::success("Conversation loaded from \"%s\".", filename.c_str());Terminal::pause();
                    Terminal::clear();
                    continue;
                } else {
                    logging::error("Failed to load conversation!");Terminal::pause();
                    continue;
                }

                // load prompt template
            } else if (cmd == "/stemplate") {
                if(chatContext.loadPromptTemplates(arg.c_str())){
                    logging::success("Template loaded from \"%s\".", arg.c_str());Terminal::pause();
                    Terminal::clear();
                    continue;
                } else {
                    logging::error("Failed to load template!");Terminal::pause();
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
            } else if (cmd == "/reset" || cmd == "/clear") {
                chatContext.resetChatHistory();
                currentActingActor = chatContext.getAssistantName();
                continue;

                // exit from program
            } else if (cmd == "/quit" || cmd == "/q") {
                exit(0);
                
            } else if (cmd == "/history") {
                std::cout << "> Current history:\n"
                          << chatContext.dumpFormatedPrompt()
                          << "\n";
                Terminal::pause();
                continue;

                // show current prompt
            } else if (cmd == "/lprompt") {
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

            } else if (cmd == "/continue") {
                // ...

            } else {
                logging::error("Wrong cmd!");
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
                logging::critical("500 Internal serve error!");
            else
                logging::error("Error to connect, please check server and try again.");
            chatContext.removeLastMessage();
            cout << "ERROR:" << res.body << endl;
            Terminal::pause();
        }else{
            chatContext.cureCompletionForChat();
            chatContext.addNewMessage(currentActingActor, chatContext.completionBuffer.buffer);
        }
    }

    atexit(Terminal::resetColor);
    return 0;
}