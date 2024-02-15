# Command Executor with Interprocess Communication (IPC)

## Overview

This project is a demonstration of using Interprocess Communication (IPC) mechanisms, specifically shared memory and pipes, in a POSIX-compliant environment to read, execute commands from a file, and display their output. The program forks child processes for reading commands from a file and executing them, using shared memory for command transfer and pipes for capturing command output.

## Features

- **Reads Commands from File**: Processes commands listed in a user-specified input file.
- **Uses Shared Memory for IPC**: Employs shared memory to transfer command data between processes.
- **Captures and Displays Command Output**: Executes commands and captures their outputs via pipes, displaying the outputs to the user.
- **Dynamic Memory Management**: Manages dynamic memory allocations for command storage and execution.
- **Error Handling and Process Synchronization**: Implements error checking for system calls and synchronizes processes to ensure orderly execution and output display.

## Requirements

- A POSIX-compliant operating system (Linux/Unix)
- GCC compiler or any compatible C compiler

## Compilation

To compile the program, use the following command in your terminal:

```bash
gcc -o command_executor main.c -lrt
```

## Usage

After compilation, run the program by providing the path to the input file as an argument:

```bash
./command_executor input.txt
```

Ensure that input.txt contains the commands you wish to execute, with each command on a new line.

## Input File Format

The input file should contain one command per line. For example:

```plaintext
ls -l
echo Hello World
whoami
```

## Output

The program executes each command in the input file and prints its output to the standard output in the following format:

```bash
The output of: [COMMAND] : is
>>>>>>>>>>>>>>>
[COMMAND OUTPUT]
<<<<<<<<<<<<<<<
```
