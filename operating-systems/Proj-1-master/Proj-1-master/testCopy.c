/*

\\\\\\\\\\\\                                                               >
===========================================================================>>>>
////////////                                                               >

STUDENTS    : Zachary Berryhill, Chris Lopes.
CLASS       : COP 4610 Intro To Operating Systems.
ASSIGNMENT  : Project 1
DATE        : 01/28/2019

notes: 
	-change getenv to getcwd.
	-make sure to do & save background execution.
	-put pipe commands in char** exe[] format.



*/

//															LIBRARIES USED.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>

//																ADDED GLOBALS.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

typedef struct
{
	char** tokens;
	int numTokens;
  char** cmds;
	char** redirFiles;
	char** flags;
} instruction;

#define psize 25
static u_int8_t child2shell = 0;
int commandcounter=0;
pid_t pidlist[psize] = {0};
int pidStatus[psize] = {0};

//											ADDED FUNCTIONS DECLARATIONS.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

char* Path_Resolution(char*, int);
u_int8_t IORedirection(char**, char**, u_int8_t);
u_int8_t Piping(char***, u_int8_t);
u_int8_t Which_Command(instruction*, char**);
u_int8_t separateTokenInst(instruction*);
u_int8_t Execute(char**, u_int8_t);
void grabParseInput(instruction*);
void recursivePiping(char***, int, int);
void copyCloseFd(int, int);
void closeFd(int);
void pipeExit(char*);
void prompt(void);
void BigExit(int exitcounter, pid_t pid[], int);
void JobsList(pid_t pid[], int);

//											GIVEN FUNCTIONS DECLARATIONS.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);

//																	MAIN.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int main(int argc, char* argv[], char* envp[])
{

	instruction instr; instr.tokens = NULL; instr.numTokens = 0;
	
	while(1)
	{
		prompt();
		grabParseInput(&instr);
		//printTokens(&instr);
		Which_Command(&instr, envp);
		clearInstruction(&instr);
		
	}
	
	printf("\n");
return 0;}

//											ADDED FUNCTIONS DEFINITIONS.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

