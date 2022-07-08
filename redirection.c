/*
Author: Benjamin Rittenhouse
File: redirection.c
Handles redirection, input redirection, and >> redirection
*/


#include "external.c"


// > command
int redirection(char* filename, char* command, char* argv[], int argc, char* flags[], int numflags, char* inputs[], int numinputs, char* external_path){
	int newstdout = open(filename,O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO); // open file for redirection
	int stdout_copy = dup(1); // copy ouput file
	close(1); // close output
	dup(newstdout); // duplicate new file
	int internal = is_internal(command);// check if command is internal
	if(internal == 1){
		all_internal_commands(command, argv, argc, flags, numflags, inputs, numinputs, external_path);
	} else {
		execute_external_command(command, argv, argc);
	}
	close(newstdout); // close file
	dup2(stdout_copy, 1);
	close(stdout_copy);

	return 0;
}


// < command
int indirection(char* filename, char* command, char* argv[], int argc, char* flags[], int numflags, char* inputs[], int numinputs, char* external_path){
	int stdin_copy = dup(0);
	int newstdin = open(filename,O_RDONLY);
	
	close(0);
	dup(newstdin);
	int internal = is_internal(command);
	if(internal == 1){
		all_internal_commands(command, argv, argc, flags, numflags, inputs, numinputs, external_path);
	} else {
		execute_external_command(command, argv, argc);
	}
	close(newstdin);

	dup2(stdin_copy, 0);
	close(stdin_copy);

	return 0;
}

// >> command
int double_redirection(char* filename, char* command, char* argv[], int argc, char* flags[], int numflags, char* inputs[], int numinputs, char* external_path){
	int newstdout;

	if(access(filename, F_OK ) == 0){ // file exists, append
		newstdout = open(filename,O_WRONLY|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO); // append to file if it exists already
	} else { // file doesn't exist, create
		newstdout = open(filename,O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO); // add to file if it was already created
	}
	
	int stdout_copy = dup(1);
	close(1); 						// close / open redirection for new file and previous output file (1)
	dup(newstdout);
	int internal = is_internal(command); 
	if(internal == 1){ // check if command is internal or external
		all_internal_commands(command, argv, argc, flags, numflags, inputs, numinputs, external_path);
	} else {
		execute_external_command(command, argv, argc);
	}
	close(newstdout);
	dup2(stdout_copy, 1); // reset file output 
	close(stdout_copy);

	return 0;
}

void pipe_myshell(char* command, char* argv[], int argc){


	/*printf("Args:\n");
	for(int j = 0; j < argc; j++){
   			printf("%s\n", argv[j]);
	}*/

	int pipe_index = -1;
	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "|") == 0){
			pipe_index = i;
		//	printf("pipe found at %d\n", pipe_index);
		}
	}

	// args before pipe
	char* before_pipe[50];
	int i;
	for(i = 0; i < pipe_index; i++){
		char* temp_before = malloc(100*sizeof(char));
    	strcpy(temp_before, argv[i]);
		before_pipe[i] = temp_before;
	}
	before_pipe[i] = NULL;

	char* command_1 = before_pipe[0];

	// args after pipe
	char* after_pipe[50];
	int c = 0;
	for(int k = pipe_index + 1; k < argc; k++){
		char* temp_after = malloc(100*sizeof(char));
    	strcpy(temp_after, argv[k]);
		after_pipe[c] = argv[k];
		c++;
	}
	after_pipe[c] = NULL;

	char* command_2 = after_pipe[c - 1];


	//printf("Command 1: %s, Command 2: %s\n", command_1, command_2);

	/*printf("Args Before:\n");
	for(int j = 0; j < i + 1; j++){
   			printf("%s\n", before_pipe[j]);
	}
	printf("Args After:\n");
	for(int j = 0; j < c + 1; j++){
   			printf("%s\n", after_pipe[j]);
	}*/

	int file_descs[2];
	int pid = -1; // pid of first child process
  	int pid2 = -1; // pid of second child process
	  
	// create pipe
  	if (pipe(file_descs) == -1){
    	perror("Failed to create pipe!");
    	exit(-1);
  	}

  	// fork first child 
  	if ((pid = fork()) < 0){
    	perror("fork 1 failed!\n");
    	exit(-1);
  	}

  	
  	if(pid == 0) { // first child 
    	// close read side of pipe
    	close(file_descs[0]);
    	// redirect stdout to write side of pipe
    	dup2(file_descs[1], 1);
    	close(file_descs[1]);
    	// exec program 1
    	if (execvp(command_1, before_pipe) < 0){
     	 perror("Could not execute command 1");
     	 exit(-1);
    	}
    }

    // fork second child
  	if ((pid2 = fork()) < 0){
    	perror("Second fork failed");
    	exit(-1);
  	}

  	if(pid2 == 0) { // second child
  		   		
  		// close write side of pipe
    	close(file_descs[1]);
    	// redirect stdin to read side of pipe
    	dup2(file_descs[0], 0);
    	close(file_descs[0]);
    	// exec program 2

    	if (execvp(command_2, after_pipe) < 0){
      		perror("Could not execute command 2");
      		exit(-1);
    	}
  	}


  	waitpid(-1, NULL, 0);
  	// parent
  	// -1 signifies to wait for ALL children to finish
  	//puts("both children finished!");
  	// close the pipes now that we are done with them
  	close(file_descs[0]);
  	close(file_descs[1]);
}