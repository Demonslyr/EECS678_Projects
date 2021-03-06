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
    delete_from_list(waitpid(-1,&status,WNOHANG));
    
    while((pid = waitpid(-1,&status,WNOHANG|WUNTRACED)) != -1)
    {
        delete_from_list(pid);
        return;

        // if (WIFEXITED(status))
        //     printf("Child ended normally.n");
        // else if (WIFSIGNALED(status))
        //     printf("Child ended because of an uncaught signal.n");
        // else if (WIFSTOPPED(status))
        //     printf("Child process has stopped.n");
    }
    //siglongjmp( env, 1 );   
}

void pwd()
{
    printf("%s\n",getenv("WKDIR"));
    return;
}

void cd(command_t cmd)
{
    char WKDIR[MAX_PATH_LENGTH];

    if (cmd.execArgs[1] == NULL)
    {
        if (getenv("WKDIR") == NULL)
        {
            printf("HOME was NULL. No change made to the Working directory.");
            return;
        }

        setenv("WKDIR",getenv("HOME"),1);
    }
    else
    {
        strcpy( WKDIR, getenv("WKDIR"));
        strcat( WKDIR, "/");
        strcat( WKDIR, cmd.execArgs[1] );
        setenv("WKDIR", WKDIR, 1);
    }
    return;
}

void checkWkdir(char* command)
{
    char wkDir[MAX_PATH_LENGTH]; 
    char temp[MAX_PATH_LENGTH];
    strcpy(wkDir, getenv("WKDIR"));
    strcat(wkDir, "/");
    strcat(wkDir, command);
    FILE * file = fopen(wkDir,"r");
    // printf("pre cmd: %s\n",command);
    // printf("pre out: %s\n",output);
    //output = command;
    if (file!=NULL)
    {
        fclose(file);
        strcpy(command, wkDir);
        //output = wkDir;
    }
    // printf("post cmd: %s\n",command);
    // printf("post out: %s\n",output);
    
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
    return;
}

int exec_pipes(command_t cmd)
{
    sigset_t tmpSa;
    sigfillset( &tmpSa);
    sigdelset(&tmpSa,SIGINT);
    sigdelset(&tmpSa,SIGTSTP);
    sigprocmask(SIG_SETMASK, &tmpSa, NULL);
    //Start blocking signals

    int ppid;
    ppid = fork();
    if(!ppid)
    {    
        char tmpcmdstr [MAX_COMMAND_LENGTH];
                
        command_t cmd_a[MAX_COMMAND_LENGTH];

        int numCommands = pipeParse(cmd, cmd_a);
        strcpy(tmpcmdstr, cmd.cmdstr);

        int status;
        pid_t pid_a[numCommands];
        int fd_a[numCommands*2];
        for (int i =0;i<numCommands;i++)
        {
            pipe(fd_a+(i*2));
        }

        for(int i = 0;i<numCommands;i++)
        {
            pid_a[i] = fork();
            
            if(pid_a[i] == 0)
            {
                if(i == 0)
                {
                    dup2(fd_a[1],1);//out
                }
                else if(i == numCommands-1)
                {
                    dup2(fd_a[(i-1)*2],0);//in
                    dup2(STDOUT_FILENO,1);//out
                }
                else
                {
                    dup2(fd_a[(i-1)*2],0);//in
                    dup2(fd_a[(i*2)+1],1);//out
                }

                for(int j = 0; j<(2*numCommands);j++)
                {
                    close(fd_a[j]);
                }
                if(strcmp(cmd.outputFile,"")&&(i==numCommands-1))
                {
                    execToFile(cmd_a[i].execArgs, cmd.outputFile);
                }
                else
                {
                    checkWkdir(cmd.execArgs[0]);
                    if((execvp(cmd_a[i].execArgs[0],cmd_a[i].execArgs)) < 0)
                    {
                        fprintf(stderr,"\nError execing %s. ERROR# %d",cmd_a[i].execArgs[0],errno);
                    }
                }
                
                exit(0);
            }
        }

        for(int j = 0; j<(2*numCommands);j++)
        {
            close(fd_a[j]);
        }

        for(int i =0; i<numCommands;i++)
        {   
            if((waitpid(pid_a[i],&status,0)) == -1)
            {
                fprintf(stderr, "%s encountered an error._1 ERROR %d",cmd_a[i].execArgs[0], errno);
                exit(EXIT_FAILURE);
            }
        }

        exit(0);
    }
    else
    {
        if(cmd.execBg)
        {
            sigfillset( &tmpSa);
            sigdelset(&tmpSa,SIGINT);
            sigdelset(&tmpSa,SIGTSTP);
            sigdelset(&tmpSa, SIGCHLD);
            sigprocmask(SIG_SETMASK, &tmpSa, NULL);
            //Stop blocking signals       
                 
            add_to_list(ppid, cmd.cmdstr);
        }
        else if((waitpid(ppid,&status,0)) == -1)
        {
            fprintf(stderr, "%s encountered an error._2 ERROR %d",cmd.cmdstr, errno);
            sigfillset( &tmpSa);
            sigdelset(&tmpSa,SIGINT);
            sigdelset(&tmpSa,SIGTSTP);
            sigdelset(&tmpSa, SIGCHLD);
            sigprocmask(SIG_SETMASK, &tmpSa, NULL);
            //Stop blocking signals because we can return if we fail
            //one of the commands and we don't want signals to stay blocked

            return EXIT_FAILURE;
        }


    }
    
    sigfillset( &tmpSa);
    sigdelset(&tmpSa,SIGINT);
    sigdelset(&tmpSa,SIGTSTP);
    sigdelset(&tmpSa, SIGCHLD);
    sigprocmask(SIG_SETMASK, &tmpSa, NULL);
    //Stop blocking signals

    return 0;
}

