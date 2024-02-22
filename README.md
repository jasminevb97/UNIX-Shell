# UNIX-Shell

## Project Description
This project involves creating a custom UNIX Shell using C programming language. The shell functions similar to ```Bash``` which incorporates features and functionalities that demonstrate understanding of system-lvel programming, process management, and file manipulation in UNIX-based systems.

## Key Features
- **Command Function**: Ability to execute external commands found in the user's PATH.
- **Pipelines**: Support for command pipelines, allowing the output of one command to serve as input to another.
- **File Redirection**: Implement input(`<`), output(`>`), and append(`>>`) redirection, emabling users to redirect command input and output to and from files.
- **Builtin Commands**: Includes shell builtin commands.

## What I learned
- **Process Management**: Gained understanding regarding process creation and management using `fork()` and `execvp()`, and parent-child process relationships.
- **Pipes and File Redirection**: Learned to create inter-process communication channels using `pipe()` and to manipulate file descriptors for input and output redirection.
- **Handling Multiple Commands**: Developed skills in parsing and executing multiple commands, enhancing the shell's flexibility and usability.
- **Memory Leak Prevention**: Learned techniques to prevent memory leaks such as allocating memory dynaically to store command structures when parsing user input to handle commands and arguments. To prevent memory leaks, I implemented a comprehensive cleanup routine that systematically frees all allocated memory once a command is executed or if an error occurs during execution.

##Running the Project
- Compile the shell: `make`
- Run the shell: `./shell`
 
