/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/ 
#include "quash.h" // Putting this above the other includes allows us to ensure
// this file's headder's #include statements are self
// contained.
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

/**************************************************************************
 * Private Variables
 **************************************************************************/
/**
 * Keep track of whether Quash should request another command or not.
 */
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;
sigjmp_buf env;
int status;

/**************************************************************************
 * Private Functions 
 **************************************************************************/
/**
 * Start the main loop by setting the running flag to true
 */
static void start() {
  running = true;
}

/**************************************************************************
 * Public Functions 
 **************************************************************************/
void catchChild(int signum)//child return or terminate callback
{
    //printf("Child died!%d\n",signum);
    pid_t pid;
    int status;
    while((pid = waitpid(-1,&status,WNOHANG)) != -1)
    {
        delete_from_list(pid);
        //printf("I got in the while!\n");
        siglongjmp(env,1);
    }
    //printf("PID: %1d\n",pid);
}

void pwd()
{
  printf("%s\n",WKDIR);
  return;
}

void cd(command_t cmd)
{
    if (cmd.execArgs[1] == NULL)
    {
        if (HOME[0] == '\0')
        {
            printf("HOME was NULL. No change made to the Working directory.");
            return;
        }

        printf("No directory specified\n");
        strcpy( WKDIR, HOME );
    }
    else
    {
        strcat( WKDIR, "/");
        strcat( WKDIR, cmd.execArgs[1] );
    }
    return;
}

void reapChild()
{
    int pid = waitpid(-1,NULL, WNOHANG);
    printf("PID: %1d Finished",pid);
    return;
}

void jobs()
{
    print_list();
    //printf("Not Yet Implimented.\n");
    return;
}

bool pathExists(char* path)
{ 
    FILE * file = fopen(path,"r");
    if (file!=NULL)
    {
        fclose(file);
        return true;
    }
    return false;
}

void testPath(char * testPath, char * testReturn)
{
    char tempPath[MAX_PATH_LENGTH], pathHolder[MAX_PATH_LENGTH];
    char * temptok;

    strcpy(testReturn, testPath);
    strcpy(pathHolder, PATH);

    if(testPath[0] == '/')
    {
        return;
    }

    strcpy( tempPath, WKDIR );
    strcat( tempPath, "/" );
    strcat( tempPath, testPath );

    if( pathExists( tempPath ) )
    {
        strcpy( testReturn, tempPath );
        return;
    }

    else
    {
        if (pathHolder == '\0')
        {
            printf("PATH is NULL\n");
            return;
        }
        temptok = strtok(pathHolder,":");

        while(temptok!=NULL)
        {
            strcpy(tempPath,temptok);
            strcat(tempPath,"/");
            strcat(tempPath, testPath);

            if(pathExists(tempPath))
            {
                strcpy(testReturn, tempPath);
                return;
            }
            temptok = strtok(NULL,":");
        }

    }
    return;
}

