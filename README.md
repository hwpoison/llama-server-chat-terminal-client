# Llama.cpp Server Terminal Client

Welcome to the llama.cpp server terminal client, a versatile chat-style interface designed for interacting with the llama server. This interface seamlessly integrates both chat and command functionalities, providing you with control over the flow of your conversations. You can save and load conversations, setting your owns prompts and templates styles.

![Chat style view](screenshot.png)

## How to Use
#### Configuration files
**my_prompts.json**  contains all prompts setted with system prompt, actors and names.
**params.json**  contains all definitions for prompt adjustements and prompt templates styles.

#### Execultable Arguments:
-  --my-prompt *prompt name*           (default: default)
-  --param-profile* profile name*      (default: default)
-  --prompt-template *prompt template* (default: empty)
-  --url "htpp://....."                (default: "http://localhost:8080/completion")

#### Command support:
To input a command, simply insert `/*command*` followed by the desired command.

##### Command List:
- **narrator:** Lets to narrator generate a narration.
- **director:** Switch to director mode input.
- **actor:** Create a new character or use an existent into convesation and lets it talks. (ex: /actor Einstein).
- **as:** Pretend to be an actor and prompt it. (ex: /as Einstein)
- **talkto:** Talk to a determinated character. It will swtich the current talking actor. (ex: /talkto Monica)

<br>

- **retry / r:** Retry the last completion.
- **continue:** Continue the completion without intervention. (The assistant will continue talking)
- **undolast:** Undo only the last completion.
- **undo / u:** Undo the last completion and user input.

<br>
  
- **save (chatname):** Save the chat. (without extension)
- **load (chatname):** Load a previously saved chat.
- **redraw:** Redraw the chat content.
- **reset:** Reset the entire chat.
- **quit / q:** Exit the program.
- **lprompt:** Print the current prompt.
- **lparams:** Print the current parameters.
- **lactors:** Print current actors.
- **rparams** Reload current parameter profile.
- **rtemplate** Reload current template profile.

### Instructions to build

    git submodule init
    git submodule update
    make
