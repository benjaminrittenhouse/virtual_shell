/*
Author: Benjamin Rittenhouse
File: myshell.c
Main file to get user input and run the shell
Inlcudes internal.c, redirection.c, external.c
*/

#include "redirection.c"

char* get_user_input();
void parse_input(char*, char*[], int * argc, char**);
void get_flags(char* [], int, char* [], int *);
void get_inputs(char* [], int , char* [], int * , int );
void parse_input_redirection(char*, char*[], int*);
void run_shell(char* , char* [], int , char* , char* [], int , char* [], int , char*,  char*[], int);

int main(int argcc, char *argvv[]){
	chdir("/Users/ben/Desktop/c/");
	char* argv[50]; // all args
	char* command = ""; // command
	int argc = -1; // # of args

	char* argv_redirection[50]; // array of strings to store redirection(doesn't null terminate before < chars)
	int argc_redirection = -1;

	char* flags[50]; // only flag args
	int numflags = -1;

	char* inp[50]; // only input args
	int numinputs = -1; // # of inputs


	char* external_path = "";


	


	// get current directory for prompt (printed before > character)
	char buff[100];
	const char* current_dir = (char*) malloc(100*sizeof(char));
	current_dir = getcwd(buff, sizeof(buff));

	char* userInput;
	char* batchfile;
	if(argcc == 1){ // we are doing user input, not a batch file
			// get path variable for external files to be found
			path();
			// set PATH
			setenv("PATH", "/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/Library/Apple/usr/bin", 1);
			userInput = get_user_input(); // get user input
			while(strcmp("exit", userInput) != 0){ // while user has not typed exit
				// run shell and continue to get user input
				run_shell(userInput, argv, argc, command, flags, numinputs, inp, numflags, external_path, argv_redirection, argc_redirection);
				userInput = get_user_input();
			}
	} else { // we are doing a batch file

		FILE *fp = fopen(argvv[1], "r");


		if(fp == NULL){
			printf("batchfile: cannot open help file\n");
			exit(1);
		}	

		char *line = NULL;
		size_t linecap = 0;
		ssize_t linelen;
		
			while((linelen = getline(&line, &linecap, fp)) != -1){ // keep getting lines from input batch file
				if(line[strlen(line) - 1] == '\n'){ // stop new line chars from breaking input
					line[strlen(line) - 1] = '\0';
				}
				userInput = line;
				// run the shell with this buffer
				run_shell(userInput, argv, argc, command, flags, numinputs, inp, numflags, external_path, argv_redirection, argc_redirection);
			}
		}
}


void run_shell(char* userInput, char* argv[], int argc, char* command, char* flags[], int numinputs, char* inp[], int numflags, char* external_path, char* argv_redirection[], int argc_redirection){
	
    	char* temp_userinput = malloc(100*sizeof(char));
    	strcpy(temp_userinput, userInput);
    	// parse and tokenize input, store commands, args, and arg count
    	parse_input(userInput, argv, &argc, &command);
    	parse_input_redirection(temp_userinput, argv_redirection, &argc_redirection);
    	get_flags(argv, argc, flags, &numflags);
    	get_inputs(argv, argc, inp, &numinputs, numflags);


    	/*printf("REDIRECTION ARGS ---- MAIN:\n");
    	for(int i = 0; i <= argc_redirection; i++){
   			printf("%s\n", argv_redirection[i]);
		}*/
		

    	//printf("argc = %d\n", argc);
    	// set up indirection / redirection
		int redirect = -1;
    	int redirectIndex = -1;
    	for(int i = 0; i < argc_redirection; i++){
    		//printf("ARG = %s\n", argv[i]);
    		if(strcmp(argv_redirection[i], ">") == 0){
    			redirect = 1;
    			redirectIndex = i;
    		} else if (strcmp(argv_redirection[i], "<") == 0){
                redirect = 2;
                redirectIndex = i;
            } else if (strcmp(argv_redirection[i], ">>") == 0){
            	redirect = 3;
            	redirectIndex = i;
            } else if (strcmp(argv_redirection[i], "|") == 0){
            	redirect = 4;
            	redirectIndex = i;
            }
    	}
    	

  		int isInternal = is_internal(command); // check if command is internal command
  		
    	// Handle passed commands
    	if(redirect == 1){ // internal and redirection
            redirection(argv_redirection[redirectIndex + 1], command, argv, argc, flags, numflags, inp, numinputs, external_path);
            //printf("redirection!!\n");
        } else if(redirect == 2){ // internal and input redirection
            indirection(argv_redirection[redirectIndex + 1], command, argv, argc, flags, numflags, inp, numinputs, external_path);
        } else if (redirect == 3){ // internal and double redirection
        	double_redirection(argv_redirection[redirectIndex + 1], command, argv, argc, flags, numflags, inp, numinputs, external_path);
        } else if (redirect == 4){ // piping
        	pipe_myshell(command, argv_redirection, argc_redirection); 
        } else if (isInternal == 1){   // just internal command no redirection
            all_internal_commands(command, argv, argc, flags, numflags, inp, numinputs, external_path);
        } else { // external command
        	all_external_commands(command, argv, argc);
        }
}

