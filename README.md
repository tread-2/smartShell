# smartShell

A simple unix-like shell written in C. This shell supports basic built-in commands and excute external programs using fork() and exec()

# Features
- Built-in functions

- 'cd'
- 'mkdir'
- 'ls'
- 'exit'
- 'touch'
- 'pwd'
- 'rmdir'
- 'help'

- Custom prompts to help users with navigation
- Executinon of external commands
- Error Handling for invalid commands and directorie changes

## Build Instructions

### Requirements
- GCC (or any C compilers)
- Unix/Linux Enviroments

### To Compile
- bash/terminal
- make smartshell ~ to make an executable file
- ./smartshell.c ~ to run your shell


# What I learned
- Process creation using fork()
- Executing commands with execvp()
- Handling input/output with readline() or getline()
- Building basic shell logic and parsing
