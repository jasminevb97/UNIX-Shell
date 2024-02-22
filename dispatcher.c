#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dispatcher.h"
#include "shell_builtins.h"
#include "parser.h"

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

/**
 * dispatch_external_command() - run a pipeline of commands
 *
 * @pipeline:   A "struct command" pointer representing one or more
 *              commands chained together in a pipeline.  See the
 *              documentation in parser.h for the layout of this data
 *              structure.  It is also recommended that you use the
 *              "parseview" demo program included in this project to
 *              observe the layout of this structure for a variety of
 *              inputs.
 *
 * Note: this function should not return until all commands in the
 * pipeline have completed their execution.
 *
 * Return: The return status of the last command executed in the
 * pipeline.
 */
static int dispatch_external_command(struct command *pipeline)
{
	 //References used for project 2, deliverable 2:
	 //https://stackoverflow.com/questions/63858076/using-execvp-to-execute-command-line-arguments-using-full-path-name-errors
	 //https://ece.uwaterloo.ca/~dwharder/icsrts/Tutorials/fork_exec/
	 //https://www.openai.com 
	

	//this checks if pipeline is null then it prints error message
	if (pipeline == NULL) {
		fprintf(stderr, "No command to execute.\n");
		return -1;
	}

	//variables for file redirection
	int initialFD = STDIN_FILENO; //file descriptor to read file
	int inputFD; //file descriptor for input file
	int outputFD; //file descriptor for output file

	int fds[2]; //array for file descriptor 0 and 1 for pipe

	struct command *currentCommand = pipeline; //points to the next command in list to be processed in pipeline

	while (pipeline) {
		//this checks if argv or command line is null then print error message
		if (!currentCommand->argv[0]){
			fprintf(stderr, "No Command.\n");
			return -1;
		}

		//create a pipe for next command
		if (pipeline->output_type == COMMAND_OUTPUT_PIPE && pipe(fds) == -1){
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		//creates new process by copying the current process
		pid_t pid = fork();

		if (pid == 0) { //first child
			//file redirection for input
			if (pipeline->input_filename) {
				inputFD = open(pipeline->input_filename, O_RDONLY);
				if (inputFD == -1) {
					fprintf(stderr, "error: input file\n");
					exit(EXIT_FAILURE);
				}
				if (dup2(inputFD, STDIN_FILENO) == -1){
					fprintf(stderr, "error: input file redirection\n");
					exit(EXIT_FAILURE);
				}
			}
			//file redirection for previous command
			if (initialFD != STDIN_FILENO) { 
				dup2(initialFD, STDIN_FILENO);
				close(initialFD);
			}

			//file redirection for output 
			if (pipeline->output_type == COMMAND_OUTPUT_PIPE){
				close(fds[0]); //close read side of pipe
				dup2(fds[1], STDOUT_FILENO); //redirect to write
				close(fds[1]); //close write side of pipe
			} else if (pipeline->output_filename) {
				int flags = O_WRONLY | O_CREAT;
				//Reference: for this output section of my code I got assistance from: //https://www.openai.com
				if (pipeline->output_type == COMMAND_OUTPUT_FILE_APPEND) {
					flags |= O_APPEND;
				} else {
					flags |= O_TRUNC;
				}

				outputFD = open(pipeline->output_filename, flags, 0644); //file permission
				if (outputFD == -1) {
					fprintf(stderr, "error: output file\n");
					exit(EXIT_FAILURE);
				}
				dup2(outputFD, STDOUT_FILENO);
				close(outputFD);
			}

			execvp(pipeline->argv[0], pipeline->argv); //command execution
			fprintf(stderr, "error: cannot find command\n");
			exit(EXIT_FAILURE);
			
			} 
			//checks if fork is a negative value then it prints error message
			else if (pid < 0) {
				fprintf(stderr, "fork error\n");
				return -1;
			}

			//parent process
			if (initialFD != STDIN_FILENO) {
				close(initialFD);
			}

			if (pipeline->output_type == COMMAND_OUTPUT_PIPE) {
				close(fds[1]); //this closes the pipe that writes
				initialFD = fds[0]; //this reads from next command
			}

		//this part is done by the parent 
		int status; //exit status of child process
		if (pipeline->output_type != COMMAND_OUTPUT_PIPE) {
			waitpid(pid, &status, 0); //wait for child process to finish
			if (WIFEXITED(status)) {
				return WEXITSTATUS(status);
			} else {
				return -1;
			}
		}

		pipeline = pipeline->pipe_to;
	}

	return 0;
}

/**
 * dispatch_parsed_command() - run a command after it has been parsed
 *
 * @cmd:                The parsed command.
 * @last_rv:            The return code of the previously executed
 *                      command.
 * @shell_should_exit:  Output parameter which is set to true when the
 *                      shell is intended to exit.
 *
 * Return: the return status of the command.
 */
static int dispatch_parsed_command(struct command *cmd, int last_rv,
				   bool *shell_should_exit)
{
	/* First, try to see if it's a builtin. */
	for (size_t i = 0; builtin_commands[i].name; i++) {
		if (!strcmp(builtin_commands[i].name, cmd->argv[0])) {
			/* We found a match!  Run it. */
			return builtin_commands[i].handler(
				(const char *const *)cmd->argv, last_rv,
				shell_should_exit);
		}
	}

	/* Otherwise, it's an external command. */
	return dispatch_external_command(cmd);
}

int shell_command_dispatcher(const char *input, int last_rv,
			     bool *shell_should_exit)
{
	int rv;
	struct command *parse_result;
	enum parse_error parse_error = parse_input(input, &parse_result);

	if (parse_error) {
		fprintf(stderr, "Input parse error: %s\n",
			parse_error_str[parse_error]);
		return -1;
	}

	/* Empty line */
	if (!parse_result)
		return last_rv;

	rv = dispatch_parsed_command(parse_result, last_rv, shell_should_exit);
	free_parse_result(parse_result);
	return rv;
}
