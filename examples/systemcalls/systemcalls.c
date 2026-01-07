#include "systemcalls.h"
#include "syslog.h"
#include "errno.h"
#include "string.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>   
#include <unistd.h> 
#include <stdlib.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd) // const char *cmd = "some command" 
{
	bool status = false;

	if (cmd) // Check if the cmd is not a null and stores an address.
	{ 
		int Temp = system(cmd);
		switch(Temp)
		{ 
			case(0):
			{
				/*If command is NULL, then a nonzero value if a shell is
          available, or 0 if no shell is available.*/
				status = true;
				break;
			}
			case(-1):
			{
				/*If a child process could not be created, or its status could
          not be retrieved, the return value is -1 and errno is set to
          indicate the error.
				*/
				printf("From systemcalls.c: %s\r\n", strerror(errno));
				break;
			}
			case(127):
			{
				/*If a shell could not be executed in the child process, then the
        return value is as though the child shell terminated by calling
        _exit(2) with the status 127.*/
				break;
			}
			default:
			{
				printf("From systemcalls.c: returned with status code %d\r\n",Temp);
				break;
			}
		}
	}
	
  return status;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*
*	TODO:
*  Execute a system command by calling fork, execv(),
*  and wait instead of system (see LSP page 161).
*  Use the command[0] as the full path to the command to execute
*  (first argument to execv), and use the remaining arguments
*  as second argument to the execv() command.
*/

bool do_exec(int count, ...)
{
  va_list args;
  va_start(args, count);
  char * command[count+1];
  int i;
  for(i=0; i<count; i++)
  {
    command[i] = va_arg(args, char *);
  }
  command[count] = NULL;
  command[count] = command[count];
	int wstatus;
	bool status = false;

	fflush(stdout);
	pid_t pid = fork();

  if (pid < 0)
  {
    printf("fork failed: %s\r\n", strerror(errno));
  }
  else if (pid == 0)
  {
    // Child, On error: execv() returns -1
		if(execv(command[0], command) != 0 )
		{
			_exit(EXIT_FAILURE);
		}
  }
  else
  {
		waitpid(pid, &wstatus, 0);

		if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0)
		{
		  status = true;
		}
  }
  va_end(args);
  return status;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
	bool status = false;
  va_list args;
  va_start(args, count);
  char * command[count+1];
  int i;
  for(i=0; i<count; i++)
  {
    command[i] = va_arg(args, char *);
  }
  command[count] = NULL;
  command[count] = command[count];

   int pid = fork();

  	if (pid == 0) {

    int fd = open(outputfile, O_TRUNC | O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
      _exit(1);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    execv(command[0], command);
    perror("Error:");
    _exit(EXIT_FAILURE);
  } 
	else if (pid < 0)
	{
    perror("Error:");
    va_end(args);
  } 
	else 
	{
    waitpid(pid, NULL, 0);
    status = true;
  }
  va_end(args);

  return status;
}
