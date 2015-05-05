// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>

#include "command.h"

typedef struct {
	command_t command; // root of command tree;
	struct GraphNode **before;	// list of all GraphNodes before it that it must wait
								// on to finish in order to execute this command. 
	// consider having an after list as well, in the future. 
	pid_t;	// initialize this to -1. 
			// if it is equal to -1, it has not yet spawned a child node. 
} GraphNode;

typedef struct {
	GraphNode* graphnode;
	// TODO implement read list and write list. 
	ListNode* next; // should be initialized to NULL
} ListNode;

typedef struct {
	// TODO implementation here. 
	// probably just maintain a linked list of GraphNodes somehow. 
	ListNode* no_dependencies; // head of the nodes that have no dependencies
	ListNode* dependencies; // head of the nodes that have dependencies. 
} DependencyGraph;

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

static bool
haveDependency()
{
	// TODO implement.
}

static void
processCommand(command_t cmd)
{
	switch (cmd->type)
	{
	case SIMPLE_COMMAND:
		// TODO implement. 
		break;
	case SUBSHELL_COMMAND:
		// TODO implement. 
		processCommand(cmd->u.subshell_command);
		break;
	default:
		processCommand(cmd->u.command[0]);
		processCommand(cmd->u.command[1]);
	}
}

static DependencyGraph*
createGraph(command_stream_t stream);
{
	// TODO implementation here. 
}

static void
executeNoDependencies(ListNode *no_dependencies) 
{
	// TODO implementation here.
	ListNode *current_node = no_dependencies;
	while (current_node != NULL)
	{
		GraphNode *i = current_node.graphnode;
		pid_t pid = fork();
		if (pid == 0)
		{
			execute_command(i->command, true);
			exit(0);
		}
		else
		{
			i->pid = pid;
		}
		current_node = current_node->next;
	}
}

static void
executeDependencies(ListNode *dependencies) 
{
	// TODO implementation here. 
	ListNode *current_node = dependencies;
	while (current_node != NULL)
	{
		int status;
		GraphNode *i = current_node.graphnode;
		// TODO check for befores and stuff. 
		// not currently too sure what to do. 

		pid_t pid = fork();
		if (pid == 0)
		{
			execute_command(i->command, true);
			exit(0);
		}
		else
		{
			i->pid = pid;
		}
		current_node = current_node->next;
	}
}

static int
executeGraph(DependencyGraph* graph)
{
	executeNoDependencies(graph->no_dependencies);
	executeDependencies(graph->dependencies);
}


int
main (int argc, char **argv)
{
  int command_number = 1;
  bool print_tree = false;
  bool time_travel = false;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = true; break;
      case 't': time_travel = true; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;

  // TODO check if this is correct?
  if (time_travel)
  {
	  DependencyGraph *graph = createGraph(command_stream_t stream);
	  int finalStatus = 0;
	  finalStatus = executeGraph(graph);
	  return finalStatus;
  }
  else
  {
	  while ((command = read_command_stream(command_stream)))
	  {
		  if (print_tree)
		  {
			  printf("# %d\n", command_number++);
			  print_command(command);
		  }
		  else
		  {
			  last_command = command;
			  execute_command(command, time_travel);
		  }
	  }

  }

  return print_tree || !last_command ? 0 : command_status (last_command);
}
