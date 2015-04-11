// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
//#include <cctype.h>

static int
charaCase(char c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9') || (c == '!') || (c == '%')
		|| (c == '+') || (c == ',') || (c == '-') || (c == '.')
		|| (c == '/') || (c == ':') || (c == '@') || (c == '^')
		|| (c == '_')) // chars that form a command.
		return 0;
	else if ((c == ';') || (c == '|') || (c == '&')) // binary ops.
		return 1;
	else if ((c == '(') || (c == ')')) // unary ops.
		return 2;
	else if ((c == '<') || (c == '>')) // redirection.
		return 3;
	else if (c == '\n') // special "empty space" token.
		return 4;
	else if (c == ' ' || c == '\t') // "empty space"
		return 5;
	else if (c == '#') // comment. 
		return 6;
	else if (c == EOF) // end of line.
		return 7;
	else // unknown character. 
		return -1;
	// continue...?
}

static int
opPrecedence(enum command_type commandType)
{
	switch (commandType)
	{
	case SEQUENCE_COMMAND:
		return 0;
	case AND_COMMAND:
	case OR_COMMAND:
		return 1;
	case PIPE_COMMAND:
		return 2;
	case SIMPLE_COMMAND:
	case SUBSHELL_COMMAND:
		return -1;
	}
	return -1; // this really shouldn't be used. 
}

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct commandNode
{
	struct command *command;
	struct commandNode *next;
};

struct command_stream
{
	struct commandNode *head;
	struct commandNode *tail;
	struct commandNode *cursor; // this allows for our implementation of 
								// read_command_stream. 
								// should be initialized to the head pointer. 
};




