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

// returns the exit status. 
void
execute(command_t c)
{
	switch (c->type) 
	{
	case SIMPLE_COMMAND:
	{
		// setup redirection. 
		if (c->input != NULL)
		{
			int fin = open(c->input, O_RDONLY, 0644);
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
			int exitStatus = WEXITSTATUS(status); // extracts exit status 
			c->u.command[0]->status = exitStatus;

			// the first command returned successfully.
			// thus, we should run the next command.
			// otherwise we should not run anything. 
			if (exitStatus == 0)
			{
				int p2 = fork();
				if (p2 == 0)
				{
					execute(c->u.command[1]);
				}
				else // TODO is this necessary? currently the need for this is the exit status. 
				{
					waitpid(p2, &status, 0); // 0 means blocking wait. 
					exitStatus = WEXITSTATUS(status); // extracts exit status 
					c->u.command[1]->status = exitStatus;

					c->status = exitStatus; // changed here, even though will be changed again.
					exit(exitStatus);
				}	
			}
			else
			{
				c->status = exitStatus; // changed here, even though may be changed again.
				exit(exitStatus);
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
			c->u.command[0]->status = exitStatus;
			//int exitStatus = c->u.command[0]->status;

			// the first command returned unsuccessfully.
			// thus, we should run the next command.
			// if the first command is successful, the second is unneccessary. 
			if (exitStatus != 0)
			{
				int p2 = fork();
				if (p2 == 0)
				{
					execute(c->u.command[1]);
				}
				else // TODO is this necessary? currently the need for this is the exit status. 
				{
					waitpid(p2, &status, 0); // 0 means blocking wait. 
					exitStatus = WEXITSTATUS(status); // extracts exit status 
					c->u.command[1]->status = exitStatus;

					c->status = exitStatus; // changed here, even though will be changed again.
					exit(exitStatus);
				}
			}
			else
			{
				c->status = 0; // the || statement ran successfully. 
				exit(0);
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
			int exitStatus = WEXITSTATUS(status); // extracts exit status 
			c->u.command[0]->status = exitStatus;

			// runs the second command after the first one is complete. 
			int p2 = fork();
			if (p2 == 0)
			{
				execute(c->u.command[1]);
			}
			else // TODO is this necessary? currently the need for this is the exit status. 
			{
				waitpid(p2, &status, 0); // 0 means blocking wait. 
				exitStatus = WEXITSTATUS(status); // extracts exit status 
				c->u.command[1]->status = exitStatus;

				c->status = exitStatus; // changed here, even though will be changed again.
				exit(exitStatus);
			}
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
				close(fd[0]);
				close(fd[1]);
				int status;
				waitpid(-1, &status, 0); // waits for firstPid to finish.
				int exitStatus = WEXITSTATUS(status);
				c->u.command[0]->status = exitStatus; // not actually sure this is right.
				waitpid(-1, &status, 0); // waits for the other child process to finish. 
				exitStatus = WEXITSTATUS(status);
				c->u.command[1]->status = exitStatus; // not actually sure this is right. 
				c->status = exitStatus; // may be changed later, but should be fine.
				exit(exitStatus); // TODO change?
			}
		}
		break;
	}
	case SUBSHELL_COMMAND:
	{
		command_t innerCmd = c->u.subshell_command;
		int p = fork();
		if (p == 0)
		{
			//if (c->input != NULL)
			//{
			//	innerCmd->input = c->input;
			//}
			//if (c->output != NULL)
			//{
			//	innerCmd->output = c->output;
			//}

			// setup redirection. 
			if (c->input != NULL)
			{
				int fin = open(c->input, O_RDONLY, 0644);
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

			execute(innerCmd);
		}
		else
		{
			int status;
			waitpid(p, &status, 0);
			int stat = WEXITSTATUS(status);
			innerCmd->status = stat;
			c->status = stat; // may be reassigned again later, but should be fine.
			exit(stat);
		}
		break;
	}
	
	//default:
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
		// just waits for the child to finish. 
		int status;
		waitpid(p, &status, 0);
		int exitStatus = WEXITSTATUS(status);
		c->status = exitStatus;
	}
}
