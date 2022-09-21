#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>


char **path;
char error_message[30] = "An error has occurred\n";

//for debug perpouses only 
void print(char **arr) {
	int p = 0;
	char *arr2 = arr[p];
	while (arr[p] != NULL) {
		printf("%s\n",arr2);
		p++;
		arr2 = arr[p];
	}
}

//Fetchline from terminal
char *readLine(){
	char *buffer;
	size_t bufsize = 32;
	size_t characters;

	buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
	characters = getline(&buffer,&bufsize,stdin);
	return buffer;
}

//Split line by spaces tabs new lines and breakstrings that do not contain spaces
char **splitLine(char *line, int n){

	size_t bufsize = 64;
	char **instructions =  malloc(bufsize * sizeof(char*));
	char *found;
	char *found2;
	int p = 0;
	const char *delim = "\t\a\n ";
	const char *delim2 = "&";
	const char *delim3 = ">";

    while( (found = strsep(&line,delim)) != NULL ){
		if (strcmp(found,"") != 0) {
			int l = strlen(found);
			if (l > 1) {
				if (strchr(found,'&')){
					while((found2 = strsep(&found,delim2)) != NULL ){
						instructions[p] = found2; 
						p++;
						instructions[p] = "&";
						p++;
					}
					instructions[p-1] = NULL;
				} else if (strchr(found,'>')){
					while((found2 = strsep(&found,delim3)) != NULL ){
						instructions[p] = found2; 
						p++;
						instructions[p] = ">";
						p++;
					} 
					instructions[p-1] = NULL;
				}else  {
					instructions[p] = found; 
					p++;
				}
			} else {
				instructions[p] = found; 
				p++;
			}
		}
	} 

	if (n > 0) {
		for (int i=1; i<=n; i++){
			instructions[p-i] = NULL;
		}
	}
	return instructions;
}

//change the global path variable and append a / to the end if one is not present 
char **handlePath(char **args, char **path){

	size_t bufsize = 128;
	if (args[1] == NULL) {
		path[0] = NULL;
	} else {
		int p = 1;
		while (args[p] != NULL) {
			char *tmp = malloc(bufsize * sizeof(char*));
			int i = strlen(args[p]);
			char x = args[p][i-1];
			if (x == '/'){
				memcpy(tmp,args[p], sizeof args[p]);				
				path[p-1] = tmp;	
			} else {
				memcpy(tmp,args[p], sizeof args[p] + sizeof(char*));
				strcat(tmp,"/");
				path[p-1] = tmp;	
			}
			p++;
			tmp = malloc(bufsize * sizeof(char*));
		}
		p=0;
		while (path[p] != NULL) {
			if(args[p+1] == NULL) {
				path[p] = NULL;
			}
			p++;
		}
	}
	return path;
}