int exec_cmd(command_t cmd)
{
    char test[MAX_PATH_LENGTH];

    testPath(cmd.execArgs[0],test);

	int pid = fork();
	if(!pid)
    {
        if ((cmd.execNumArgs > 2)&& (strchr(cmd.cmdstr,'|')!= NULL)){
            char tmpcmdstr [MAX_COMMAND_LENGTH];
            strcpy(tmpcmdstr,cmdstr);

//             
// 
// 
// 
// int npipes;
// int status;
// char buf[4096];
// pid_t * pid_a[npipes+1];
// int * fd_a[(npipes+1)*2];
// int rsize;
// 
// for(int  = 0; i<pipes+1;){pipe(fd_a[i]);i=i+2;}
// 
// for(int i=0;i<(npipes+1);i++){
//  pid_a[i]=fork();
//  if(!pid_a[i]){
//      dup2();
//      dup2();
//      for(int j = 0;j<[(npipes+1)*2],j++){close(fd_a[j])}
// 
// 
// 
//  }
// }



            printf("There was a | in the cmd str\n");
        }
        else if ((cmd.execNumArgs > 2) && (cmd.execArgs[cmd.execNumArgs-2][0] == '<'))
        {          
            char string[MAX_COMMAND_LENGTH];
            FILE * file = fopen(cmd.execArgs[cmd.execNumArgs-1], "r");

            while(fgets(string, MAX_COMMAND_LENGTH, file) != NULL)
            {
                char* args[MAX_PATH_LENGTH];

                size_t len = strlen(string);
                char last_char = string[len - 1];

                if (last_char == '\n' || last_char == '\r')
                {
                    // Remove trailing new line character.
                    string[len - 1] = '\0';
                    len = len - 1;
                }

                args[0] = test;

                char * temp;
                temp = strtok(string," ");
                args[1] = temp;
                
                int i = 2;
                while(((temp = strtok(NULL," ")) != NULL) && (i<255))
                {
                    args[i] = temp;
                    i++;
                }
                args[i] = NULL;

                int pid2 = fork();
                if(!pid2)
                {
                    //fprintf(stderr,"in fork2 %s\n", args[1]);
                    if(execv(test,args)<0)
                    {
                        fprintf(stderr, "Error execing %s. Error# %d\n",cmd.cmdstr, errno);
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    if((waitpid(pid2,&status,0))==-1)
                    {
                        fprintf(stderr, "Process encountered an error. ERROR%d", errno);
                        return EXIT_FAILURE;
                    }
                }
                
            }
            
        }
        else if ((cmd.execNumArgs > 2) && (cmd.execArgs[cmd.execNumArgs-2][0] == '>'))
        {
            cmd.execArgs[cmd.execNumArgs-2] = NULL;
            //replace text.txt with last argument
            //remove text.txt and > arg
            //call exec as normal
            int file = open(cmd.execArgs[cmd.execNumArgs-1],O_CREAT|O_WRONLY,S_IRWXU);
            dup2(file, 1);

            if(execv(test,cmd.execArgs)<0)
            {
                fprintf(stderr, "Error execing %s. Error# %d\n",cmd.cmdstr, errno);
                exit(EXIT_FAILURE);
            }
            
            //puting things back the way they were.
            close(file);
            dup2(1,1);

            //fprintf(stderr, "\nEname = %s, str = %s\n",cmd.execArgs[0], cmd.execArgs[1]);
        }
        else
        {
            if(execv(test,cmd.execArgs)<0)
            {
                fprintf(stderr, "Error execing %s. Error# %d\n",cmd.cmdstr, errno);
                exit(EXIT_FAILURE);
            }
        }
        exit(0);
	}
    else{
        if(!cmd.execBg)
        {
            //add pid and command w/o path to jobs structure
            if((waitpid(pid,&status,0))==-1)
            {
                fprintf(stderr, "Process encountered an error. ERROR%d", errno);
                return EXIT_FAILURE;
            }
        }
        else
        {
            printf("[%d] is running\n", pid);

            add_to_list(pid, cmd.execArgs[0]);
        }
    }
    return(0);
}

void set(command_t cmd)
{
    char * temp;
    temp = strtok(cmd.execArgs[1],"=");

    if( !strcmp(temp, "PATH") )
    {
        strcpy(PATH, strtok(NULL, " "));
        printf("PATH set to %s\n", PATH);
    }
    else if( !strcmp(temp, "HOME") )
    {
        strcpy(HOME, strtok(NULL, " "));
        printf("HOME set to %s\n", HOME);
    }
    else
    {
        printf("Cannot set %s\n", temp);
    }	
}

void echo(command_t cmd)
{
    if( !strcmp(cmd.execArgs[1], "$PATH") )
    {
        printf("%s\n", PATH);
    }
    else if( !strcmp(cmd.execArgs[1], "$HOME") )
    {
        printf("%s\n", HOME);
    }
    else if( !strcmp(cmd.execArgs[1], "$WKDIR") )
    {
        printf("%s\n", WKDIR);
    }
    else
    {
        printf("Cannot echo %s\n", cmd.execArgs[1]);
    }
}

bool is_running() {
    return running;
}

void terminate() {
    running = false;
}

bool get_command(command_t* cmd, FILE* in) 
{
    if (isatty(fileno(stdin)))
        printf( "     meh:~%s$ ", WKDIR );
    
    sigsetjmp(env,1);

    if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) 
    {
        cmd->execBg = false;
        size_t len = strlen(cmd->cmdstr);
        char last_char = cmd->cmdstr[len - 1];

        if (last_char == '\n' || last_char == '\r') {
            // Remove trailing new line character.
            cmd->cmdstr[len - 1] = '\0';
            cmd->cmdlen = len - 1;
            len = len - 1;
            last_char = cmd->cmdstr[len-1];
        }
        else
            cmd->cmdlen = len;

        if (last_char == '&')
        {
            cmd->cmdstr[len - 1] = '\0';
            cmd->cmdlen = len - 1;
            cmd->execBg = true;
        }
        char tempcmd[MAX_COMMAND_LENGTH];
        strcpy(tempcmd,cmd->cmdstr);
        char * temp;
        temp = strtok(tempcmd," ");
        cmd->execArgs[0] = temp;

        int i = 1;
        while(((temp = strtok(NULL," ")) != NULL) && (i<255))
        {
            cmd->execArgs[i] = temp;
            i++;
        }
        cmd->execArgs [i] = NULL;
        //printf("i: %d",i);
        cmd->execNumArgs = i;
        return true;
    }
    else
        return false;
}

/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */
int main(int argc, char** argv) { 
    command_t cmd; //< Command holder argument
      
    start();
    
    struct sigaction sa;
    sigset_t mask_set;
    sigfillset(&mask_set);
    sigdelset(&mask_set,SIGINT);
    sigdelset(&mask_set,SIGTSTP);
    sa.sa_handler = catchChild;
    sigaction(SIGCHLD, &sa,NULL);//child termination calls catchChild;

    strcpy( PATH, getenv("PATH") );
    strcpy( HOME, getenv("HOME") );
    strcpy( WKDIR, HOME );

    puts("hOi! Welcome to Quash!");

    // Main execution loop
    while (is_running() && get_command(&cmd, stdin)) 
    {
        // NOTE: I would not recommend keeping anything inside the body of
        // this while loop. It is just an example.

        // The commands should be parsed, then executed.
        if (!strcmp(cmd.cmdstr, "q")||!strcmp(cmd.cmdstr, "exit")||!strcmp(cmd.cmdstr, "quit"))
            terminate(); // Exit Quash
        else if(!strcmp(cmd.execArgs[0], "set"))
            set(cmd);//set globa variables
        else if(!strcmp(cmd.execArgs[0], "echo"))
            echo(cmd);
        else if(!strcmp(cmd.execArgs[0], "pwd"))
            pwd(cmd);
        else if(!strcmp(cmd.execArgs[0], "cd"))
            cd(cmd);
        else if(!strcmp(cmd.execArgs[0], "jobs"))
            jobs();
        else 
            exec_cmd(cmd);
        //puts(cmd.cmdstr); // Echo the input string
    }

    return EXIT_SUCCESS;
}

