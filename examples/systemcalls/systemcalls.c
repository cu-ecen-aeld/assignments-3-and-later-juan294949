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

/* Note From assignment3-part1 implement the TODO there related to video content and system() and exec() functions.  
  See provided test code in */
	/*
		pid_t wait(int *_Nullable wstatus);
  	pid_t waitpid(pid_t pid, int *_Nullable wstatus, int options);
	
			wait() and waitpid()
       The wait() system call suspends execution of the calling thread
       until one of its children terminates.  The call wait(&wstatus) is
       equivalent to:

        waitpid(-1, &wstatus, 0);

       The waitpid() system call suspends execution of the calling thread
       until a child specified by pid argument has changed state.  By
       default, waitpid() waits only for terminated children, but this
       behavior is modifiable via the options argument, as described
       below.

       The value of pid can be:

       < -1   meaning wait for any child process whose process group ID
              is equal to the absolute value of pid.

       -1     meaning wait for any child process.

       0      meaning wait for any child process whose process group ID
              is equal to that of the calling process at the time of the
              call to waitpid().

       > 0    meaning wait for the child whose process ID is equal to the
              value of pid.
	*/

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
  // this line is to avoid a compile warning before your implementation is complete
  // and may be removed
  command[count] = command[count];
	
/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
// Source - https://stackoverflow.com/a
// Posted by tmyklebu, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-06, License - CC BY-SA 3.0
  int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0666); // write the command to the file openned here.
// REDIRECT_FILE, 3, "/bin/sh", "-c", "echo home is $HOME"
	int  stdout_ = dup(STDOUT_FILENO);
if (fd >= 0)
{
	dup2(fd,STDOUT_FILENO); // Standart output redirects to the openned file testfile.txt.
	close(fd); // close the file descriptor.
	switch (count)
	{
		case 2:
		{
			status = do_exec(count,*command,*(command +1));
			break;
		}
		case 3:
		{
			status = do_exec(count,*command,*(command +1),*(command+2));
			break;
		}
		default:
		{
			break;
		}
	}
} else{ perror("\n\ropen");}

	dup2(stdout_,STDOUT_FILENO);
	close(stdout_);
  va_end(args);
	
  return status;
}
