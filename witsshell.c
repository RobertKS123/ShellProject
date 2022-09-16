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
		printf("%s ",arr2);
		p++;
		arr2 = arr[p];
	}
	printf("\n");
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

char **splitLine(char *line, int n){ //IF COMMIT REVERTED CHANGE THIS NBNB

	size_t bufsize = 64;
	char **instructions =  malloc(bufsize * sizeof(char*));
	char *found;
	int p = 0;
	const char *delim = "\t\r\a\n ";

    while( (found = strsep(&line,delim)) != NULL ){
		if (strcmp(line,"") != 0) {
			instructions[p] = found; 
			p++;
		}
	} 
	
	//printf("n: %d\n",n);	
	// if (n > 0) {
	// 	for (int i=1; i<=n; i++){
	// 		//printf("remove pointer i: %d\n",i);
	// 		instructions[p-i] = NULL;
	// 	}
	// }
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
	//print(args);
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

	//print(args[0]);
	//print(args[1]);
	return args;
}

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

int doInstructions(char ***args){
	int wpid;
	int status = 0;

	int p = 0;

	while (args[p] != NULL) {
		p++;
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
			perror("fork");
			//exit(1);
		} else if (pids[i] == 0) {
			char** temp = args[i];
			char *fileName = handleOutput(temp);
			FILE *fp;
			if (fileName != NULL) {
				//printf("hit");
				fp = freopen(fileName, "w", stdout); 
			}
			status = execv(temp[0], temp);
			if (status == -1) {
				printf("Process did not terminate correctly\n");
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
		return 1;
	}
	a = ammendPaths(a,path);
	//print(a[0]);
	//	print(a[1]);
	if (path[0] != NULL){
		int l = doInstructions(a);
	}
	return 1;
}

int fileLength(char *fileName) {
	FILE *fp = fopen(fileName,"r");
	if (!fp) {
		fprintf(stderr, "Error opening file '%s'\n", fileName);
		return -1;
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
		int s;
		if (len == 1) {
			s = 0;
		} else {
			s = 2;
		}
		char **split = splitLine(rdLine, s);
		//print(split);
		char ***args = splitParrallel(split);
		status = builtIns(args,path);
		len--;
		//printf("%zd\n",l);
	}
	fclose(fp);

	return status;
}

int main(int MainArgc, char *MainArgv[]){

	//print(MainArgv);

	size_t bufsize = 64;
	path = malloc(bufsize * sizeof(char*));
	path[0] = "/bin/";

	bool status = 1;
	char *line;
	char **instructions;
	char ***args;

	if (MainArgv[1] !=  NULL) {
		status = batchMode(MainArgv[1], path);
	} else {
		do {
			printf("witsshell> ");
			line = readLine();
			instructions = splitLine(line,1);
			args = splitParrallel(instructions);
			//print(args);
			status = builtIns(args,path);
		} while (status);
	}
	return(0);
}