// gets user input
// ** TO DO: is chopping off pr when passed, not sure why**
char* get_user_input(){

		// get prompt / current working directory
		char b[100];
		char* pr = (char*) malloc(100*sizeof(char));
		pr = getcwd(b, sizeof(b));		

		// create vars for user input
		char *userInput;
    	size_t size = 200;
    	size_t chars;
    	userInput = (char *)malloc(size * sizeof(char));

    	// print prompt
    	printf("%s>", pr);
    	// get user input
    	chars = getline(&userInput,&size,stdin);

    	// store length of input
    	int len = strlen(userInput);

    	// remove new line character for strcmp purposes
    	if(userInput[len - 1] == '\n'){
    		len -= 1;
    		userInput[len] = '\0';
    	}


    	free(userInput);
    	return userInput;
}


// parse input using strtok to get all individual args
void parse_input(char* userinput, char* argv[], int * argc, char** command){
	const char delim[2] = " ";
	char* token;

	// get token
	token = strtok(userinput, delim);

	int i = 0;
	while(token != NULL){
		
		// stop when we get to these, if they exist
		if(strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || strcmp(token, "<") == 0 || strcmp(token, "|") == 0){
			break;
		}


		// save the command (haven't found command yet)
		if(i == 0){
			int len = strlen(*command);
			if(token[strlen(token) - 1] == '\n'){ // remove new line char
				token[strlen(token) - 1] = '\0';
			}
			
			char* temp_token = malloc(100*sizeof(char));
			strcpy(temp_token, token);
			*command = temp_token; // store the command
			argv[i] = token;
			i++;
			token = strtok(NULL, delim); // get next token
		} else {
			char* temp_token = malloc(100*sizeof(char));
			strcpy(temp_token, token);
			// copy all args into argv array, we already have command
			argv[i] = temp_token; // store the args
			i++;
			token = strtok(NULL, delim); // get next token
		}	
	}

	// last val set to null for execvp
	argv[i] = NULL;
	*argc = i; // store count of args in arc
}


void parse_input_redirection(char* userinput, char* argv_redirection[], int * argc_redirection){
	//printf("user input given %s\n", userinput);
	const char delim[2] = " ";
	char* token;

	// get token
	token = strtok(userinput, delim);

	int i = 0;
	while(token != NULL){

		if(token[strlen(token) - 1] == '\n'){ // remove new line char
				token[strlen(token) - 1] = '\0';
		}
		char* temp_token = malloc(100*sizeof(char));
		strcpy(temp_token, token);
		argv_redirection[i] = temp_token; // store the args
		i++;
		token = strtok(NULL, delim); // get next token
	}	

	*argc_redirection = i; // store count of args in arc
}




// takes tokenized list of arguments, stores them in flags array of strings
void get_flags(char* argv[], int argc, char* flags[], int * numflags){
	int count = 0;
	for(int i = 0; i < argc; i++){
		char* curr = argv[i];
		if(curr[0] == '-'){ // arg is a flag
			flags[count] = argv[i];
			count++;
		}
	}

	*numflags = count;
}

// takes tokenized list of arguments, stores them in input array of strings
// goes to where numflags stopped and reads in inputs from there
void get_inputs(char* argv[], int argc, char* inputs[], int * numinputs, int numflags){
	int count = 0;
	for(int i = numflags; i < argc; i++){
		if(strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0 || strcmp(argv[i], "<") == 0){
			break;
		}
		inputs[count] = argv[i];
		count++;
	}

	*numinputs = count;

}

