Memory Allocation Simulator

This software simulates memory allocation using the Best Fit method. Users can specify the memory size they want to simulate and the number of processes needing memory allocation. The program then allocates memory segments, for each process using either the First Fit or Best Fit method, depending on memory.

How it Functions

1. Memory Initialization; The program sets up memory blocks based on the size dividing them into fixed size blocks (16MB each). It utilizes shared memory for communication between processes.

2. Process Generation; Users provide the number of processes they wish to simulate. The program generates sizes for these processes.

3. Memory Allocation; Each process is. Memory allocation takes place. The First Fit method is used for allocating memory to the process while subsequent processes use the Best Fit method. Details of memory allocation are displayed, including process ID, segment ID, segment size allocated block ID, block size block size after allocation and block start address.

4. Cleanup; In case of program interruption by the user (, via Ctrl+C) cleanup procedures are executed to release shared memory.

YouTube link: https://www.youtube.com/watch?v=FZ_glq6HOS0