//get and return output file name if one is present
char *handleOutput(char **args){
	char *name = NULL;
	int i = 0;
	while (args[i] != NULL) {
		if (strcmp(args[i],">" )==0){
			if (args[i+1] != NULL){
				name = args[i+1];
				args[i] = NULL;
				args[i+1] = NULL;
			}  else {
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			if (args[i+2] == NULL) {

			} else {
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
		}
		i++;
	}
	return name;
}

//split parrallel commands into a triple pointer
char ***splitParrallel(char **arr){
	size_t bufsize = 128;
	char ***args =  malloc(bufsize * sizeof(char*));

	int p = 0;
	int i = 0;
	int n = 0;
	char **temp = malloc(bufsize * sizeof(char*));
	while(arr[p] != NULL){
		p++;
	}
	if (p != 0) {
		if (strcmp(arr[p-1],"&") == 0 ){
			arr[p-1] = NULL;
		}
	}
	p = 0;

	while(arr[p] != NULL){
		if (strcmp(arr[p],"&") == 0){
			args[i] = temp;
			i++;
			n = 0;
			temp = malloc(bufsize * sizeof(char*));
		} else {
			temp[n] = arr[p];
			n++;
		}
		p++;
	}
	args[i] = temp;
	return args;
}

//check if the specified instruction is valid given the current path of the shell
char ***ammendPaths(char ***args, char **path) {
	int p = 0;
	while (args[p] != NULL) {
		char **temp = args[p];
		int i = 0;
		while (path[i] != NULL) {
			char *tempPath = (char *) malloc( strlen(path[i]) + 1 );
			strcpy(tempPath,path[i]);
			strcat(tempPath,temp[0]);
			if (access(tempPath,X_OK) != -1){
				temp[0] = tempPath;
				args[p] = temp;
			}  else {
				i++;
			}
		}
		p++;
	}
	return args;
}

//preform execv and execv parrallel instructions
int doInstructions(char ***args){
	int wpid;
	int status = 0;

	int p = 0;

	while (args[p] != NULL) {
		p++;
	} 
	if (p == 0) {
		write(STDERR_FILENO, error_message, strlen(error_message));
		return(1);
	}

	pid_t pids[p];
	for (int i=0; i < p; ++i) {
		if ((pids[i] = fork()) < 0) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			//exit(1);
		} else if (pids[i] == 0) {
			char** temp = args[i];
			char *fileName = handleOutput(temp);
			FILE *fp;
			if (fileName != NULL) {
				fp = freopen(fileName, "w", stdout); 
			}
			status = execv(temp[0], temp);
			if (status == -1) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			fclose(fp);
		}
		//free(temp);
		//temp = malloc(bufsize * sizeof(char*));
	}

	pid_t pid;
	while (p > 0) {
		pid = wait(&status);
		p--;
	}
}

//check if a given directory exsists
int workingDir(char **args) {
	int i = 1;
	if (args[i] == NULL) {
		write(STDERR_FILENO, error_message, strlen(error_message));
		return(0);
	}
	if (strcmp(args[i],"-la") == 0){
		i++;
	}
	if (access(args[i],X_OK) == -1){
		write(STDERR_FILENO, error_message, strlen(error_message));
		return(0);
	} else {
		i++;
		if(args[i] != NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			return(0);
		}
	}

}

//check current instruction for any built in commands
int builtIns(char ***a, char **path) {
	char **args = a[0];
	if (args[0] == NULL){
		return(1);
	}
	if (strcmp(args[0],"path" )==0) {
		path = handlePath(args,path);
		return (1);
	}
	if (strcmp(args[0],"exit" )==0) {
		if (args[1] != NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			return(0);
		} else {
			return 0;
		}
	}
	if (strcmp(args[0],"cd" )==0) {
		if (args[1] != NULL){
			if (access(args[1],X_OK) == -1){
				write(STDERR_FILENO, error_message, strlen(error_message));
				return(1);
			} 
			if(args[2] != NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				return(1);
			}
			chdir(args[1]);
		} else {
			write(STDERR_FILENO, error_message, strlen(error_message));
			return(1);
		}
		return 1;
	}
	if (path[0] == NULL) {
		write(STDERR_FILENO, error_message, strlen(error_message));
		return(1);
	}
	a = ammendPaths(a,path);
	if (path[0] != NULL){
		int l = doInstructions(a);
	}
	return 1;
}

//checks if a file exsists and returns its number of lines
int fileLength(char *fileName) {
	FILE *fp = fopen(fileName,"r");
	if (access(fileName, F_OK) == 0) {
		//printf("here\n");
	} else {
		//printf("there\n");
	}
	if (!fp) {
		//fprintf(stderr, "Error opening file '%s'\n", fileName);
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	int lines=0;
	int ch=0;
	while(!feof(fp))
	{
		ch = fgetc(fp);
		if(ch == '\n')
		{	
			lines++;
		}
	}
	lines++;
	fclose(fp);
	//printf("%d\n",lines);
	return(lines);
}

//reads a file and runs the shell in batch mode
int batchMode(char *fileName, char **path){

	int status = 0;

	FILE *fp = fopen(fileName,"r");

	size_t l = 0;
	ssize_t read;
	char * rdLine = NULL;
	int len = fileLength(fileName);
	if (len == -1){
		return 0;
	}

	fp = fopen(fileName,"r");

	while ((read = getline(&rdLine, &l, fp)) != -1) {

		status = 0;

		char **split = splitLine(rdLine, 0);

		char ***args = splitParrallel(split);

		char **temp = args[0];

		status = builtIns(args,path);

	}
	fclose(fp);

	return status;
}


int main(int MainArgc, char *MainArgv[]){

	size_t bufsize = 128;
	path = malloc(bufsize * sizeof(char*));
	path[0] = "/bin/";
	path[1] = "usr/bin/";

	bool status = 1;
	char *line;
	char **instructions;
	char ***args;
	
	if (MainArgv[1] !=  NULL) {
		if (MainArgv[2] != NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
		status = batchMode(MainArgv[1],path);
	}  else {
	do {
			printf("witsshell> ");
			line = readLine();
			instructions = splitLine(line,0);
			args = splitParrallel(instructions);
			status = builtIns(args,path);
	} while (status);
	}
	return(0);
}



