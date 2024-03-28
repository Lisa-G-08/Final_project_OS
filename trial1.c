#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// Process Control Block (PCB) structure
typedef struct {
    int pid;        // Process ID
    char* name;     // Process name
    int size;       // Size of the process
    int start_addr; // Starting address in memory
} PCB;

// Memory Block structure
typedef struct {
    int start_addr; // Starting address of the memory block
    int size;       // Size of the memory block
} MemoryBlock;

// Method to allocate memory to blocks as per Best fit algorithm 
void bestFit(MemoryBlock memory[], int m, PCB processes[], int n) 
{ 
    // Stores block id of the block allocated to a process 
    int allocation[n]; 
  
    // Initially no block is assigned to any process 
    for (int i = 0; i < n; i++) 
        allocation[i] = -1; 
  
    // pick each process and find suitable blocks 
    // according to its size ad assign to it 
    for (int i = 0; i < n; i++) 
    { 
        // Find the best fit block for current process 
        int bestIdx = -1; 
        for (int j = 0; j < m; j++) 
        { 
            if (memory[j].size >= processes[i].size) 
            { 
                if (bestIdx == -1) 
                    bestIdx = j; 
                else if (memory[j].size < memory[bestIdx].size) 
                    bestIdx = j; 
            } 
        } 
  
        // If we could find a block for current process 
        if (bestIdx != -1) 
        { 
            // allocate block j to p[i] process 
            allocation[i] = bestIdx; 
  
            // Reduce available memory in this block. 
            memory[bestIdx].size -= processes[i].size; 
        } 
    } 
    printf("For best fit:\n");
    printf("\nProcess No.\tProcess Size\tBlock no.\n"); 
    for (int i = 0; i < n; i++) 
    { 
        printf(" %d\t\t%d\t\t", processes[i].pid, processes[i].size); 
        if (allocation[i] != -1) 
            printf("%d", allocation[i] + 1); 
        else
            printf("Not Allocated"); 
        printf("\n"); 
    } 
} 

void worstFit(MemoryBlock memory[], int m, PCB processes[], int n)
{
    // Stores block id of the block allocated to a process
    int allocation[n];

    // Initially no block is assigned to any process
    memset(allocation, -1, sizeof(allocation));

    // pick each process and find suitable blocks
    // according to its size ad assign to it
    for (int i = 0; i < n; i++)
    {
        // Find the worst fit block for current process
        int wstIdx = -1;
        for (int j = 0; j < m; j++)
        {
            if (memory[j].size >= processes[i].size)
            {
                if (wstIdx == -1 || memory[wstIdx].size < memory[j].size)
                    wstIdx = j;
            }
        }

        // If we could find a block for current process
        if (wstIdx != -1)
        {
            // allocate block j to p[i] process
            allocation[i] = wstIdx;

            // Reduce available memory in this block.
            memory[wstIdx].size -= processes[i].size;
        }
    }
    
    printf("For worst fit:\n");
    printf("\nProcess No.\tProcess Size\tBlock no.\n");
    for (int i = 0; i < n; i++)
    {
        printf(" %d\t\t%d\t\t", processes[i].pid, processes[i].size);
        if (allocation[i] != -1)
            printf("%d", allocation[i] + 1);
        else
            printf("Not Allocated");
        printf("\n");
    }
}

void firstFit(MemoryBlock memory[], int m, PCB processes[], int n){
    int i, j;
    int allocation[n]; 
    for(i = 0; i < n; i++) { 
        allocation[i] = -1; 
    } 

    for (i = 0; i < n; i++) { 
        for (j = 0; j < m; j++) { 
            if (memory[j].size >= processes[i].size) { 
                // allocating block j to the ith process S
                allocation[i] = j; 
  
                // Reduce available memory in this block. 
                memory[j].size -= processes[i].size; 
  
                break;    //go to the next process in the queue 
            } 
        } 
    } 
    
    printf("For first fit:\n");
    printf("\nProcess No.\tProcess Size\tBlock no.\n"); 
    for (int i = 0; i < n; i++) 
    { 
        printf(" %i\t\t\t", processes[i].pid); 
        printf("%i\t\t\t\t", processes[i].size); 
        if (allocation[i] != -1) 
            printf("%i", allocation[i] + 1); 
        else
            printf("Not Allocated"); 
        printf("\n"); 
    } 
} 

void nextFit(MemoryBlock memory[], int m, PCB processes[], int n){
    // Stores block id of the block allocated to a process
    // Initially no block is assigned to any process
    int allocation[n];
    for (int i = 0; i < n; i++)
        allocation[i] = -1;

    int j = 0;
    int lastAllocatedBlock = 0; // Keeps track of the last allocated block

    // pick each process and find suitable blocks
    // according to its size and assign to it
    for (int i = 0; i < n; i++)
    {
        // Start from the last allocated block
        j = lastAllocatedBlock;

        // Do not start from beginning
        while (j < m)
        {
            if (memory[j].size >= processes[i].size)
            {
                // allocate block j to p[i] process
                allocation[i] = j;

                // Reduce available memory in this block.
                memory[j].size -= processes[i].size;

                // Update the last allocated block
                lastAllocatedBlock = j + 1;

                break;
            }

            j++;
        }

        // If we couldn't find a block for the current process, start from the beginning
        if (allocation[i] == -1)
        {
            j = 0;
            while (j < lastAllocatedBlock)
            {
                if (memory[j].size >= processes[i].size)
                {
                    // allocate block j to p[i] process
                    allocation[i] = j;

                    // Reduce available memory in this block.
                    memory[j].size -= processes[i].size;

                    // Update the last allocated block
                    lastAllocatedBlock = j + 1;

                    break;
                }

                j++;
            }
        }
    }

    printf("For next fit:\n");
    printf("Process No.\tProcess Size\tBlock no.\n");
    for (int i = 0; i < n; i++)
    {
        printf("\t%d\t\t%d\t\t", processes[i].pid, processes[i].size);
        if (allocation[i] != -1)
            printf("%d", allocation[i] + 1);
        else
            printf("Not Allocated");
        printf("\n");
    }
}

  
// Driver Method 
int main() 
{ 
    MemoryBlock memory[] = {
        {0, 100},
        {100, 500}, 
        {300, 200}, 
        {450, 300},
        {550, 600}
    };

    PCB processes[] = {
        {1, "Process A", 212, -1},
        {2, "Process B", 417, -1},
        {3, "Process C", 112, -1},
        {4, "Process D", 426, -1},
        {5, "Process E", 120, -1}
    };

    int m = sizeof(memory) / sizeof(memory[0]); 
    int n = sizeof(processes) / sizeof(processes[0]); 
  
    bestFit(memory, m, processes, n); 
    //worstFit(memory, m, processes, n);
    //firstFit(memory, m, processes, n);
    //nextFit(memory, m, processes, n);
  
    return 0 ; 
}
