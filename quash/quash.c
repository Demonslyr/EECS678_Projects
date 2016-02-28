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
//sigjmp_buf env;
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
    printf("Child died!%d\n",signum);
    pid_t pid;
    int status;
    while((pid = waitpid(-1,&status,WNOHANG)) != -1)
    {
        printf("I got in the while!\n");
        //unregister_child(pid,status);
    }
    printf("PID: %1d\n",pid);
    //siglongjmp(env,1);
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

void jobs()
{
    printf("Not Yet Implimented.\n");
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
    printf(pathExists(testPath)? "true\n" : "false\n");

    char tempPath[MAX_PATH_LENGTH];
    char * temptok;

    strcpy(testReturn, testPath);


    if(testPath[0] == '/')
    {
        printf("it was an absolute path\n");
        return;
    }

    strcpy( tempPath, WKDIR );
    strcat( tempPath, "/" );
    strcat( tempPath, testPath );
    printf(" tempPath: %s\n", tempPath );

    if( pathExists( tempPath ) )
    {
        strcpy( testReturn, tempPath );
        return;
    }

    else
    {
        printf("I'm here\n");
        printf("PATH: %s\n",PATH);
        if (PATH[0] == '\0')
        {
            printf("PATH is NULL\n");
            return;
        }
        temptok = strtok(PATH,":");

        while(temptok!=NULL)
        {
            strcpy(tempPath,temptok);
            strcat(tempPath,"/");
            strcat(tempPath, testPath);

            printf("Checking %s\n",tempPath);
            if(pathExists(tempPath))
            {
                strcpy(testReturn, tempPath);
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

    strcpy(cmd.execArgs[0], test);
    //sigsetjmp(env,1);
	int pid = fork();
	if(!pid)
	{
    	fprintf(stderr, "Ename = %s, str = %s\n",cmd.execArgs[0], cmd.execArgs[1]);
		if(execv(cmd.execArgs[0],cmd.execArgs)<0)
		{
			fprintf(stderr, "Error execing %s. Error# %d\n",cmd.cmdstr, errno);
			exit(EXIT_FAILURE);
		}
	}

	printf("[%d] is running\n", pid);
    
    if((waitpid(pid,&status,0))==-1)
    {
        fprintf(stderr, "Process encountered an error. ERROR%d", errno);
        return EXIT_FAILURE;
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
    printf( "     meh:~%s$ ", WKDIR );

    if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) 
    {
        size_t len = strlen(cmd->cmdstr);
        char last_char = cmd->cmdstr[len - 1];

        cmd->execBg = false;

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

        char * temp;
        temp = strtok(cmd->cmdstr," ");
        cmd->execArgs[0] = temp;

        int i = 1;
        while((temp != NULL) && (i<255))
        {
            cmd->execArgs[i] = strtok(NULL," ");
            i++;
        }
        cmd->execArgs [i+1] = "\0";

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
	
  //signal response;
  struct sigaction sa;
  sigset_t mask_set;
  sigfillset(&mask_set);
  sigdelset(&mask_set,SIGINT);
  sigdelset(&mask_set,SIGTSTP);
  sa.sa_handler = catchChild;
  sigaction(SIGCHLD, &sa,NULL);//child termination calls catchChild;
  
  //PATH[0] = '\0';
  //HOME[0] = '\0';
  strcpy( PATH, getenv("PATH") );
  strcpy( HOME, getenv("HOME") );
  strcpy( WKDIR, HOME );

  puts("hOi! Welcome to Quash!");

	// Main execution loop
	while (is_running() && get_command(&cmd, stdin)) {
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

