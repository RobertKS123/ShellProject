#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

char **path;

void print(char **arr) {
	int p = 0;
	char *arr2 = arr[p];
	while (arr[p] != NULL) {
		printf("%s\n",arr2);
		p++;
		arr2 = arr[p];
	}
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
	struct block *blocks;

	int p = 0;
	int i = 0;
	int n = 0;
	char **temp = malloc(bufsize * sizeof(char*));

	while(arr[p] != NULL){
		//printf("%s\n",arr[p]);
		if (strcmp(arr[p],"&") == 0){
			args[i] = temp;
			print(args[i]);
			i++;
			n = 0;
			free(temp);
			temp = malloc(bufsize * sizeof(char*));
		} else {
			temp[n] = arr[p];
			//printf("%s\n",temp[n]);
			n++;
		}
		p++;
	}
	args[i] = temp;
	print(args[i]);

	return args;
}

char ***ammendPaths(char ***args, char **path) {
	int p = 0;
	while (args[p] != NULL) {
		char **temp = args[p];
		int i = 0;
		//printf("I don't understand");
		while (path[i] != NULL) {
			char *tempPath = (char *) malloc( strlen(path[i]) + 1 );
			strcpy(tempPath,path[i]);
			strcat(tempPath,temp[0]);
			if (access(tempPath,X_OK) != -1){
				temp[0] = tempPath;
				args[p] = temp;
				//print(temp);
			}  else {
				i++;
			}
		}
		p++;
	}
	return args;
}

int doInstructions(char ***args, char **path){
	int wpid;
	int status = 0;

	int p = 0;
	int len = 0;

	while (args[p] != NULL) {
		len++;
	}
	printf("%d\n",len);
	p = 0;

	pid_t pids[len];

	// while (args[p] != NULL) {
	// 	char **temp = args[p];
	// 	while (path[p] != NULL) {
	// 		//Check is directory exsists cannot be done is parrallel path decomposes 
	// 		char *temp = (char *) malloc( strlen(path[p]) + 1 );
	// 		strcpy(temp,path[p]);
	// 		strcat(temp,args[0]);
	// 		//printf("%s\n",path[p]);
	// 		if (access(temp,X_OK) != -1){
	// 			args[0] = temp;
	// 			//check out put
	// 			char *fileName = handleOutput(args);
	// 			if (fileName != NULL) {
	// 				freopen(fileName, "w", stdout); 
	// 			}
	// 			pid = fork();
	// 			if (pid == 0){
	// 				status = execv(args[0], args); 
	// 				if (status == -1) {
	// 					printf("Process did not terminate correctly\n");
	// 					exit(1);
	// 				}
	// 			}
	// 			while ((wpid = wait(&status)) > 0);
	// 		} else {
	// 			p++;
	// 		}
	// 	}
	// }
	//here is where the parrallel needs to happen

}

int builtIns(char ***a, char **path) {
	char **args = a[0];
	if (strcmp(args[0],"path" )==0) {
		path = handlePath(args,path);
		return 1;
	}
	if (strcmp(args[0],"exit" )==0) {
		return 0;
	}
	if (strcmp(args[0],"cd" )==0) {
		chdir(args[1]);
		return 0;
	}
	a = ammendPaths(a,path);
	if (path[0] != NULL){
		int l = doInstructions(a,path);
	}
	return 1;
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
		status = builtIns(args,path);
	} while (status);
	return(0);
}



