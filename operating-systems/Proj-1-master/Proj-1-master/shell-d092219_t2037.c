/*

\\\\\\\\\\\\                                                              >
==========================================================================>>>>
////////////                                                              >

STUDENTS    : James Hardy, Sean Collard, Zachary Berryhill.
CLASS       : COP 4610 Intro To Operating Systems.
ASSIGNMENT  : Project 1
DATE        : 09/22/2019

*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<errno.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

typedef struct{char** tokens; int numTokens;} instruction;

//function definitions.
char* getEnvVar(char*);
void addToken(instruction*, char*);
void cleanTokens(instruction*);
void replaceToken(instruction*, int, char*);

int main()
{
  char* token = NULL;
  char* temp = NULL;
  bool validInstr;
  bool execExists;
  bool envVar;
  bool goodIO;

//cPa_cmd1OpL type: char pointer array. name: command 1 operation list 
  char** cPa_cmd1OpL = NULL;
  char** cPa_cmd2OpL = NULL;
  char** cPa_cmd3OpL = NULL;


  instruction instr;
  instr.tokens = NULL;
  instr.numTokens = 0;
  
  while(1)
  {
    goodIO = true;
    //Prompts the user for input within the current working directory
    printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
    //Input into instruction
    
    do
    {
      scanf("%ms", &token);
      temp = (char*)malloc((strlen(token) + 1) * sizeof(char));
      int i;
      int start = 0;
 
      for(i = 0; i < strlen(token); i++)
      { if(token[i] == '|' || token[i] == '<' || token[i] == '>' || token[i] == '&')
        {
            
          if(i-start > 0)
          { memcpy(temp, token + start, i - start);
            temp[i-start] = '\0';
            addToken(&instr, temp);
          }

          char specialChar[2];
          specialChar[0] = token[i];
          specialChar[1] = '\0';
          addToken(&instr, specialChar);
          start = i + 1;
        } 
      }

      if(start< strlen(token))
      { memcpy(temp, token + start, strlen(token) - start);
        temp[i-start] = '\0';
        addToken(&instr, temp);
      }

      free(token);
      free(temp);
      token = NULL;
      temp = NULL;
    }while('\n' != getchar());

    validInstr = true;

//built ins
    if(!strcmp(instr.tokens[0],"echo"))
    { int j;
      for(j = 1; j < instr.numTokens; j++)
      { if(instr.tokens[j][0] == '$' && getEnvVar(instr.tokens[j]) != NULL)
          {replaceToken(&instr, j, getEnvVar(instr.tokens[j]));}
        else if(instr.tokens[j][0] == '$' && getEnvVar(instr.tokens[j]) == NULL)
          {printf("%s: unidentified variable\n", instr.tokens[j]);}
      }
        
      for(j = 1; j < instr.numTokens; j++){printf("%s ", instr.tokens[j]);}
      
      validInstr = false;
      printf("\n");
    }
    if(!strcmp(instr.tokens[0], "cd"))
    {
      chdir(instr.tokens[1]);
      goto eOwhile;
    }

    if(!strcmp(instr.tokens[0],"exit") && validInstr){return 0;}

//Environment Var and Shortcut Resolution
    int i;
    //type: unsigned int (8 bit) vector. size: 2 name: redirection piping position.
    u_int8_t ui8v2_rPpos[2] = {0};//never more than 2 redirection/pipe chars. 
    //name: redirection piping indx
    u_int8_t ui8_rPidx = 0;

    for(i = 0; i < instr.numTokens && validInstr; i++)
    {
        envVar = false;
        
        if(instr.tokens[i][0] == '$' && getEnvVar(instr.tokens[i]) != NULL)
        { replaceToken(&instr,i,getEnvVar(instr.tokens[i]));
          envVar = true;
        }
        else if(instr.tokens[i][0] == '$')
        { validInstr = false;
          printf("%s: unidentified variable\n", instr.tokens[i] + 1);
        }
        else if(instr.tokens[i][0] == '.')
        {    
          if(instr.tokens[i][1] == '.')
          { if(instr.tokens[i][2] == '/' || instr.tokens[i][2] == '\0')
            {
              if(getenv("PWD") != "/"){temp = getEnvVar("$PWD");}
                  
              int j = strlen(temp) - 1;
                      
              while(temp[j] != '/')
              { temp[j] = '\0';
                  j--;
              }

              if(j != 0){temp[j] = '\0';}
              
              replaceToken(&instr,i,strcat(temp, instr.tokens[i] + 2));
              temp = NULL;        
            }
            else
            { printf("Error: No directory above root\n");
              validInstr = false;
            }

          }
          else if(instr.tokens[i][1] == '/' || instr.tokens[i][1] == '\0')
          { replaceToken(&instr, i, strcat(getEnvVar("$PWD"),
                  instr.tokens[i] + 1));
//***************How to do CD********************
//replaceToken(&instr, i, strcat(getenv("PWD"), instr.tokens[i] + 1));
          }

        }
        else if(instr.tokens[i][0] == '~' 
                && (instr.tokens[i][1] == '/' 
                || instr.tokens[i][1] == '\0'))
          {replaceToken(&instr, i, getEnvVar("$HOME"));}
        
        else if(instr.tokens[i][0] == '/')
        {
          //Root directory
        }
       
        else if(!strcmp(instr.tokens[i],"<") 
                || !strcmp(instr.tokens[i],">") 
                || !strcmp(instr.tokens[i],"|"))
        {
            //Redirect or Pipe
            if(ui8_rPidx < 2){ui8v2_rPpos[ui8_rPidx++] = i;}
	    else{goodIO = false;}//to many pipes/redirects.
        }
        else if(instr.tokens[i][0] == '-' && i != 0)
        {
//flag.
        }
        else
        { if(!(strchr(instr.tokens[i],'/') != NULL))
          {
            int j = 0;
            int start = 0;
            char* cp_op;
            execExists = false;

//path resolution.
            temp = malloc(strlen(getenv("PATH"))*sizeof(char));
            temp = getEnvVar("$PATH");
            while(temp[j] != '.' && !execExists)
            {
              while(temp[j] != ':'){j++;}

               //cp_op character pointer one path
              cp_op = malloc((j - start + strlen(instr.tokens[0]))*sizeof(char));

             //copy temp(directory) until the colon into cp_op
              strncpy(cp_op,temp + start ,j - start);
              j = start = j+1;

//cp_op now contains the path to search for the command
//concatenate path with the command
              strcat(strcat(cp_op, "/"), instr.tokens[0]);
              
//lookup if the path is valid.
              if(access(cp_op,X_OK) != -1)//file exists.
              { replaceToken(&instr, i, cp_op);
                execExists = true;
              }

            }
//Checking current directory
            cp_op = strcat(strcat(getEnvVar("$PWD"), "/"), instr.tokens[i]);

            if(access(cp_op,X_OK) != -1)
            {
              replaceToken(&instr, i, cp_op);
              execExists = true;
            }
             if(!execExists){
               printf("%s: Command not found\n", instr.tokens[i]);
               validInstr = false;
             }
             temp = NULL;
           }//E.O. if strchr "first character" != NULL
          
          else if(validInstr)//find relative path
          { temp = getEnvVar("$PWD");
            strcat(strcat(temp, "/"), instr.tokens[i]);
            replaceToken(&instr, i, temp);
          }
         }//E.O. else line 176

//check for file existence
         if(instr.tokens[i][0] == '/' && !envVar)
         {  if(access(instr.tokens[i], F_OK) == -1)
           {  printf("%s: Invalid File Path\n", instr.tokens[i]);
              validInstr = false;
           }
         }
       }
   
//========== execv char** writing, command checking / file checking==========

//the below code can be used for redirection commands if needed.

//type: unsigned interpret (8 bit). name: pipe or redirect.
//0x00 (not valid), 0x01 (piping), 0x02 (redirection) 
  u_int8_t ui8_pOrR = 0x00;

  if(ui8_rPidx != 0)//at least one | or <,> char.
  {
    //make sure command after | or <,> char is valid.
    if((access(instr.tokens[0],X_OK) == -1)){goodIO = false;}
    else if((*instr.tokens[0] == '|') 
             || (*instr.tokens[instr.numTokens-1] == '|')
             || (*instr.tokens[0] == '<') || (*instr.tokens[0] == '>')
             || (*instr.tokens[instr.numTokens-1] == '<')
             || (*instr.tokens[instr.numTokens-1] == '>'))
      {goodIO = false;}//first or last char | or <,> char.
    
    else//command 1 valid and | or <,> char found.
    { 
      if(*instr.tokens[ui8v2_rPpos[0]] == '|')//1st spec char is |.
      	{ui8_pOrR = 0x01;}
      else if((*instr.tokens[ui8v2_rPpos[0]] == '<') || (*instr.tokens[ui8v2_rPpos[0]] == '>'))
        {ui8_pOrR = 0x02;}
      else{goodIO = false;}//for clarity.

      //command 2 is not a valid executable.
      if((ui8_pOrR == 0x01) && (access(instr.tokens[ui8v2_rPpos[0] + 1], X_OK) == -1))
        {goodIO = false;}
      else if((ui8_pOrR == 0x02) && (access(instr.tokens[ui8v2_rPpos[0] + 1], F_OK) == -1))
        {goodIO = false;}
      else//command 2 is a valid executable or file.
      {
      //when going through the next bits of code think of
      //  ls -al | wc -w | ./something -S
      //  0  1   2 3  4  5 6           7
      //  ui8v2_rPpos[0] = 2;first special character position.
      //  ui8v2_rPpos[1] - ui8v2_rPpos[0] =  3;special character 2 positions - SC 1 pos.
      //  instr.numTokens - ui8v2_rPpos[1] = 3;

        //instruction 2 length
        u_int8_t ui8_cmd2OpLidx;

        if(ui8_rPidx == 2){ui8_cmd2OpLidx = ui8v2_rPpos[1] - ui8v2_rPpos[0];}

        else{ui8_cmd2OpLidx = instr.numTokens - ui8v2_rPpos[0];}

        //initialize instruction 1 char** to correct length.
        cPa_cmd1OpL = malloc(sizeof(char*)*(ui8v2_rPpos[0] + 1));

        //initialize instruction 2 to correct length.
      //==========check this is trouble with passing into execv. maybe to long.==========          
        cPa_cmd2OpL = malloc(sizeof(char*)*ui8_cmd2OpLidx);
        u_int8_t j = 0;//keeps track of which second instruction.

        //writes command 1 to appropriate char**. 
        for(i = 0; i < ui8v2_rPpos[0]; i++)
        {
          //allocate instruction / flag / filename size.
          cPa_cmd1OpL[i] = malloc(sizeof(char)*(strlen(instr.tokens[j]) + 1));
          //writes instruction / flag / filename size.
          cPa_cmd1OpL[i] = instr.tokens[j];
          j++;
        }
        //write command 2 to appropriate char**.
        j = 1;
        for(i = 0; i < (ui8_cmd2OpLidx - 1); i++)
        {
          //allocate instruction / flag / filename size.
          cPa_cmd2OpL[i] = 
            malloc(sizeof(char)*(strlen(instr.tokens[ui8v2_rPpos[0] + j]) + 1));
          //write to array.
          cPa_cmd2OpL[i] = instr.tokens[ui8v2_rPpos[0] + j];
          j++;	
        }
        //instruction 1 and two have now been written

        if(ui8_rPidx == 2)//a second | or <,> character exists.
        { 
          //command 3 is not a valid executable.
          if((ui8_pOrR == 0x01) && (access(instr.tokens[ui8v2_rPpos[1] + 1], X_OK) == -1))
            {goodIO = false;}
          else if((ui8_pOrR == 0x02) && (access(instr.tokens[ui8v2_rPpos[1] + 1], F_OK) == -1))
            {goodIO = false;}
          else//command 3 is valid execuatble.
          {  
            //instruction 3 length.
            u_int8_t ui8_cmd3OpLidx = ((instr.numTokens) - ui8v2_rPpos[1]);
            cPa_cmd3OpL = malloc(sizeof(char*)*ui8_cmd3OpLidx);
            j = 1;
            //writes command 3 to appropriate char**
            for(i = 0; i < (ui8_cmd3OpLidx - 1); i++)
            {   
              cPa_cmd3OpL[i] = 
              malloc(sizeof(char)*(strlen(instr.tokens[ui8v2_rPpos[1] + j]) + 1));
              cPa_cmd3OpL[i] = instr.tokens[ui8v2_rPpos[1] + j];
              j++;	
            } 
          }
        }
      }
    }
  }


//=======E.O. char** writing for redirection and piping=======


//=================REDIRECTION================
    if(!goodIO){printf("\nerror (badIO)\n");}

    else if(ui8_pOrR == 0x02)
    {
      int fd1 = -1, fd2 = -1;
      fd1 = open(instr.tokens[i+1], S_IRUSR | S_IROTH);
      fd2 = creat(instr.tokens[i+1], S_IWUSR | S_IWOTH | S_IRUSR | S_IROTH);

      //i_pFd[2] type: integer. name: pipe file descriptor.
      int status;

      pid_t pid = fork();

//child process
      if(pid == 0)
      { 
//set fd1 to input stream
        if(fd1 != -1)
        { close(0);
          dup(fd1);
          close(fd1);
        }

//set fd2 to output stream
       if(fd2 != -1)
       { close(1);
         dup(fd2);
         close(fd2);
       }

//done with redirection. clean tokens.
//should just call clean tokens here.       
       free(instr.tokens[2]);
       free(instr.tokens[1]);
       instr.tokens[1] = NULL;
       instr.tokens[2] = NULL;
       execv(instr.tokens[0], instr.tokens);
       printf("\nproblem\n");
       exit(1);
      }
//fault creating child.  
      else if(pid == -1)
      { printf("\npiping error (could not start child process)\n");
        exit(1);
      }

//parent process.
     else
     { waitpid(pid, &status, 0);
       if(fd1 != -1){close(fd1);}
       if(fd2 != -2){close(fd2);}
     }
     goto eOwhile;
   }//E.0. if(foundRedirect).

//=================E.O. REDIRECTION=================

//=================PIPING=================
  else if(ui8_pOrR == 0x01)
  {
      int status;
      int i_pFd[2];
      pipe(i_pFd);
      pid_t pid_fk1 = fork();
      if(pid_fk1 == -1){perror("fork"); exit(1);}
      if(pid_fk1 == 0)//we are in child process 1
      {
        pid_t pid_fk2 = fork();
        if(pid_fk2 == 0)//child process 2 code.
        {
          dup2(i_pFd[0],0);
          close(i_pFd[1]);
          execv(cPa_cmd2OpL[0],cPa_cmd2OpL);
          perror("execv command 2"); exit(1);
        }
        else//child process 1 code.
        {
          dup2(i_pFd[1],1);
          close(i_pFd[0]);
          execv(cPa_cmd1OpL[0], cPa_cmd1OpL);
          perror("execv command 2"); exit(1);
        }
        waitpid(pid_fk2, &status, 0);

/* code segment for double pipe commands.
        if(cPa_cmd3OpL != NULL)
        {
          pid_t pid_fk3 = fork();
          if(pid_fk3 == 0)
          {
            dup2(i_pFd[],);
          }
        }
*/
      }
      else{waitpid(pid_fk1, &status, 0); close(i_pFd[0]); close(i_pFd[1]);}
      
      goto eOwhile;
  }

