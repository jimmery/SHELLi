// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

/* FIXME: You may need to add #include directives, macro definitions,
static function definitions, etc.  */


// TODO. hey. i have no idea why this matters right now. pls help. 
int
command_status(command_t c)
{
	return c->status;
}

void
execute(command_t c)
{
	switch (c->type) {
	case SIMPLE_COMMAND:
	{
		// setup redirections, if necessary. 
		if (c->input != NULL)
		{
			int fin = open(c->input, O_CREAT | O_TRUNC | O_WRONLY, 0644);
			if (fin < 0)
				// somehow return an error. 
				error(1, 1, "File not found");
			if (dup2(fin, 0) < 0) // closes stdin and duplicates fin into point to where stdin was. 
				error(1, 1, "Error making input stream");
		}
		if (c->output != NULL)
		{
			int fout = open(c->output, O_CREAT | O_TRUNC | O_WRONLY, 0644);
			if (fout < 0)
				error(1, 1, "File not found");
			if (dup2(fout, 1) < 0) // closes stdout and duplicates fout point to where stdout was. 
				error(1, 1, "Error making output stream");
		}

		// actually run the command. 
		if (strcmp(c->u.word[0], "exec") == 0)
			execvp(c->u.word[1], &c->u.word[1]);
		else
			execvp(c->u.word[0], &c->u.word[0]);
		break;
	}
	case AND_COMMAND:
	{
		int p = fork();
		if (p == 0) // child. left hand command first. 
		{
			execute(c->u.command[0]);
		}
		else
		{
			int status;  // used as an argument for waitpid. 
			waitpid(p, &status, 0); // 0 means blocking wait. 
			int exitStatus = WEXIT_STATUS(status); // extracts exit status 

			// the first command returned successfully.
			// thus, we should run the next command.
			// otherwise we should not run anything. 
			if (exitStatus == 0)
			{
				execute(c->u.command[1]);
			}
		}
		break;
	}
	case OR_COMMAND:
	{
		int p = fork();
		if (p == 0) // child. left hand command first. 
		{
			execute(c->u.command[0]);
		}
		else
		{
			int status;  // used as an argument for waitpid. 
			waitpid(p, &status, 0); // 0 means blocking wait. 
			int exitStatus = WEXITSTATUS(status); // extracts exit status 

			// the first command returned unsuccessfully.
			// thus, we should run the next command.
			// if the first command is successful, the second is unneccessary. 
			if (exitStatus != 0)
			{
				execute(c->u.command[1]);
			}
		}
		break;
	}
	case SEQUENCE_COMMAND:
	{
		int p = fork();
		if (p == 0) // child. left hand command first. 
		{
			execute(c->u.command[0]);
		}
		else
		{
			int status;  // used as an argument for waitpid. 
			waitpid(p, &status, 0); // 0 means blocking wait. 
			//int exitStatus = WEXIT_STATUS(status); // extracts exit status 

			// runs the second command after the first one is complete. 
			execute(c->u.command[1]);
		}
		break;
	}
	case PIPE_COMMAND:

		// TODO I THINK THIS ONE IS REALLY MESSED UP. JUST COPYING FROM DISC NOTES FOR NOW. 
	{
		int fd[2];
		pipe(fd);
		int firstPid = fork();
		if (firstPid == 0)
		{
			close(fd[1]); // close the unused write end. 
			dup2(fd[0], 0); // setting up a read pipe.
			execute(c->u.command[1]);
		}
		else
		{
			int secondPid = fork();
			if (secondPid == 0)
			{
				close(fd[0]); // close unused read end. 
				dup2(fd[1], 1);
				execute(c->u.command[0]);
			}
			else
			{
				// is this else statement even necessary? 
				close(fd[0]);
				close(fd[1]);
				int status;
				waitpid(-1, &status, 0); // waits for one child process to finish.
				waitpid(-1, &status, 0); // waits for the other child process to finish. 
			}
		}
		break;
	}
	case SUBSHELL_COMMAND:
	{
		command_t innerCmd = c->u.subshell_command;
		if (c->input != NULL)
		{
			innerCmd->input = c->input;
		}
		if (c->output != NULL)
		{
			innerCmd->output = c->output;
		}
		execute(innerCmd);
		break;
	}
	default:
		// TODO: idk lol. 
	}
}

void
execute_command(command_t c, bool time_travel)
{
	/* FIXME: Replace this with your implementation.  You may need to
	add auxiliary functions and otherwise modify the source code.
	You can also use external functions defined in the GNU C Library.  */
	//error (1, 0, "command execution not yet implemented");

	int p = fork();
	if (p == 0) // child
	{
		execute(c);
	}
	else
	{
		// TODO figure out what happens after all the commands have been executed. 
	}
}
