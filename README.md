# myShell

CENG322 Operating Systems homework

C program similar to a shell interface.

Built-in functions:
1) cd <directory> : change the current directory to <directory>
  * If the <directory> argument is not present, program change the directory to $HOME environment variable.
  * Supports relative and absolute paths.
 
2) dir : print the current working directory.
 
3) history : print 10 most recently entered commands in shell.
 
4) bye : terminate your shell process.
 
 
For any other commands, program consider them as system commands. Forsystem commands, program creates a child process using fork system call, and the child
process executes the command.
 
Program supports pipe operator between two processes. 
 
