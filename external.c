/*
Author: Benjamin Rittenhouse
File: external.c
File to handle and execute external files and executables for the virtual shell
*/

#include "internal.c"
#include <sys/types.h>
#include <signal.h>

// method to find where an external file exists
// uses path_locations variable
int find_file(char* command){
	int index = -1;
//	printf("command = %s\n", command);
	for(int i = 0; i < path_locations_count; i++){
		// change to path location
		chdir(path_locations[i]);

		struct dirent *dp;
		DIR* dirptr = opendir(path_locations[i]);

		dp = readdir(dirptr);
		while(dp != NULL){
			if(strcmp(command, dp->d_name) == 0){ // we found the file in this directory
				index = i;
			}
			dp = readdir(dirptr);
		}

		

	}
	// change back to myshell dir
	chdir("/Users/ben/Desktop/OS/project-2-benjaminrittenhouse/myshell");

	return index;
	//return path_locations[index];
}

// method to execute an external command
void execute_external_command(char* command, char* argv[], int argc){

	int background = 0;
	if(strcmp(argv[argc - 1], "&") == 0){ // if we want to have a background process, chop argv down 1
 		argv[argc - 1] = NULL;
 		argc = argc - 1;
 		background = 1;
	} 
	char* tempCommand = malloc(100*sizeof(char));
	strcpy(tempCommand, command);
	int f = fork();
	if(f == 0){
		execvp(tempCommand, argv);
	} else {
		int status = 0;
		if(background == 1){ // if we want to have a background process, dont wait
 
		} else { // else, wait
			waitpid(-1, NULL, 0);
			wait(&status);
		}
	}


}

void all_external_commands(char* command, char* argv[], int argc){
	execute_external_command(command, argv, argc);
}