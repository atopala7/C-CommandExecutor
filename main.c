#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <sys/mman.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

void writeOutput(char* command, char* output);

int main(int argc, char** argv)
{
	// the process ID
	pid_t pid;
	// the child process's status
	int status;

	// the size of the memory object
	const int SIZE = 4096;
	// the name of the memory object
	const char *name = "memory1";
	// the shared memory file descriptor
	int fd;
	// a pointer to the shared memory object
	char *ptr;
	// a string written to shared memory
	//char *message_0 = "hello";

	char ch;
	// a file pointer
	FILE *fp;
	
	// array for the commands read from the file
	char **cmd;

	// a pointer used to look through the string in memory for line breaks, and to give to cmd each character before them
	char *ptr2;
	// an index used for creating the strings in cmd
	int stringIndex;

	// The number of lines read from the input
	int numLines;

	// the size of a line in the cmd array
	const int LINE_SIZE = 128;

	// the file descriptors for the pipe
	int pipeFd[2];

	char *command;
	char **arguments;

	char c;
	int strLen;
	int numArgs;
	int strPtr;
	int argPtr;
	int currentArg;

	char read_msg[SIZE];
	
	// Ensure we have a single command-line argument
	if (argc != 2)
	{
		fprintf(stderr, "Invalid number of arguments.\n");
		return 1;
	}

	// Fork a child process to read the input file
	pid = fork();

	if (pid < 0) // Error occurred
	{
		fprintf(stderr, "Fork failed");
		return 1;	
	}
	else if (pid == 0) // Child process
	{
		// Open the file given as an argument
		fp = fopen((char *)argv[1], "r");
		if (fp == NULL)
		{
			fprintf(stderr, "Failed to read file.\n");
			return 1;	
		}

		// Create the shared memory object
		fd = shm_open(name, O_CREAT | O_RDWR, 0666);
		// Configure the size of the shared memory object
		ftruncate(fd, SIZE);
		// Memory map the shared memory object
		ptr = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		// Write each character from the file to the shared memory object
		while ((ch = fgetc(fp)) != EOF)
		{
			// Write to the shared memory object
			sprintf(ptr, "%c", ch);
			ptr += 1;//strlen(message_0);		
		}

		// Close the file
		fclose(fp);

		return 0;
	}
	else // Parent process
	{
		pid = wait(&status);

		// Open the shared memory object
		//fd = shm_open(name, O_RDONLY, 0666);
		fd = shm_open(name, O_CREAT | O_RDWR, 0666);
		// Memory map the shared memory object
		ptr = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		numLines = 0;
		ptr2 = ptr;

		// Count the number of lines in the memory area
		while (*ptr2 != '\0')
		{
			if (*ptr2 == '\n')
			{
				numLines += 1;
			}
			ptr2 += 1;
		}

		cmd = malloc(numLines * sizeof(char *));

		for (int i = 0; i < numLines; i++)
		{
			cmd[i] = malloc(LINE_SIZE * sizeof *cmd[i]);
		}

		ptr2 = ptr;
		for (int i = 0; i < numLines; i++)
		{
			stringIndex = 0;
			while (*ptr2 != '\0' && *ptr2 != '\n' && *ptr2 != '\r')
			{
				cmd[i][stringIndex] = (char)ptr2[0];	
				stringIndex += 1;
				ptr2 += 1;
			}
			while (*ptr2 == '\n' || *ptr2 == '\r')
			{
				ptr2 += 1;
			}
		}

		for (int i = 0; i < numLines; i++)
		{
			c = cmd[i][0];

			strPtr = 0;
			strLen = 0;
			numArgs = 0;

			while (c != '\0')
			{
				if (c != ' ')
				{
					while ((int)c > 32 && (int)c < 123)
					{
						strLen += 1;
						c = cmd[i][strLen];
					}
					numArgs += 1;
				}
				strLen += 1;
				c = cmd[i][strLen];
			}
			numArgs -= 1;
			strLen -= 1;

			strPtr = 0;
			c = cmd[i][strPtr];
			while ((int)c > 32 && (int)c < 123)
			{
				strPtr += 1;
				c = cmd[i][strPtr];
			}

			command = malloc(LINE_SIZE * sizeof(char));
			arguments = malloc((numArgs+1) * sizeof(char *));
			for (int j = 0; j < numArgs+1; j++)
			{
				arguments[j] = malloc(LINE_SIZE * sizeof *arguments[j]);
			}

			for (int j = 0; j < strPtr; j++)
			{
				command[j] = cmd[i][j];
				arguments[0][j] = command[j];
			}
			command[strPtr] = '\0';
			arguments[0][strPtr] = '\0';

			currentArg = 1;
			argPtr = 0;
			for (int j = strPtr; j < strLen; j++)
			{
				c = cmd[i][j];
				if (c != ' ')
				{
					argPtr = 0;
					while ((int)c > 32 && (int)c < 123)
					{
						arguments[currentArg][argPtr] = c;
						j += 1;
						c = cmd[i][j];
						argPtr += 1;
					}
					arguments[currentArg][argPtr] = '\0';
					currentArg += 1;
				}
			}

			arguments[numArgs+1] = NULL;

			// Create a pipe
			if (pipe(pipeFd) == -1)
			{
				fprintf(stderr, "Pipe failed.");
				return 1;
			}
			// Fork a child process
			pid = fork();

			if (pid < 0) // Error occurred
			{
				fprintf(stderr, "Fork failed");
				return 1;	
			}
			else if (pid == 0) // Child process
			{
				// Close the unused end of the pipe
				close(pipeFd[READ_END]);


				// Send stdout and stderr to the pipe
				dup2(pipeFd[WRITE_END], 1);
				dup2(pipeFd[WRITE_END], 2);
				close(pipeFd[WRITE_END]);

				execvp(command, arguments);

				return 0;
			}
			else // Parent process
			{
				wait(0);
				close(pipeFd[WRITE_END]);

				// Clean read_msg
				for (int j = 0; j < SIZE; j++)
					read_msg[j] = '\0';

				// Read from the pipe
				read(pipeFd[READ_END], read_msg, SIZE);

				writeOutput(cmd[i], read_msg);
			}

			/*
			// Free the memory allocated for the command string 
			free(command);
			// Free the memory allocated for arguments
			for (int j = 0; j < numArgs; j++)
			{
				free(arguments[j]);
			}
			free(arguments);
			*/
		}

		// Remove the shared memory object
		shm_unlink(name);

		// Free the memory allocated for cmd
		for (int i = 0; i < numLines; i++)
		{
			free(cmd[i]);
		}
		free(cmd);
	}

	return 0;
}

void writeOutput(char* command, char* output)
{
	printf("The output of: %s : is\n", command);
	printf(">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);	
}