//=================E.O. PIPING================= 


//evaluate instruction.
    else if(execExists)
    {
//foreground

      pid_t pid = fork();
      int status;
      if(pid == 0)//execv from child process.
      {
//new process image.
//execv() takes parameters as shown.
//  e.x. absolute path to command, (name of command, flags, filename, NULL).

//  e.x. 2 
//    char* arg[] = {"ls", "-al", "./file.txt", NULL}
//    execv("/bin/ls",arg);
//    filename and flags are optional.
       execv(instr.tokens[0],instr.tokens);
       printf("Problem executing %s\n", instr.tokens[0]);
       exit(1);
      }
      
      else if(pid == -1)//child process was not created sucessfully.
        {exit(1);}
      
      else//wait for the child process to finish executing.
        {waitpid(pid, &status, 0);}
    }

eOwhile:
    cleanTokens(&instr);
    if(cPa_cmd1OpL != NULL){free(cPa_cmd1OpL);} 
    if(cPa_cmd2OpL != NULL){free(cPa_cmd2OpL);}
    if(cPa_cmd3OpL != NULL){free(cPa_cmd3OpL);}
    cPa_cmd1OpL = NULL; cPa_cmd2OpL = NULL; cPa_cmd3OpL = NULL;
  }
return 0;}//=================E.O. Main=================