int exec_cmd(command_t cmd)
{

    pid_t pid = fork();
	if(!pid)
    {
        if( strcmp(cmd.inputFile,"") )
        {          
            char string[MAX_COMMAND_LENGTH];
            FILE * file = fopen(cmd.inputFile, "r");

            while(fgets(string, MAX_COMMAND_LENGTH, file) != NULL)
            {
                char* args[MAX_PATH_LENGTH];

                size_t len = strlen(string);
                char last_char = string[len - 1];

                if (last_char == '\n' || last_char == '\r')
                {
                    string[len - 1] = '\0';
                    len = len - 1;
                }

                args[0] = cmd.execArgs[0];

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
                    if(strcmp(cmd.outputFile,""))
                    {
                        execToFile(args, cmd.outputFile);
                    }
                    else
                    {
                        checkWkdir(cmd.execArgs[0]);
                        if(execvp(cmd.execArgs[0],args)<0)
                        {
                            fprintf(stderr, "Error execing %s. Error# %d\n",cmd.cmdstr, errno);
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                else
                {
                    if((waitpid(pid2,&status,0))==-1)
                    {
                        fprintf(stderr, "Process encountered an error._3 ERROR%d", errno);
                        return EXIT_FAILURE;
                    }
                }
                
            }
            
        }
        else if (strcmp(cmd.outputFile,""))
        {
            execToFile(cmd.execArgs, cmd.outputFile);
        }
        else
        {
            // char * tmpstr[MAX_PATH_LENGTH];
            // strcpy(tmpstr,cmd.execArgs[0]);
            // printf("arg0: %s\n",cmd.execArgs[0]);
            // printf("tmpstr: %s\n",tmpstr);

            //if(execvp(tmpstr,cmd.execArgs)<0)
            checkWkdir(cmd.execArgs[0]);
            if(execvp(cmd.execArgs[0],cmd.execArgs)<0)
            {
                fprintf(stderr, "Error execing %s. Error# %d\n",cmd.cmdstr, errno);
                exit(EXIT_FAILURE);
            }
        }
        exit(0);
	}
    else
    {

        sigset_t tmpSa;
        sigfillset( &tmpSa);
        sigdelset(&tmpSa,SIGINT);
        sigdelset(&tmpSa,SIGTSTP);
        sigprocmask(SIG_SETMASK, &tmpSa, NULL);
        //Start blocking signals

        if(!cmd.execBg)
        {
            //add pid and command w/o path to jobs structure
            if((waitpid(pid,&status,0))==-1)
            {
                fprintf(stderr, "Process encountered an error._4 ERROR%d", errno);
                sigfillset( &tmpSa);
                sigdelset(&tmpSa,SIGINT);
                sigdelset(&tmpSa,SIGTSTP);
                sigdelset(&tmpSa, SIGCHLD);
                sigprocmask(SIG_SETMASK, &tmpSa, NULL);
                //Stop blocking                
                return EXIT_FAILURE;
            }
            sigfillset( &tmpSa);
            sigdelset(&tmpSa,SIGINT);
            sigdelset(&tmpSa,SIGTSTP);
            sigdelset(&tmpSa, SIGCHLD);
            sigprocmask(SIG_SETMASK, &tmpSa, NULL);
            //Stop blocking            
        }
        else
        {
            sigfillset( &tmpSa);
            sigdelset(&tmpSa,SIGINT);
            sigdelset(&tmpSa,SIGTSTP);
            sigdelset(&tmpSa, SIGCHLD);
            sigprocmask(SIG_SETMASK, &tmpSa, NULL);
            //Stop blocking???
            printf("[%d] is running\n", pid);

            add_to_list(pid, cmd.execArgs[0]);
        }
    }
    return(0);
}

void execToFile(char ** execArgs, char * outputFile)
{
    //cmd.execArgs[cmd.execNumArgs-2] = NULL;
    //call exec as normal
    int file = open(outputFile,O_CREAT|O_APPEND|O_WRONLY,S_IRWXU);
    dup2(file, 1);
    checkWkdir(execArgs[0]);
    if(execvp(execArgs[0],execArgs)<0)
    {
        fprintf(stderr, "Error execing %s. Error# %d\n",execArgs[0], errno);
        exit(EXIT_FAILURE);
    }
    //puting things back the way they were.
    close(file);
}

void set(command_t cmd)
{
    char * temp;
    temp = strtok(cmd.execArgs[1],"=");

    if( !strcmp(temp, "PATH") )
    {
        if(setenv("PATH", strtok(NULL, " "), 1) == 0)
            printf("PATH set to %s\n", getenv("PATH"));
    }
    else if( !strcmp(temp, "HOME") )
    {
        if(setenv("HOME", strtok(NULL, " "), 1) == 0)
            printf("HOME set to %s\n", getenv("HOME"));
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
        printf("%s\n", getenv("PATH"));
    }
    else if( !strcmp(cmd.execArgs[1], "$HOME") )
    {
        printf("%s\n", getenv("HOME"));
    }
    else if( !strcmp(cmd.execArgs[1], "$WKDIR") )
    {
        printf("%s\n", getenv("WKDIR"));
    }
    else
    {
        printf("%s\n", cmd.cmdstr+5);
        //echo everything after the word echo
        //& at the end of the string and < or > must be enclosed in parenthesis
    }
}

bool is_running() 
{
    return running;
}

void terminate() 
{
    running = false;
}

bool get_command(command_t* cmd, FILE* in) 
{
    if (isatty(fileno(in)))
        printf( "meh:~%s$ ", getenv("WKDIR") );

    //sigsetjmp( env, 1 );

    if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) 
    {
        char tmpArgs [MAX_COMMAND_LENGTH][MAX_PATH_LENGTH];

        cmd->execBg = false;
        strcpy(cmd->inputFile,"");
        strcpy(cmd->outputFile,"");

        size_t len = strlen(cmd->cmdstr);
        char last_char = cmd->cmdstr[len - 1];

        if (last_char == '\n' || last_char == '\r') 
        {
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

        while ( cmd->cmdstr[0] == ' ' )
        {
            char temp[MAX_COMMAND_LENGTH];

            strcpy(temp, cmd->cmdstr+1);
            strcpy(cmd->cmdstr, temp);
        }

        if ( !strcmp(cmd->cmdstr, "") )
        {
            return false;
        }
        
        int i = 0;

        char tempcmd[MAX_COMMAND_LENGTH];
        strcpy(tempcmd, cmd->cmdstr);
        char * temp;
        temp = strtok(tempcmd," ");
        strcpy(tmpArgs[i], temp);
        cmd->execArgs[i] = tmpArgs[i];

        i++;

        while(((temp = strtok(NULL," ")) != NULL) && (i<255))
        {
            if( !strcmp(temp, "<") || !strcmp(temp, ">") )
            {
                if( !strcmp(temp, "<") )
                {
                    temp = strtok(NULL, " ");
                    strcpy( cmd->inputFile, temp );
                    temp = strtok(NULL, " ");
                }

                if( temp != NULL && !strcmp(temp, ">") )
                {
                    temp = strtok(NULL, " ");
                    strcpy( cmd->outputFile, temp );
                    temp = strtok(NULL, " ");
                }

                if( temp != NULL )
                {
                    printf("Error: incorrect command format\n");
                    return false;
                }

                cmd->execArgs[i] = NULL;
                cmd->execNumArgs = i;

                return true;
            }
            
            strcpy( tmpArgs[i], temp );
            
            cmd->execArgs[i] = tmpArgs[i];
            i++;
        }

        cmd->execArgs [i] = NULL;
        cmd->execNumArgs = i;

        return true;
    }
    else
        return false;
}

int pipeParse(command_t cmd, command_t * cmd_a)
{
    char tmpcmdstr [MAX_COMMAND_LENGTH];

    strcpy(tmpcmdstr, cmd.cmdstr);

    char * temp;
    char argStore[MAX_COMMAND_LENGTH][MAX_PATH_LENGTH];
    int argIter = 0;

    int numCommands = 0;
    temp = strtok(tmpcmdstr,"|");
    strcpy(cmd_a[numCommands].cmdstr, temp);

    numCommands++;
    while((temp = strtok(NULL,"|")) != NULL)
    {
        strcpy(cmd_a[numCommands].cmdstr, temp);
        numCommands++;
    }
    
    for(int i = 0; i < numCommands; i++)
    {
        cmd_a[i].execBg = false;
        size_t len = strlen(cmd_a[i].cmdstr);
        char last_char = cmd_a[i].cmdstr[len - 1];

        if (last_char == '\n' || last_char == '\r') 
        {
            // Remove trailing new line character.
            cmd_a[i].cmdstr[len - 1] = '\0';
            cmd_a[i].cmdlen = len - 1;
            len = len - 1;
            last_char = cmd_a[i].cmdstr[len-1];
        }
        else
            cmd_a[i].cmdlen = len;

        if (last_char == '&')
        {
            cmd_a[i].cmdstr[len - 1] = '\0';
            cmd_a[i].cmdlen = len - 1;
            cmd_a[i].execBg = true;
        }

        while ( cmd_a[i].cmdstr[0] == ' ' )
        {
            char temp[MAX_COMMAND_LENGTH];

            strcpy(temp, cmd_a[i].cmdstr+1);
            strcpy(cmd_a[i].cmdstr, temp);
        }

        char tempcmd[MAX_COMMAND_LENGTH];
        
        char * arg;
        
        strcpy(tempcmd, cmd_a[i].cmdstr);
        arg = strtok(tempcmd," ");
        
        int numArgs = 0;
        strcpy(argStore[argIter], arg);
        cmd_a[i].execArgs[numArgs] = argStore[argIter];
        argIter++;
        numArgs++;

        while((arg = strtok(NULL," ")) != NULL)
        {
            if( !strcmp(arg, "<") || !strcmp(arg, ">") )
            {
                break;
            }

            strcpy(argStore[argIter], arg);
            cmd_a[i].execArgs[numArgs] = argStore[argIter];
            argIter++;
            numArgs++;
        }

        cmd_a[i].execArgs[numArgs] = NULL;
        cmd_a[i].execNumArgs = numArgs;
    }

    return numCommands;
}

bool killChild(command_t cmd)
{
    int job_id = atoi(cmd.execArgs[2]);
    int pid = search_by_job_id( job_id );
    //printf("\npid = %d",pid);
    if(pid == 0)
    {
        printf( "Job ID %d not found in current jobs\n", job_id );
        return false;
    }
    else
    {
        sigset_t tmpSa;
        sigfillset( &tmpSa);
        sigdelset(&tmpSa,SIGINT);
        sigdelset(&tmpSa,SIGTSTP);
        sigdelset(&tmpSa, SIGCHLD);
        sigprocmask(SIG_SETMASK, &tmpSa, NULL);
        //Stop blocking 
        //printf("\nexcArg1 = %s",cmd.execArgs[1]);
        kill(pid, atoi(cmd.execArgs[1]) );
        return true;
    }
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
    struct sigaction NULL_sa;
    struct sigaction sa;
    sigset_t mask_set;
    sigfillset(&mask_set);
    sigdelset(&mask_set,SIGINT);
    sigdelset(&mask_set,SIGTSTP);
    sa.sa_handler = catchChild;
    sigprocmask(SIG_SETMASK, &mask_set, NULL);
    //TODO: this is involved withe the error 10 problem. Removing it remedies the issue for now but breaks other things.
    sigaction(SIGCHLD, &sa,NULL);//child termination calls catchChild;

    setenv( "WKDIR", getenv("HOME"), 1 );

    puts("hOi! Welcome to Quash!");

    // Main execution loop
    while (is_running()) 
    {
        // NOTE: I would not recommend keeping anything inside the body of
        // this while loop. It is just an example.

        // The commands should be parsed, then executed.
        if( !get_command(&cmd, stdin) );
        else if (!strcmp(cmd.cmdstr, "q")||!strcmp(cmd.cmdstr, "exit")||!strcmp(cmd.cmdstr, "quit"))
            terminate(); // Exit Quash
        else if(!strcmp(cmd.execArgs[0], "set"))
            set(cmd);//set environment variables
        else if(!strcmp(cmd.execArgs[0], "echo"))
            echo(cmd);//echos environment variables
        else if(!strcmp(cmd.execArgs[0], "pwd"))
            pwd(cmd);//prints current working directory
        else if(!strcmp(cmd.execArgs[0], "cd"))
            cd(cmd);//changes the working directory
        else if(!strcmp(cmd.execArgs[0], "jobs"))
            jobs();//prints out a list of currently running jobs
        else if(!strcmp(cmd.execArgs[0], "kill"))
            killChild(cmd);//kills specified job
        else if (!strcmp(cmd.execArgs[0], "wait"))
            sleep(atoi(cmd.execArgs[1]));
        else if (strchr(cmd.cmdstr,'|')!= NULL)
            exec_pipes(cmd);//executes piped commands
        else 
            exec_cmd(cmd);//executes normal commands
    }

    return EXIT_SUCCESS;
}

