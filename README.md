# Virtual Shell

# Program Description and Overview

In this project, I created my own virtual shell to run locally within my machine. The shell takes user input as arguments, and runs commands whether they are internal commands (handled by code locally on our side) or external commands (executable files ran by linux). Redirection is also utilized in this project, as we let our user change what is used for stdin, stdout, or both. This includes piping (using output from one command as input for another) and backround processes (letting more commands run before the first one finishes).

My code includes a few files for organizational purposes: myshell.c, internal.c, external.c, and redirection.c

# myshell.c

	- This file is the one that actually runs the shell. It includes main(). This file gets the user input using our method get_user_input(), as well as parse_input() to get the user input into an array form that can be passed to executables using the execvp() command once we know the command. The command is at argv[0], and the rest is flags and arguments. The array is added a NULL value at the very end for the purpose of passing it to execvp().

	- In myshell.c we also include run_shell(), which checks for redirection and executes the commands.


# internal.c

	- This file is used for handling the internal functions on a local code side. These functions are: 
			cd, clr, dir, environ, echo, help, pause, and quit

	- Each of these functions get their own handler method. Some of the functions, such as clr, are as simple as a print statement. Others, such as echo, take the args given from the user and print them to the screen. Environ uses an external variable and loops through all the environment variables of the system. All of these methods are commented out in the source code to show what each one does in more detail. Please see internal.c


# external.c

	- This file handles all the external commands. You will see that there is not as much code compared to internal commands because we are executing executables that already exist somewhere in the system. We first find the executable and then run it by passing the arg list that the user entered with the NULL terminated array of args.


# redirection.c

	- This file handles the redirection and piping processes from the user. We first detect if an args list includes <, >, >> or |. If so, we set redirection to the corresponding number and when we get to executing the commands in main, we call the corresponding handler method. Ex. if redirection == 1, call the ">" redireciton handler method. Inside these methods we check whether the command is internal or external and execute it inbetween the file descriptors changes.

	- File descriptors are retrieved from the file using the open() command, and the file descriptors for stdout and stdin are already known (1 and 0). We close these, open the file descriptor for the corresponding file, and then run our commands. We then set it back to 0 or 1 based on what we did.