//Adds token to instruction token list
void addToken(instruction* instr_ptr, char* tok){
    if (instr_ptr->numTokens == 0)
        instr_ptr->tokens = (char**) malloc(sizeof(char*));
    else
        instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));  
    instr_ptr->tokens[instr_ptr->numTokens] = (char*)malloc((strlen(tok)+1)*sizeof(char));
    strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);
    instr_ptr->numTokens++;
}

//Memory clean up
void cleanTokens(instruction* instr_ptr){
    int i;
    for(i = 0; i < instr_ptr->numTokens; i++)
        free(instr_ptr->tokens[i]);
    free(instr_ptr->tokens);
    instr_ptr->tokens = NULL;
    instr_ptr->numTokens = 0;
}

//Function returns either enviornment variables, the input string, or NULL if user types in a false environment variable
char* getEnvVar(char* str_ptr){
  if(str_ptr[0] == '$' && getenv(str_ptr + 1) != NULL){
    char* temp = (char*) malloc((strlen(str_ptr))*sizeof(char));
    strcpy(temp, str_ptr + 1);
    str_ptr = malloc((strlen(getenv(temp))*sizeof(char)));
    strcpy(str_ptr, getenv(temp));
    return str_ptr;
  }else
    return NULL;
}

void replaceToken(instruction* instr_ptr, int place, char* repTok){
  char* temp;
  temp = (char*)malloc(strlen(repTok)*sizeof(char));
  temp = repTok;
  instr_ptr->tokens[place] = realloc(instr_ptr->tokens[place], strlen(temp));
  strcpy(instr_ptr->tokens[place], temp);
}

