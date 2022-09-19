#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

//char **path;
char **path;
char error_message[30] = "An error has occurred\n";

void print(char **arr) {
	int p = 0;
	char *arr2 = arr[p];
	while (arr[p] != NULL) {
		printf("%s\n",arr2);
		p++;
		arr2 = arr[p];
	}
	//printf("\n");
}

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

	//printf("got this far");
	characters = getline(&buffer,&bufsize,stdin);
	return buffer;
}

char **splitLine(char *line, int n){ //IF COMMIT REVERTED CHANGE THIS NBNB

	size_t bufsize = 64;
	char **instructions =  malloc(bufsize * sizeof(char*));
	char *found;
	int p = 0;
	const char *delim = "\t\a\n ";

    while( (found = strsep(&line,delim)) != NULL ){
		if (strcmp(found,"") != 0) {
			instructions[p] = found; 
			p++;
		}
	} 

	//printf("n: %d\n",n);	
	if (n > 0) {
		for (int i=1; i<=n; i++){
			//printf("remove pointer i: %d\n",i);
			instructions[p-i] = NULL;
		}
	}
	//print(instructions);
	return instructions;
}

char **handlePath(char **args, char **path){

	size_t bufsize = 64;
	char *tmp = malloc(bufsize * sizeof(char*));
	if (args[1] == NULL) {
		path[0] = NULL;
	} else {
		int p = 1;
		while (args[p] != NULL)
		{
			char *s = strchr(args[p],'/');
			printf("This Line contains /: %s\n",s);
			if(s != NULL){
				memcpy(tmp,args[p], sizeof args[p]);				
				path[p-1] = tmp;	
			} else {
				strcat(args[p],"/");
				memcpy(tmp,args[p], sizeof args[p]);
				path[p-1] = tmp;	
			}
			p++;
		}
	}
	return path;
}

char *handleOutput(char **args){
	char *name = NULL;
	int i = 0;
	//print(args);
	while (args[i] != NULL) {
		if (strcmp(args[i],">" )==0){
			//printf("%s\n",args[i+2]);
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

char **splitLineParrallel(char *line, int n){

	size_t bufsize = 64;
	char **instructions =  malloc(bufsize * sizeof(char*));
	char *found;
	int p = 0;
	const char *delim = "&";

    while( (found = strsep(&line,delim)) != NULL ){
		if (strcmp(found,"") != 0) {
			instructions[p] = found; 
			p++;
		}
	} 

	//printf("n: %d\n",n);	
	if (n > 0) {
		for (int i=1; i<=n; i++){
			//printf("remove pointer i: %d\n",i);
			instructions[p-i] = NULL;
		}
	}
	//print(instructions);
	return instructions;
}

char ***splitParrallel(char **arr){
	size_t bufsize = 128;
	char ***args =  malloc(bufsize * sizeof(char*));
	//struct block *blocks;

	int p = 0;
	int i = 0;
	int n = 0;
	char **temp = malloc(bufsize * sizeof(char*));
	// //printf("I did it\n");
	// while(arr[p] != NULL){
	// 	p++;
	// }
	// //print(arr);
	// if (strcmp(arr[p-1],"&") == 0 ){
	// 	//printf("%s\n",arr[p-1]);
	// 	arr[p-1] = NULL;
	// }
	// p = 0;

	while(arr[p] != NULL){
		//printf("%s\n",arr[p]);
		if (strcmp(arr[p],"&") == 0){
			args[i] = temp;
			//printf("%d\n",i);
			//print(args[i]);
			//print(temp);
			i++;
			n = 0;
			temp = malloc(bufsize * sizeof(char*));
		} else {
			temp[n] = arr[p];
			n++;
		}
		p++;
	}
	//printf("%d\n",i);
	args[i] = temp;
	//print(args[i]);
	//print(temp);

	if (p == 1) {
		//check for s1&s2&s3

	}
	//print(args[0]);
	//print(args[1]);
	return args;
}

char ***ammendPaths(char ***args, char **path) {
	int p = 0;
	while (args[p] != NULL) {
		char **temp = args[p];
		print(temp);
		int i = 0;
		printf("path:\n");
		print(path);
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
		print(args[p]);
		p++;
	}
	return args;
}

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
	//printf("%d\n",p);

	//size_t bufsize = 64;
	//char **temp = malloc(bufsize * sizeof(char*));

	pid_t pids[p];
	for (int i=0; i < p; ++i) {
		//printf("%d\n",i);
		//temp = args[i];
		//print(temp);
		if ((pids[i] = fork()) < 0) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			//exit(1);
		} else if (pids[i] == 0) {
			char** temp = args[i];
			char *fileName = handleOutput(temp);
			FILE *fp;
			//temp = ammendPaths(temp, path);
			if (fileName != NULL) {
				//printf("hit");
				fp = freopen(fileName, "w", stdout); 
			}
			status = execv(temp[0], temp);
			//printf("%d\n",status);
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

int workingDir(char **args) {
	int i = 1;
	if (args[i] == NULL) {
		//printf("UHHH");
		write(STDERR_FILENO, error_message, strlen(error_message));
		return(0);
	}
	if (strcmp(args[i],"-la") == 0){
		i++;
	}
	//printf("NOOOO");
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

int builtIns(char ***a, char **path) {
	char **args = a[0];
	if (args[0] == NULL){
		//write(STDERR_FILENO, error_message, strlen(error_message));
		return(1);
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
		return(1);
	}
	// a = ammendPaths(a,path);
	// if (path[0] != NULL){
	// 	int l = doInstructions(a);
	// }
	return 0;
}

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

int batchMode(char *fileName){

	//printf("hello");
	size_t bufsize = 128;
	path = malloc(bufsize * sizeof(char*));
	path[0] = "/bin/";
	path[1] = "usr/bin/";


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

		char **split = splitLine(rdLine, 0);
		printf("path1:\n");
		print(path);

		char ***args = splitParrallel(split);

		status = builtIns(args,path);
		if (status != 1) {
			char **temp = args[0];
			if (strcmp(temp[0],"path" )==0) {
				path = handlePath(temp,path);
				//print(path);
				status = 1;
			}
		}
		if (status != 1) {
			//print(args[0]);
			printf("run ammend\n");
			print(path);
			args = ammendPaths(args,path);
			if (path[0] != NULL){
				status = doInstructions(args);
			}
		}
		//len--;
	}
	fclose(fp);

	return status;
}

int main(int MainArgc, char *MainArgv[]){

	//print(MainArgv);

	//printf("%s\n",MainArgv[2]);
	//size_t bufsize = 64;
	//path = malloc(bufsize * sizeof(char*));
	//path[0] = "/bin/";

	bool status = 1;
	char *line;
	char **instructions;
	char ***args;

	//error = malloc(bufsize * sizeof(char*));

	
	if (MainArgv[1] !=  NULL) {
		if (MainArgv[2] != NULL) {
			//print(MainArgv);
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
		status = batchMode(MainArgv[1]);
	}  else {
		char **path;
		path[0] = "/bin/";
		do {
			printf("witsshell> ");
			line = readLine();
			instructions = splitLine(line,0);
			args = splitParrallel(instructions);
			//print(args);
			//printf("I did it\n");
			status = builtIns(args,path);
		} while (status);
	}
	return(0);
}



