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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
//#include <strings.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

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
void exec_cmd(command_t cmd)
{
	int pid = 0;//fork();
	if(!pid)
	{


		int slen = strcspn(cmd.cmdstr," ");
		char path[slen];
		strncpy(path, cmd.cmdstr, slen);
		printf("\n%s\n",path);
		/*if((execl(,cmd.cmdstr,(char *)0))<0)
		{
			fprint(stderr, "\nError execing %s. Error# %d\n",cmd.cmdstr, errno);
			return EXIT_FAILURE;
		}*/
		exit(0);
	}
	else
	{
		printf("[%d] is running\n", pid);
		if((waitpid(pid,&status,0))==-1)
		{
			fprintf(stderr, "Process encountered an error. ERROR%d", errno);
			//return EXIT_FAILURE;
		}
	}
}

bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

bool get_command(command_t* cmd, FILE* in) {
  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];

    if (last_char == '\n' || last_char == '\r') {
      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;
    }
    else
      cmd->cmdlen = len;
    
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
  
  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.

    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit")||!strcmp(cmd.cmdstr, "quit"))

      terminate(); // Exit Quash
    else 
      exec_cmd(cmd);
      //puts(cmd.cmdstr); // Echo the input string
  }

  return EXIT_SUCCESS;
}
