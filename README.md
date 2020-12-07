## project0-2 
### Data Structures in Pintos Kernel

Before diving into Pintos projects, it is for practicing Pintos data structures - List, Hash table and Bitmap.
This libraries are originally located in "pintos/src/lib/kernel" and "pintos/src/lib/user".

This project is writing an interactive program that can check functionalities of list, hash table and bitmap in Pintos kernel.



## project1
### User Program (1)

There are no basic functions in original Pintos files. To run a user program, we have to implement those basic functions of OS: 1. Passing Arguments to User Stack, 2. Waiting a Child Process to exit, 3. Checking validity of User required Virtual Address to Use System Call.
Plus, there would be a exit message when a process exited.



## project2 
### User Program (2)

To use file system, we have to implement system calls about file system: create, remove, open, close, filesize, read, write, seek, tell



## project3
### Threads
Default scheduler used in pintos is a Round-Robin scheduler without considering priority and aging issue. In this project, we will upgrade the scheduler to consider both issues and also make alarm clock system call more efficient. To achieve efficiency, we have to convert original method of busy waiting
to sleep/awake technique. 
Furthermore, to make BSD scheduler, we have to implement complex calculation of priority using the values of RECENT CPU, NICE, LOAD AVERAGE at a proper clock tick. Fixed point operations are also needed.
