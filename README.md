# Llama.cpp Server Terminal Client

Welcome to the llama.cpp server terminal client, a versatile chat-style interface designed for interacting with the llama server. This interface seamlessly integrates both chat and command functionalities, providing you with control over the flow of your conversations. You can save and load conversations, setting your owns prompts and templates styles.

![Chat style view](screenshot.png)

## How to Use
#### Configuration files
**my_prompts.json**  contains all prompts setted with system prompt, actors and names.
**params.json**  contains all definitions for prompt adjustements and prompt templates styles.

#### Execultable Arguments:
 --my-prompt *prompt name*
 --param-profile* profile name*
 --prompt-template *prompt template*


#### Command support:
To input a command, simply insert `/*command*` followed by the desired command.

##### Command List:
- **narrator:** Generate Narrator content
- **director:** Switch to director mode input
- **retry / r:** Retry the last completion
- **continue:** Continue the completion without intervention

- **undolast:** Undo only the last completion
- **undo / u:** Undo the last completion and user input

- **save (chatname):** Save the chat (without extension)
- **load (chatname):** Load a previously saved chat
- **redraw:** Redraw the chat content
- **reset:** Reset the entire chat
- **quit / q:** Exit the program
- **prompt:** Print the current prompt
- **params:** Print the current parameters
