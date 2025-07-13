# Simple Streaming Service in C

## Overview
This repository contains a C program that simulates a basic streaming service. It manages users and movies through various operations, executed in response to specific input commands. The core functionality includes user registration, movie management, and suggestions based on user preferences, all handled through linked lists.

## Structure of the Repository
- `main.c`: The main driver of the program, orchestrating the flow of data and responses to different events.
- `streaming_service.c`: Implements the logic for each functionality like user registration, movie addition, and suggestions.
- `streaming_service.h`: Header file with definitions for structures (user, movie, new_movie, suggested_movie) and declarations of functions used in the program.

## Features
- **User Operations**: Register new users, maintain and manage user data, including watch history and suggested movies.
- **Movie Management**: Add movies with details, categorize them, and manage movie suggestions based on user interactions.
- **Event Processing**: The program responds to specific events coded as 'R', 'U', 'A', 'D', 'W', 'S', 'F', 'T', 'M', 'P', representing various operations.

## Compiling and Execution

### Compilation:
Use the included Makefile with the command:
```
make
```

### Execution:
After compilation, execute the program with:
```
./cs240StreamingService path/to/input_file
```
Replace `path/to/input_file` with the path to the file containing the event list.

## Input Format
The input file should consist of lines formatted as per the commands described in `streaming_service.h`. Each line represents an event that triggers specific functionalities in the program.

Refer to the test files provided for examples.