u_int8_t IORedirection(char** exePath, char** filePath, u_int8_t fBexe)
{
	int fd1, fd2, status;
	
	char* outfile;
	
	if(filePath[0] != NULL)
	{
		fd1 = open(filePath[0], S_IRUSR | S_IROTH);
	}
	else
	{
		fd1 = -1;
	}
	if(filePath[1] == NULL)
	{
		fd2 = -1;
	}
	else
	{
		//resolve path.
		outfile = Path_Resolution(filePath[1], 1);
		
		//file exists.
		if(outfile != NULL)
		{
			remove(outfile);
		}
		else	//file does noto exist.
		{
			outfile = Path_Resolution(filePath[1], 0);
			fd2 = creat(outfile, S_IWUSR | S_IWOTH | S_IRUSR | S_IROTH);
		}
		
		fd2 = creat(outfile, S_IWUSR | S_IWOTH | S_IRUSR | S_IROTH);
	}

	pid_t pid = fork();

	switch(pid)
	{
		case -1:
		{
			printf("\nredirection: error creating fork.");
		}
		case 0:
		{
			
			//input redirection.
			if(fd1 != -1)
			{
				close(0);
				dup(fd1);
				close(fd1);
			}
			//output redirection.
			if(fd2 != -1)
			{
				close(1);
				dup(fd2);
				close(fd2);
			}
			execv(exePath[0], exePath);
		}
		default:
		{
			waitpid(pid, &status, 0);
//			if(fd1 != -1){close(fd1);}
//			if(fd2 != -1){close(fd2);}
			return 1;
		}
	}
	return 2;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
u_int8_t Execute(char** exe, u_int8_t fBexe)
{
	int status;
	
	printf("\ncommandcounter# = %d", commandcounter);
	int tmpCmdCntr = commandcounter;
	pidlist[commandcounter] = fork();
	
	switch(pidlist[commandcounter])
	{
		case -1:
		{
			perror("\nExecute(): error creating fork");
		}
		case 0:
		{
			
			execv(exe[0], exe);
		}
		default:
		{
			//background execution. continue running shell.
			if(fBexe)
			{
				waitpid(pidlist[tmpCmdCntr], &pidStatus[tmpCmdCntr], WNOHANG);
				
			}
			else	//foreground execution. wait for child process to finish.
			{
				waitpid(pidlist[tmpCmdCntr], &pidStatus[tmpCmdCntr], 0);
			}
			commandcounter++;
		}
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
u_int8_t Piping(char*** exe, u_int8_t fBexe)
{
	int status0;
	int i_pFd[2];
	
	pipe(i_pFd);
	pid_t fk0 = fork();
	
	switch(fk0)
	{
		case -1:
		{
			pipeExit("\nerror creating fk0.");
		}
		case 0:	//fk0
		{
			pid_t fk1 = fork();
			switch(fk1)
			{
				case -1:
				{
					pipeExit("\nerror creating fk1.");		
				}
				case 0:	//fk1
				{
					dup2(i_pFd[0],0);
					close(i_pFd[1]);
					execv(exe[1][0],exe[1]);
					perror("execv command 2"); exit(1);
				}
				default:	//parent of fk1 (fk0).
				{
					dup2(i_pFd[1],1);
					close(i_pFd[0]);
					execv(exe[0][0], exe[0]);
					perror("execv command 1"); exit(1);
				}
			}
		}
		default:	//shell (parent of fk0).
		{				
			waitpid(fk0, &status0, 0);
			close(i_pFd[0]); close(i_pFd[1]);
		}
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void prompt(void)
{
char* user=getenv("USER");
char* machine=getenv("MACHINE");
int PMAX=100;
char cwd[PMAX];
getcwd(cwd, sizeof(cwd));
printf("\n%s@%s:%s> ",user,machine,cwd);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void BigExit(int exitcounter, pid_t pid[],int size)
{
	int stat;
	int i=0;
	while (i<size)
	{
		pid_t cpid = waitpid(pid[i], &stat, 0);
		if (WIFEXITED(stat));
	 i++;
	}
	printf("Exiting Now!\n");
	printf("Commands Executed: %d\n",exitcounter);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void JobsList(pid_t pid[], int size)
{
	int i=0;
	int stat;

	while(i<size)
	{
		pid_t return_pid = waitpid(pid[i], &stat, WNOHANG); /* WNOHANG def'd in wait.h */
		if (return_pid == -1)
		{
			/* error */
		}
		if (return_pid == 0)
		{
			printf("[%d] + [%d][-----]\n",i,pid[i]);
		}		
		i++;
  }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//produces garbage on stdin. Otherwise workds fine. not using for that
//reason.

void recursivePiping(char** exe[], int indx, int inFd)
{
	if(exe[indx + 1] == NULL)
	{
		int lPidStatus;
		pid_t lPid = fork();
		switch(lPid)
		{
			case -1:
			{
				pipeExit("\nerror creating last child process.");
			}
			case 0:
			{
				child2shell = 1;
				copyCloseFd(inFd, STDIN_FILENO);
				execv(exe[indx][0], exe[indx]);
				pipeExit("\nerror with last execcution.");
			}
			default:
			{
				waitpid(lPid, &lPidStatus, 0);
			}
		}
	}
	else
	{
		int i_pFd[2];
		if(pipe(i_pFd) == -1)
		{
			pipeExit("\nerror creating pipe.");
		}
		switch(fork())
		{
			case -1:
			{
				pipeExit("\nerror creating child process.");
			}
			case 0:
			{
				child2shell = 1;
				closeFd(i_pFd[0]);
				copyCloseFd(inFd, STDIN_FILENO);
				copyCloseFd(i_pFd[1], STDOUT_FILENO);
				execv(exe[indx][0], exe[indx]);
				pipeExit("\nerror with execcution.");
			}
			default:
			{				
				closeFd(i_pFd[1]);
				closeFd(inFd);
				recursivePiping(exe, (indx + 1), i_pFd[0]);
			}
		}
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void copyCloseFd(int i_oFd, int i_nFd)
{
	if(i_oFd != i_nFd)
	{
		if(dup2(i_oFd, i_nFd) != -1)
		{
			closeFd(i_oFd);
		}
		else
		{
			pipeExit("\nerror with dup2.");
		}
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void closeFd(int fd)
{
	int fd2close = fd;
	if(close(fd2close) == -1)
	{
		perror("\nerror closing fd.");
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void pipeExit(char* errMsg)
{
	perror(errMsg);
	(child2shell ? _exit : exit)(EXIT_FAILURE);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

char* Path_Resolution(char* consolePath, int vYorN)
{
	//type: char pointer. name: real path pointer.
	char* rp_ptr;
	char* homePath;
	int maxPathLen = 100;
	if(consolePath[0] == '~')
	{
		homePath = getenv("HOME");
		char toRealPath[strlen(homePath) + strlen(consolePath) + 1];
		strcpy(toRealPath, homePath);
		strcat(toRealPath, consolePath + 1);
		char* resolvedPath = (char*)malloc((strlen(toRealPath) + 1) * sizeof(char));
		rp_ptr = realpath(toRealPath, resolvedPath);
		if(vYorN)
		{
			return rp_ptr;
		}
		else
		{
			return resolvedPath;
		}
	}
	else
	{
		char* resolvedPath = (char*)malloc((strlen(consolePath) +
					(maxPathLen) + 1) * sizeof(char));
		rp_ptr = realpath(consolePath, resolvedPath);
		if(vYorN)
		{
			return rp_ptr;
		}
		else
		{
			return resolvedPath;
		}
	}
	return NULL;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//returns absolute path of EXE or NULL if not found.
char* checkPathExe(char* instToken)
{
	char* pathTokenOg = getenv("PATH");
	char pathToken[strlen(pathTokenOg) + 1];
	
	strcpy(pathToken, pathTokenOg);
	
	char* path = strtok(pathToken, ":");
	
	while(path != NULL)
	{
		char findexe[strlen(instToken) + strlen(path) + 2];
			
		strcpy(findexe, path);
		strcat(findexe, "/");
		strcat(findexe, instToken);
			
		if(access(findexe, X_OK) == 0)
		{
			char* retCharArr = (char*)malloc((strlen(findexe) + 1) * (sizeof(char)));
	
			strcpy(retCharArr, findexe);
			
			return retCharArr;
		}
		else
		{
			path = strtok(NULL, ":");
		}
	}
	
	return NULL;	
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

u_int8_t Which_Command(instruction* instr, char** envp)
{
  //type: string; name: token 0.
	//description: holds first token (token 0)
	
	char* s_tok0 = NULL;
	int indx = 0;
	
	if((instr->numTokens - 1) > 0)
	{
		s_tok0 = instr->tokens[0];
	}
	else
	{
		return 0;
	}
	
	//Builtins Variables.
  if(!strcmp(s_tok0, "echo"))
	{
		if(instr->tokens[1][0] == '$')
		{
			printf("\n%s", getenv((instr->tokens[1] + 1)));
		}
		else
		{
			if(instr->tokens[1] != NULL)
			{
				printf("\n%s", instr->tokens[1]);
			}
		}
		
	}
	else if(!strcmp(s_tok0, "env"))
  {
		//list all environment variables.
		while(envp[indx] != NULL)
		{
			printf("\n%s", envp[indx++]);
		}
		indx = 0;
		printf("\n");
	}
	else if(!strcmp(s_tok0, "exit"))
	{
		//wait here for bacground processes to finish.
		BigExit(commandcounter,pidlist,psize);
		return 25;
		//also need to print number of commands executed.
		//printf("\nExiting Now!");
	}
	else if(!strcmp(s_tok0, "cd"))
	{
		if(instr->tokens[1] != NULL)
		{
			char* cd = Path_Resolution(instr->tokens[1], 1);
			int maxPathLen = 100 + strlen(cd);
			char newPath[maxPathLen];
			if(cd != NULL)
			{
				chdir(cd);
			}
		}
	}
	else if(!strcmp(s_tok0, "jobs"))
  {
		//display background processes.
		//printf();
		indx = 0;
		while(indx < 25)
		{
			if((long)pidlist[indx] != 0)
			{	
				printf("\njobs[%d] = %ld + [%d]", indx, (long)pidlist[indx],
					pidStatus[indx]);
			}
			indx++;
		}
		indx = 0;
		while(indx < 25)
		{
			printf("\nstatus[%d] = %d", indx, pidStatus[indx]);
			indx++;
		}
	}
  else
  {
		separateTokenInst(instr);
		
  }
	return 1;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

u_int8_t separateTokenInst(instruction* instr)
{
	//validCmd:
		//0x80: redirection character found.
		//0x40: piping char found.
		//0x20: error. invalid command.
		
	//fBexe:
		//0x00: foreground execution.
		//0x01: bacground execution.

	int numTokens = instr->numTokens, indx = 0;
	u_int8_t validCmd = 0x00, fBexe = 0;
	char** tokens = instr->tokens;
	char** redirFiles = (char**)calloc(numTokens, sizeof(char*));
	char** cmds = (char**)calloc(numTokens, sizeof(char*));
	char** flags = (char**)calloc(numTokens, sizeof(char*));
	
	for(indx = 0; indx < (numTokens - 1); indx++)
	{
		if(tokens[indx][0] == '<')
		{
			redirFiles[0] = Path_Resolution(tokens[indx + 1], 1);

			//invalid redirection command
			if((indx == 0) || (redirFiles[0] == NULL))
			{
				printf("\ninvalid input file");
				validCmd |= 0x20;
				return 0;
			}
			validCmd |= 0x80;
			indx++;
		}
		else if(tokens[indx][0] == '>')
		{
			redirFiles[1] = tokens[indx + 1];
			//invalid redirection command.
			if(indx == 0)
			{	
				printf("\ninvalid command.");
				validCmd |= 0x20;
				return 0;
			}
			validCmd |= 0x80;
			indx++;
		}
		else if(tokens[indx][0] == '|')
		{
			validCmd |= 0x40;
			
			//must be in the path 
			if((tokens[indx + 1][0] == '.') || (tokens[indx + 1][0] == '~')
						|| (tokens[indx + 1][0] == '/'))
			{
				cmds[indx + 1] = Path_Resolution(tokens[indx + 1], 1);
			}
			else
			{
				cmds[indx + 1] = checkPathExe(tokens[indx + 1]);
			}
			//invalid piping command.
			if((indx == 0) || (cmds[indx + 1] == NULL))
			{
				validCmd |= 0x20;
				return 0;
			}
			indx++;
		}
		else if(tokens[indx][0] == '-')
		{
			if(indx > 0)
			{
				flags[indx - 1] = tokens[indx];
			}
			else
			{
				validCmd |= 0x20;
				return 0;
			}
		}
		else if(tokens[indx][0] == '&')
		{
			//invalid background execution command. 
				if(indx != (numTokens - 2))
				{
					validCmd |= 0x20;
					printf("\nnon-valid background command.");
					return 0;
				}
			else
			{
				printf("\nvalid background command.");
				fBexe = 1;
			}
		}
		else
		{
			if(indx == 0)
			{
				//must be in the path 
				if((tokens[indx][0] == '.') || (tokens[indx][0] == '~')
						|| (tokens[indx][0] == '/'))
				{
					cmds[indx] = Path_Resolution(tokens[indx], 1);
				}
				else
				{
					cmds[indx] = checkPathExe(tokens[indx]);
					
				}
				if(cmds[indx] == NULL)
				{
					validCmd |= 0x20;
					return 0;
				}
			}
			else	//must be a cmd with a file parameter.
			{
				flags[indx - 1] = Path_Resolution(tokens[indx], 1);
				if(flags[indx - 1] == NULL)
				{
					validCmd |= 0x20;
					return 0;
				}
			}
		}
	}
	
	//instr->cmds = (char**)malloc(numTokens * sizeof(char*));
	instr->cmds = cmds;
	instr->flags = flags;
	instr->redirFiles = redirFiles;

	char* exe0[3] = {NULL};
	char* exe1[3] = {NULL};
	
	indx = 0;
	int exeIndx = 0;
	while(indx < (numTokens - 1))
	{
		if(cmds[indx] != NULL)
		{
			if(exeIndx == 1)
			{
				exe1[0] = cmds[indx];
				exeIndx++;
				if(flags[indx] != NULL)
				{
					//save command, flag, & NULL.
					exe1[1] = flags[indx];
					exe1[2] = NULL;
				}
				else
				{	
					exe1[1] = NULL;
				}
			}
			else if(exeIndx < 1)
			{
				exe0[0] = cmds[indx];
				exeIndx++;
				if(flags[indx] != NULL)
				{
					//save command, flag, & NULL.
					exe0[1] = flags[indx];
					exe0[2] = NULL;
				}
				else
				{	
					exe0[1] = NULL;
				}
			}
			else
			{
				//do nothing
				break;
			}
		}
		indx++;
	}
	
	char** exe[] = {exe0, exe1};

	//redirection.
	if((validCmd == 0x80))
	{
		IORedirection(exe[0], instr->redirFiles, fBexe);
	}
	//piping.
	else if(validCmd == 0x40)
	{
		Piping(exe, fBexe);
	}
	//execution
	else if(validCmd == 0x00)
	{
		printf("\ncalling execute.");
		Execute(exe[0], fBexe);
	}
	else
	{
		printf("\ninvalid command");
		return 0;
	}
return 1;}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void grabParseInput(instruction* instr)
{
	char* token = NULL;
	char* temp = NULL;
	
	printf("\n\nPlease enter an instruction: ");
	
	do
	{
		//scans for next token and allocates token var to size of scanned token
		scanf("%ms", &token);
		temp = (char*)malloc((strlen(token) + 1) * sizeof(char));

		int i;
		int start = 0;
		for (i = 0; i < strlen(token); i++)
		{
			//pull out special characters and make them into 
			//a separate token in the instruction
			if (token[i] == '|' || token[i] == '>' || token[i] == '<' 
						|| token[i] == '&')
			{
				if (i-start > 0)
				{
					memcpy(temp, token + start, i - start);
					temp[i-start] = '\0';
					//addToken(&instr, temp);
					addToken(instr, temp);
					
				}

				char specialChar[2];
				specialChar[0] = token[i];
				specialChar[1] = '\0';

				//addToken(&instr,specialChar);
				addToken(instr,specialChar);

				start = i + 1;
			}
		}

		if (start < strlen(token))
		{
			memcpy(temp, token + start, strlen(token) - start);
			temp[i-start] = '\0';
			//addToken(&instr, temp);
			addToken(instr, temp);
		}

		//free and reset variables
		free(token);
		free(temp);

		token = NULL;
		temp = NULL;
	}while('\n' != getchar());    //until end of line is reached

	//addNull(&instr);
	addNull(instr);
}

//											GIVEN FUNCTION DEFINITIONS.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//#############################################################################
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr_ptr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens,
			(instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = 
		(char *)malloc((strlen(tok)+1) * sizeof(char));
		
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens,
			(instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void printTokens(instruction* instr_ptr)
{
	int i;
	printf("Tokens:\n");
	
  for (i = 0; i < instr_ptr->numTokens; i++) {
		if ((instr_ptr->tokens)[i] != NULL)
			printf("%s\n", (instr_ptr->tokens)[i]);
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);
		
		//free(instr_ptr->cmds[i]);
		//free(instr_ptr->flags[i]);
		//free(instr_ptr->redirFiles[i]);

	free(instr_ptr->tokens);
	//free(instr_ptr->cmds);
	//free(instr_ptr->flags);
	//free(instr_ptr->redirFiles);

	instr_ptr->tokens = NULL;
	//instr_ptr->cmds = NULL;
	//instr_ptr->flags = NULL;
	//instr_ptr->redirFiles = NULL;
	instr_ptr->numTokens = 0;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