command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");

  int lineNum = 1; // this will be used for error messages. 

  // maintains the stack for operations. 
  // this will use the enum values provided in command-internals as values. 
  int opStackSize = 10; // TODO Change?
  enum command_type* opStack = (enum command_type*)malloc(opStackSize * sizeof(enum command_type));
  int opIndex = 0;

  // maintains the stack for commands. 
  int cmdStackSize = 10; // TODO Change?
  struct command** cmdStack = (struct command**) malloc(cmdStackSize*sizeof(struct command*));
  int cmdIndex = 0;

  // maintains the buffer for the string for commands, etc.
  int bufSize = 100; // TODO change?
  char* buffer = (char*)malloc(bufSize * sizeof(char));
  int bufIndex = 0;
  int numWords = 0;

  //initializes the command_stream_t that will be returned. 
  command_stream_t commands = (command_stream_t) malloc(sizeof(struct command_stream));
  commands->head = NULL;
  commands->tail = NULL;
  commands->cursor = NULL;

  char c = get_next_byte(get_next_byte_argument);
  int charType = charaCase(c);
  int prevType = 4; // This corresponds with a newline character. This implies a new command tree. 
  // the prevType variable only takes on 3 values: 0 (for previous is a command)
  // 1 (for previous is an operator), and 4 (for it's time to start a new command tree)
  while (c != EOF) // TODO check the end condition. 
  { 
	  if (charType == 0) // letters, numbers, etc.
	  {
		  buffer[bufIndex] = c;
		  bufIndex++;
		  c = get_next_byte(get_next_byte_argument);
		  charType = charaCase(c);

		  int prevBufCase = 5;

		  // reads in everything up until an operator or the end of a line/file
		  while (charType == 0 || charType == 5)
		  {
			  if (bufIndex >= bufSize)
			  {
				  bufSize *= 2;
				  buffer = (char*)realloc(buffer, bufSize * sizeof(char));
			  }
			  buffer[bufIndex] = c;
			  bufIndex++;

			  if (prevBufCase == 5 && charType == 0)
			  {
				  numWords++;
			  }

			  prevBufCase = charType;
			  c = get_next_byte(get_next_byte_argument);
			  charType = charaCase(c);
		  }

		  // parses the words from the buffer. 
		  if (bufIndex >= bufSize)
		  {
			  bufSize *= 2;
			  buffer = (char*)realloc(buffer, bufSize * sizeof(char));
		  }
		  buffer[bufIndex] = 0;
		  bufIndex++;

		  struct command *cmd = (struct command*) malloc(sizeof(struct command));
		  cmd->type = SIMPLE_COMMAND;
		  cmd->status = -1;

		  cmd->u.word = (char**)malloc(numWords*sizeof(char*));

		  // creates a buffer to which the words array can point into. 
		  char* cmd_words = (char*)malloc(bufIndex*sizeof(char));

		  int i = 0;
		  int wordNum = 0;
		  while(i < bufIndex - 1) // the last char is a null byte. 
		  {
			  // points to the first character of non-space character. 
			  cmd->u.word[wordNum] = &(cmd_words[i]);
			  wordNum++;
			  while (charaCase(buffer[i]) == 0)
			  {
				  cmd_words[i] = buffer[i];
				  i++;
			  }
			  // removes all space characters into that of null bytes. 
			  while (charaCase(buffer[i]) == 5)
			  {
				  cmd_words[i] = 0;
				  i++;
			  }
		  }

		  // place the command on the stack. 
		  if (cmdIndex >= cmdStackSize)
		  {
			  cmdStackSize *= 2;
			  cmdStack = (struct command**) realloc(cmdStack, cmdStackSize*sizeof(struct command*));
		  }
		  cmdStack[cmdIndex] = cmd;
		  cmdIndex++;

		  bufIndex = 0; // resets the buffer. 
		  prevType = 0;
	  }
	  else if (charType == 1) //binary operator
	  {
		  // we cannot have a binary operator without a command in front. 
		  if (prevType != 0)
		  {
			  error(1, 1, "Syntax Error in Line %d: Expected binary operator preceded by operand", lineNum);
		  }
		  enum command_type opType;

		  // determine which kind of operation it is. 
		  if (c == ';')
		  {
			  opType = SEQUENCE_COMMAND;
			  c = get_next_byte(get_next_byte_argument);
		  }
		  else if (c == '&')
		  {
			  char c_next = get_next_byte(get_next_byte_argument);
			  if (c_next != '&')
			  {
				  error(1, 1, "Error in Line %d: Character Unknown", lineNum);
			  }
			  opType = AND_COMMAND;
			  c = get_next_byte(get_next_byte_argument);
		  }
		  else if (c == '|')
		  {
			  char c_next = get_next_byte(get_next_byte_argument);
			  if (c_next == '|')
			  {
				  opType = OR_COMMAND;
				  c = get_next_byte(get_next_byte_argument);
			  }
			  else
			  {
				  opType = PIPE_COMMAND;
				  c = c_next;
			  }
			  // TODO check other cases for errors?
		  }

		  int opPrec = opPrecedence(opType);

		  // pop all operators with >= precedence off operator stack. 
		  while (opIndex > 0)
		  {
			  // check the top of the operator stack. 
			  enum command_type topOp = opStack[opIndex - 1];
			  // if (, stop. 
			  if (topOp == SUBSHELL_COMMAND)
				  break;
			  int topPrec = opPrecedence(topOp);
			  // if top has lower precedence, stop. 
			  if (topPrec < opPrec)
				  break;
			  // here, top has a precedence >= current operator. 
			  opIndex--; // pop the operator off the stack. 

			  // pop two commands off the stack. 
			  if (cmdIndex < 2)
			  {
				  error(1, 1, "Syntax Error in Line %d: Binary operator without enough operands", lineNum);
			  }
			  cmdIndex--;
			  struct command *cmdb = cmdStack[cmdIndex];
			  cmdIndex--;
			  struct command *cmda = cmdStack[cmdIndex];

			  struct command *newCmd = (struct command *)malloc(sizeof(struct command));
			  newCmd->type = topOp;
			  newCmd->status = -1;
			  newCmd->input = NULL;
			  newCmd->output = NULL;
			  newCmd->u.command[0] = cmda;
			  newCmd->u.command[1] = cmdb;

			  // place the command on the stack. 
			  // no need to check index because two commands just popped off. 
			  cmdStack[cmdIndex] = newCmd;
			  cmdIndex++;
		  }
		  // push new operator onto operator stack. 
		  if (opIndex >= opStackSize)
		  {
			  opStackSize *= 2;
			  opStack = (enum command_type*)realloc(opStack, opStackSize*sizeof(enum command_type));
		  }
		  opStack[opIndex] = opType;
		  opIndex++;

		  prevType = 1;
	  }
	  else if (charType == 2)
	  {
		  if (c == '(') // TODO are there any conditions of use?
		  {
			  enum command_type opType = SUBSHELL_COMMAND;

			  if (opIndex >= opStackSize)
			  {
				  opStackSize *= 2;
				  opStack = (enum command_type*)realloc(opStack, opStackSize*sizeof(enum command_type));
			  }
			  opStack[opIndex] = opType;
			  opIndex++;
			  prevType = 1; // the open paren is similar to the binary ops for the following chars.
		  }
		  else // c == ')'
		  {
			  struct command *newCmd;
			  // TODO essentially copy paste of while loop above? can we turn that into a function easily?
			  // pop all operators with >= precedence off operator stack. 
			  while (opIndex > 0)
			  {
				  // check the top of the operator stack. 
				  enum command_type topOp = opStack[opIndex - 1];
				  // if (, stop. 
				  if (topOp == SUBSHELL_COMMAND)
					  break;

				  // keep popping operators until we reach a '(' (SUBSHELL_COMMAND on stack)
				  opIndex--; // pop the operator off the stack. 

				  // for each operator popped off, 
				  // pop two commands off the stack, as all other operators are binary operators. 
				  if (cmdIndex < 2)
				  {
					  error(1, 1, "Syntax Error in Line %d: Binary operator without enough operands", lineNum);
				  }
				  cmdIndex--;
				  struct command *cmdb = cmdStack[cmdIndex];
				  cmdIndex--;
				  struct command *cmda = cmdStack[cmdIndex];

				  newCmd = (struct command *)malloc(sizeof(struct command));
				  newCmd->type = topOp;
				  newCmd->status = -1;
				  newCmd->input = NULL;
				  newCmd->output = NULL;
				  newCmd->u.command[0] = cmda;
				  newCmd->u.command[1] = cmdb;

				  // place the command on the stack. 
				  // should never overflow, as 2 were just popped off. 
				  cmdStack[cmdIndex] = newCmd;
				  cmdIndex++;
			  }
			  if (opIndex <= 0)
			  {
				  error(1, 1, "Syntax Error in Line %d: No matching open parentheses", lineNum);
			  }
			  opIndex--; // pop the '(' off the operator stack. 

			  if (cmdIndex <= 0)
			  {
				  error(1, 1, "Syntax Error in Line %d: No command in parentheses", lineNum);
			  }
			  cmdIndex--;
			  struct command *cmd_in = cmdStack[cmdIndex];
			  struct command *cmd_out = (struct command *)malloc(sizeof(struct command));
			  cmd_out->type = SUBSHELL_COMMAND;
			  cmd_out->status = -1;
			  cmd_out->u.subshell_command = cmd_in;

			  // place the command on the stack. 
			  // should never overflow, as one command was just popped off. 
			  cmdStack[cmdIndex] = newCmd;
			  cmdIndex++;
			  prevType = 0; // after a close paren, it acts as if we just created a new command.
		  }

		  c = get_next_byte(get_next_byte_argument);
	  }
	  else if (charType == 3) // redirection
	  {
		  if (prevType != 0) // && cmdIndex <= 0
		  {
			  error(1, 1, "Syntax Error in Line %d: expected command preceding redirection", lineNum);
		  }
		  char ch = c;

		  // ignores everything up until the first non-whitespace character. 
		  c = get_next_byte(get_next_byte_argument);
		  charType = charaCase(c);
		  while (charType == 5)
		  {
			  c = get_next_byte(get_next_byte_argument);
			  charType = charaCase(c);
		  }

		  // if letters/digits/etc. are read before the next newline/operator,  
		  if (charType != 0) 
		  {
			  error(1, 1, "Error in Line %d: Expected input or output", lineNum);
		  }

		  // reads in everything up until an operator or the end of a line/file
		  while (charType == 0 || charType == 5)
		  {
			  if (bufIndex >= bufSize)
			  {
				  bufSize *= 2;
				  buffer = (char*)realloc(buffer, bufSize * sizeof(char));
			  }
			  buffer[bufIndex] = c;
			  bufIndex++;
			  c = get_next_byte(get_next_byte_argument);
			  charType = charaCase(c);
		  }

		  // trims the buffer of ending whitespaces
		  while (charaCase(buffer[bufIndex - 1]) == 5)
		  {
			  bufIndex--; 
		  }

		  if (ch == '<') // check previous input is NULL?
		  {
			  // malloc an input. 
			  cmdStack[cmdIndex - 1]->input = (char*)malloc(bufIndex * sizeof(char)); 
			  int i = 0;
			  for (; i < bufIndex; i++)
			  {
				  cmdStack[cmdIndex - 1]->input[i] = buffer[i];
			  }
		  }
		  else
		  {
			  // malloc an output. 
			  cmdStack[cmdIndex - 1]->output = (char*)malloc(bufIndex * sizeof(char));
			  int i = 0;
			  for (; i < bufIndex; i++)
			  {
				  cmdStack[cmdIndex - 1]->output[i] = buffer[i];
			  }
		  }

		  bufIndex = 0; // reset the buffer. 
		  prevType = 0; // after this, this is the same as if a new command was created.
	  }
	  else if (charType == 4) // newline. 
	  {
		  // previous things form a command. 
		  // this newline can be a considered a SEQUENCE_COMMAND. can we?
		  if (prevType == 0)
		  {
			  // treat the newline as a SEQUENCE_COMMAND, which has lower precedence than all other 
			  // commands, which implies that the command stack should be completely empty
			  // after this operation. However, there is a chance that this is the end of the 
			  // command tree. 
			  while (opIndex > 0)
			  {
				  // check the top of the operator stack. 
				  enum command_type topOp = opStack[opIndex - 1];
				  // TODO check something: 
				  // the specs say that newlines can appear before ), but this doesn't make
				  // any sense to me. This assumes that it cannot. 
				  if (topOp == SUBSHELL_COMMAND)
				  {
					  error(1, 1, "Syntax Error in Line %d: Expected closing parentheses", lineNum);
				  }
				  opIndex--; // pop the operator off the stack. 

				  // pop two commands off the stack. 
				  if (cmdIndex < 2)
				  {
					  error(1, 1, "Syntax Error in Line %d: Binary operator without enough operands", lineNum);
				  }
				  cmdIndex--;
				  struct command *cmdb = cmdStack[cmdIndex];
				  cmdIndex--;
				  struct command *cmda = cmdStack[cmdIndex];

				  struct command *newCmd = (struct command *)malloc(sizeof(struct command));
				  newCmd->type = topOp;
				  newCmd->status = -1;
				  newCmd->input = NULL;
				  newCmd->output = NULL;
				  newCmd->u.command[0] = cmda;
				  newCmd->u.command[1] = cmdb;

				  // place the command on the stack. 
				  // no need to check indexes, as two commands just popped. 
				  cmdStack[cmdIndex] = newCmd;
				  cmdIndex++;
			  }
			  if (opIndex > 0)
			  {
				  error(1, 1, "Syntax Error in Line %d: Operators without enough operands", lineNum);
			  }
			  if (cmdIndex > 1)
			  {
				  error(1, 1, "Syntax Error in Line %d: Operands without enough operators", lineNum);
			  }
			  char c_next = get_next_byte(get_next_byte_argument);
			  if (c_next == '\n' || c_next == EOF)
			  {
				  //END OF COMMAND TREE.

				  struct commandNode *node = (struct commandNode*) malloc(sizeof(struct commandNode));
				  node->command = cmdStack[0];
				  node->next = NULL;

				  if (commands->head == NULL)
				  {
					  commands->head = node;
					  commands->tail = node;
					  commands->cursor = node;
				  }
				  else
				  {
					  commands->tail->next = node;
					  commands->tail = node;
				  }
				  prevType = 4;
				  cmdIndex = 0; // stack is reset. 
				  c = get_next_byte(get_next_byte_argument);
			  }
			  else
			  {
				  //Treat this newline as a semicolon (sequence command). 
				  //since sequence commands have lowest priority, we can assume that all
				  // remaining commands/operators should come off at this point, under
				  // expected circumstances. 
				  // push new operator onto operator stack. 
				  if (opIndex >= opStackSize)
				  {
					  opStackSize *= 2;
					  opStack = (enum command_type*)realloc(opStack, opStackSize*sizeof(enum command_type));
				  }
				  opStack[opIndex] = SEQUENCE_COMMAND;
				  opIndex++;
				  prevType = 1; // as if we just added an operator onto the stack. 
				  c = c_next;
			  }
		  }
		  else if (prevType == 1) // previous is a command. 
		  {
			  enum command_type topOp = opStack[opIndex - 1];
			  // this deals with the optional semicolon ending of any command tree line.
			  if (topOp == SEQUENCE_COMMAND)
			  {
				  char c_next = get_next_byte(get_next_byte_argument);

				  if (c_next == '\n' || c_next == EOF)
				  {
					  // END OF COMMAND TREE. 
					  opIndex--; // remove the SEQUENCE_COMMAND
					  if (opIndex > 0)
					  {
						  error(1, 1, "Syntax Error in Line %d: Operators without enough operands", lineNum);
					  }
					  if (cmdIndex > 1)
					  {
						  error(1, 1, "Syntax Error in Line %d: Operands without enough operators", lineNum);
					  }

					  struct commandNode *node = (struct commandNode*) malloc(sizeof(struct commandNode));
					  node->command = cmdStack[0];
					  node->next = NULL;

					  if (commands->head == NULL)
					  {
						  commands->head = node;
						  commands->tail = node;
						  commands->cursor = node;
					  }
					  else
					  {
						  commands->tail->next = node;
						  commands->tail = node;
					  }
					  prevType = 4;
					  cmdIndex = 0; // stack is reset. 
					  c = get_next_byte(get_next_byte_argument);
				  }
				  else
				  {
					  // then we just advance the character. 
					  // in the case that the top operation is not a sequence command, 
					  // we treat the newline as an empty space and maintain prevType.
					  c = c_next; // prevType remains 1.
				  }
			  }
		  }
		  else // prevType == 4;
		  {
			  // just advance the counter. treat as empty space. 
			  // keep whatever prevType it is. 
			  c = get_next_byte(get_next_byte_argument);
		  }

		  lineNum++; //increase the line counter for errors sake.
	  }
	  else if (charType == 5) // empty space. 
	  {
		  // just advance to the next character. 
		  c = get_next_byte(get_next_byte_argument);
		  // keep the same prevType
	  }
	  else if (charType == 6) // comment. 
	  {
		  c = get_next_byte(get_next_byte_argument);
		  while (c != '\n' && c != EOF)
		  {
			  c = get_next_byte(get_next_byte_argument);
		  }
	  }
	  else // unknown character
	  {
		  error(1, 1, "Error in Line %d: %c Character Unknown", lineNum, c);
	  }	
	  charType = charaCase(c);
  }

  if (prevType == 1)
  {
	  error(1, 1, "Syntax Error in Line %d: Expected operand following operator", lineNum);
  }

  // pop all operators with >= precedence off operator stack.
  while (opIndex > 0)
  {
	  // check the top of the operator stack. 
	  enum command_type topOp = opStack[opIndex - 1];
	  // if (, stop. 
	  if (topOp == SUBSHELL_COMMAND)
	  {
		  error(1, 1, "Syntax Error in Line %d: Expected closing parentheses", lineNum);
	  }
	  opIndex--; // pop the operator off the stack. 

	  // pop two commands off the stack. 
	  if (cmdIndex < 2)
	  {
		  error(1, 1, "Syntax Error in Line %d: Binary operator without enough operands", lineNum);
	  }
	  cmdIndex--;
	  struct command *cmdb = cmdStack[cmdIndex];
	  cmdIndex--;
	  struct command *cmda = cmdStack[cmdIndex];

	  struct command *newCmd = (struct command *)malloc(sizeof(struct command));
	  newCmd->type = topOp;
	  newCmd->status = -1;
	  newCmd->input = NULL;
	  newCmd->output = NULL;
	  newCmd->u.command[0] = cmda;
	  newCmd->u.command[1] = cmdb;

	  // place the command on the stack. 
	  // no need to check indexes, as two commands just popped. 
	  cmdStack[cmdIndex] = newCmd;
	  cmdIndex++;
  }

  if (opIndex > 0)
  {
	  error(1, 1, "Syntax Error in Line %d: Operators without enough operands", lineNum);
  }
  if (cmdIndex > 1)
  {
	  error(1, 1, "Syntax Error in Line %d: Operands without enough operators", lineNum);
  }

  if (cmdIndex == 1)
  {
	  struct commandNode *node = (struct commandNode*) malloc(sizeof(struct commandNode));
	  node->command = cmdStack[0];
	  node->next = NULL;

	  if (commands->head == NULL)
	  {
		  commands->head = node;
		  commands->tail = node;
		  commands->cursor = node;
	  }
	  else
	  {
		  commands->tail->next = node;
		  commands->tail = node;
	  }
  }

  free(opStack);
  free(cmdStack);
  free(buffer);
  return commands;
}

command_t
read_command_stream (command_stream_t s)
{
	if (s->cursor == NULL)
		return NULL;
	struct commandNode* temp = s->cursor;
	s->cursor = s->cursor->next;
	return temp->command;
}
