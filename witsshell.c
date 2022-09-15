#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

char **path;

bool endLoop(char *arr){
	const char x[4] = {'e','x','i','t'};
	bool status = true;
	if (sizeof arr >= 4){
		for(int i=0;i<4;i++){
			if (arr[i] == x[i]){
				status = false;
			} else {
				status = true;
				break;
			}
		}
	}
	return status;
}

char *readLine(){
	char *buffer;
	size_t bufsize = 32;
	size_t characters;

	buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

	characters = getline(&buffer,&bufsize,stdin);
	return buffer;
}

char **splitLine(char *line){

	size_t bufsize = 64;
	char **instructions =  malloc(bufsize * sizeof(char*));
	char *found;
	int p = 0;
	const char *delim = "\t\r\a\n ";

    while( (found = strsep(&line,delim)) != NULL ){
		instructions[p] = found; 
		p++;
	}
	instructions[p-1] = NULL;

	return instructions;
}

char **handlePath(char **args, char **path){
	if (args[1] == NULL) {
		path[0] = NULL;
	} else {
		int p = 1;
		while (args[p] != NULL)
		{
			path[p-1] = args[p];
			p++;
		}
	}
	return path;
}

char *handleOutput(char **args){
	char *name = NULL;
	int i = 0;
	while (args[i] != NULL) {
		if (strcmp(args[i],">" )==0){
			if (args[i+2] == NULL) {
				name = args[i+1];
				args[i] = NULL;
				args[i+1] = NULL;
				//printf("%s\n",name);
			} else {
				printf("ERROR\n");
			}
		}
		i++;
	}
	return name;
}

char ***splitParrallel(char **arr){
	size_t bufsize = 128;
	char ***args =  malloc(bufsize * sizeof(char*));

	int p = 0;
	int i = 0;
	int n = 0;
	char **temp;

	while(arr[p] != NULL){
		printf("%d\n",p);
		if (strcmp(arr[p],"&") == 0){
			//args[i] = temp;
			i++;
			n = 0;
			temp = NULL;
		} else {
			args[i][n] = arr[p];
			printf("%s\n",temp[n]);
			n++;
		}
		p++;
	}
	args[i] = temp;

	return args;
}

int doInstructions(char **args, char **path){
	int wpid;
	int pid;
	int status = 0;

	if (strcmp(args[0],"path" )==0) {
		path = handlePath(args,path);
		return 1;
	}
	if (strcmp(args[0],"exit" )==0) {
		return 0;
	}
	if (strcmp(args[0],"cd" )==0) {
		chdir(args[1]);
	}

	if (path[0] != NULL){
		int p = 0;
		while (path[p] != NULL) {
			char *temp = (char *) malloc( strlen(path[p]) + 1 );
			strcpy(temp,path[p]);
			strcat(temp,args[0]);
			//printf("%s\n",path[p]);
			if (access(temp,X_OK) != -1){
				args[0] = temp;
				char *fileName = handleOutput(args);
				if (fileName != NULL) {
					pid = fork();
					if (pid == 0){
						freopen(fileName, "w", stdout); 
						execv(args[0], args);
						if (status == -1) {
							printf("Process did not terminate correctly\n");
							exit(1);
						}
					}
				} else {
					pid = fork();
					if (pid == 0){
						status = execv(args[0], args); 
						if (status == -1) {
							printf("Process did not terminate correctly\n");
							exit(1);
						}
					}
				while ((wpid = wait(&status)) > 0);
				}
			} else {
				p++;
			}
		}
	}
	return 1;
}

void print(char ***arr) {
	int p = 0;
	char **arr2 = arr[p];
	while (arr2 != NULL) {
		int i = 0;
		char *arr3 = arr2[i];
		while (arr3 != NULL) {
			arr3 = arr2[i];
			printf("%s\n",arr3);
			i++;
		}
		p++;
	}
}

int main(int MainArgc, char *MainArgv[]){

	//chdir("/bin/");
	size_t bufsize = 64;
	path = malloc(bufsize * sizeof(char*));
	path[0] = "/bin/";

	bool status = 1;
	char *line;
	char **instructions;
	char ***args;

	do {
		printf("witsshell> ");
		line = readLine();
		instructions = splitLine(line);
		args = splitParrallel(instructions);
		//print(args);
		//status = doInstructions(instructions,path);
	} while (status);
	return(0);
}



