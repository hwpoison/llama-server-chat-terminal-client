# Llama.cpp/server Terminal Client

This is a lightweight terminal interface alternative for llama.cpp server following an outline of a chat script.

* Many commands for manipulate the conversation flow and also debug it.
* Prompt templates and Personal prompts can be created and customized.
* Load/Save conversation.
* Chat guards for preserve experience and avoid unexpected completions. 
* Compilable for Windows and Linux.

*llama.cpp project server:* https://github.com/ggerganov/llama.cpp/tree/master/examples/server

![Chat style view](/screenshots/screenshot.png)

Prompt templates can be added and customized.

![Chat style view](/screenshots/prompt_templates.png)
![Chat style view](/screenshots/screenshot2.png)

## How to Use
#### Configuration files:
* **my_prompts.json**  Contains all personal prompts definitions. Supports system prompt and an actor system.
* **params.json**  Contains all definitions for prompt format/template and the parameters profiles.

#### Execultable Arguments:
-  --my-prompt *prompt name*            (default: default)
-  --param-profile *profile name*       (default: default)
-  --prompt-template *prompt template*  (default: empty)
-  --no-chat-guards: Disable the chat guards (default: false)
-  --ip <ip address>                    (default: 127.0.0.1)
-  --port <port>                        (default: 8080)

#### Command support:
To input a command, simply insert `/command` followed by the desired command.

##### Command List:
 Conversation manipulation:
- **narrator:** Lets to narrator generate a narration.
- **director:** Switch to director mode input.
- **actor:** Create a new character or use an existent into convesation and lets it talks. (ex: /actor Einstein).
- **as:** Pretend to be an actor and prompt it. (ex: /as Einstein)
- **talkto:** Talk to a determinated character. It will swtich the current talking actor. (ex: /talkto Monica)
- **insert:** Multiline mode insertion. For finish it and submit write "EOF" or "eof" and then enter.
- **retry / r:** Retry the last completion.
- **continue:** Continue the completion without intervention. (The assistant will continue talking)
- **undolast:** Undo only the last completion.
- **undo / u:** Undo the last completion and user input.

Conversation saving:
- **save (chatname):** Save the chat. (without extension)
- **load (chatname):** Load a previous saved chat.

Manage configurations:
- **redraw:** Redraw the chat content.
- **reset:** Reset the entire chat.
- **quit / q:** Exit the program.
- **lprompt:** Print the current prompt.
- **lactors:** Print current actors.
- **lparams:** Print the current parameters.
- **rparams** Reload current parameter profile.
- **sparam (parameter profile name)** Load and set param profile in runtime.
- **rtemplate** Reload current template profile.
- **stemplate (template name)** Load and set prompt template in runtime.

#### About the chat guards:
The chat guards adds the prompt template tokens into stop words array. Also all the posible variations for the actors chat tags. 

### Instructions to build

    git submodule init
    git submodule update
    make

## **Tested on**

<table><tbody><tr><td>Windows</td><td>GCC</td><td>gcc (x86_64-posix-seh-rev1, Built by MinGW-Builds project) 13.1.0</td></tr><tr><td>Linux</td><td>GCC</td><td>gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0</td></tr></tbody></table>
