/*
Author: Benjamin Rittenhouse
File: internal.c
File for internal functions to be handled by virtual shell
To be included in other files
*/

#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/fcntl.h>
#include<dirent.h>
#include<stdlib.h>
#include<unistd.h>

extern char **environ;

// locations for where external file could be located
char* ext_path;
char* path_locations[50];
int path_locations_count;

const char* pwd();

// clear handler
void clear(){
	printf("\e[1;1H\e[2J");
}

// list all environment variables
void envi(){
	for (char **e = environ; *e; e++)
        printf("%s\n", *e);
}

// get path variable as string from env
void path(){
	for (char **e = environ; *e; e++){
        if(strstr(*e, "PATH") != NULL){
        	ext_path = *e;
        }
	}


    char temp[50];

	// 	token = strtok(userinput, delim);
	// str tok over all values inside path variable
	int count = 0;
	char* ret = "";
	char delim[2] = ":";
	int t = 0;
	ret = strtok(ext_path, delim);
	path_locations_count = 0;
	while(ret != NULL){
		if(t == 0){ // chop out first "PATH = "	
			path_locations[count] = "/usr/local/bin"; // temporarily broken
			count++;
			t = 1;
			ret = strtok(NULL, delim);
		} else {
			path_locations[count] = ret;
			count++;
			ret = strtok(NULL, delim);
		}
	}


	path_locations_count = count;
	
	/*printf("TOKENS:\n");
    for(int i = 0; i < count; i++){
   			printf("%s\n", path_locations[i]);
	}*/

	// set path back to normal
	
}


// echo / print user input 
int echo(int argc, char* flags[], int numflags, char* inputs[], int numinputs){
	
	if(numflags > 0){ // if there are flags 
		 	for(int i = 0; i < numflags; i++){
 				if(strcmp(flags[i], "-n") == 0){ // if the -n flag is passed, do not print trailing \n character
 					int i;
    				for(i = 1; i < numinputs; i++){
    					printf("%s ", inputs[i]);
    				}

 				} else { // else print the end line character
 					int i;
    				for(i = 1; i < numinputs; i++){
    					printf("%s ", inputs[i]);
    				}

    				printf("\n");
 				}
 			}
	} else {
		for(int i = 1; i < numinputs; i++){
			printf("%s ", inputs[i]);
		}

		printf("\n");
	}

	return 1;
}

// prints help.txt file, has all commands and information
void help(){
	FILE *fp = fopen("help", "r");

	if(fp == NULL){
		printf("help: cannot open help file\n");
		exit(1);
	}

	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	while((linelen = getline(&line, &linecap, fp)) > 0){
		char *buffer = line;
		while(buffer != NULL){
			printf("%s", buffer);
			buffer = fgets(line, linelen, fp);
		}
	}
}


// quit the shell
void quit(){
	exit(0);
}


// change directory
const char* cd(char* newd, int argc){

	if(argc == 1){
		const char* a = pwd();
		return a;
	}

	if(newd[strlen(newd) - 1] == '\n'){
		newd[strlen(newd) - 1] = '\0';
	}
	// get current directory
	char buff2[100];
	char* pathname = (char*) malloc(100*sizeof(char));
	pathname = getcwd(buff2, sizeof(buff2));
	
	const char *new_path = (const char*) malloc(sizeof(char*)*100);
	int length = strlen(pathname);
	//printf("LENGTH IS %d\n", length);

	if(strcmp(newd, " ") == 0 || strcmp(newd, "") == 0 || newd == NULL){ // cd nothing

	} else if(strcmp(newd, ".") == 0){ // we are changing into current directory (do nothing)
		return new_path;
	} else if (strcmp(newd, "..") == 0){ // we are changing back one directory
			int index = 0;
			if(!(length < 2) && index != -1){

				// find where the last "/" character is
				index = -1;
				int count = 0;
				for(int i = 0; i < length; i++){
					if(pathname[i] == '/'){
						index = i;
						count++;
					}
				}	




				// create temp string without word after index
				// i.e. "Users/Ben" becomes "Users/"

				char* temp = (char*) malloc(sizeof(char*)*100);
				if(count != 1){ // there is more than one slash
					strncpy(temp, pathname, index); // chop off last term after last slash

					// saving to const char (for chdir())
					new_path = temp;
					chdir(new_path);
				} else if(count == 1 && index == 0){
					new_path = "/";
					chdir(new_path);
				}

			} else {
				new_path = "/"; // new path is the lowest directory, home
				chdir(new_path);
			}
	} else { // we are changing into specified directory
		char tempPath[100] = "";
		// copy into one string


		strcpy(tempPath, pathname);

		// avoid adding double slash
		if(!(tempPath[strlen(tempPath) - 1] == '/')){
			strcat(tempPath, "/");
		}

		// concatenate new path of working directory
		strcat(tempPath, newd);
		new_path = tempPath;


		int c = chdir(new_path);
		// if we get an error, path does not exist
		if(c == -1){
			printf("path does not exist\n");
			// saving to const char (for chdir())
			return pathname;
		} else {
			// else change dir
			chdir(new_path);
		}
	}

	return new_path;
}


// helper method to print current directory
const char* pwd(){
	char buff[100];
	char* current_dir = (char*) malloc(100*sizeof(char));;
	current_dir = getcwd(buff, sizeof(buff));
	printf("%s\n", current_dir);

	return current_dir;
}


// read contents of directory
void dir(char* d, int argc){
	// change to desired directory
	cd(d, argc);

	// get current directory
	char buff2[100];
	char* pathname = (char*) malloc(100*sizeof(char));;
	pathname = getcwd(buff2, sizeof(buff2));

	// store dirptr
	struct dirent *dp;
	DIR* dirptr = opendir(pathname);

	dp = readdir(dirptr);
	while(dp != NULL){
		printf("%s\n", dp->d_name);
		dp = readdir(dirptr);
	}

	cd("..", argc);
}


// pause until enter is pressed
void pauseInput(){
	char c = getchar();
	while(c != '\n'){
		c = getchar();
	}
}


// all internal commands handled
void all_internal_commands(char* command, char* argv[], int argc, char* flags[], int numflags, char* inputs[], int numinputs, char* change){
	if(strcmp(command, "clr") == 0){
                clear();
    } else if(strcmp(command, "environ") == 0){
                envi();
    } else if (strcmp(command, "echo") == 0){
                echo(argc, flags, numflags, inputs, numinputs);
    } else if (strcmp(command, "help") == 0){
                help();
    } else if (strcmp(command, "quit") == 0){
                quit();
    } else if (strcmp(command, "cd") == 0){
               	cd(argv[1], argc);
    } else if (strcmp(command, "pwd") == 0){ // extra function to print current directory
                pwd();
    } else if (strcmp(command, "dir") == 0){
                dir(argv[1], argc);
    } else if (strcmp(command, "pause") == 0){
                pauseInput();
    } 
}


// returns 1 if command is internal, 0 if otherwise
int is_internal(char* command){
	char* argument_list[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "pwd"};
	for(int i = 0; i < 9; i++){
		if(strcmp(argument_list[i], command) == 0){
			return 1;
		}
	}

	return 0;
}