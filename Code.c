// Memory Allocation Simulation


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define SIZE_OF_KB 1024

typedef struct {
    int id;
    int size;
    char* start_address;
} MBBlock;

typedef struct {
    int process_id;
    int segment_id;
    int block_id;
    int size;
    char* start_address;
} Segment;

typedef struct {
    int pid;
    int size;
    Segment* segments;
} PCB;

int* generate_values(int n, int m);
int* determine_segment_sizes(int process_size, int block_sizes);
MBBlock* bestFit(MBBlock *blocks, int num_blocks, char* mem_block, int total_size_MB, PCB process, int segment_num);
MBBlock* memory_allocation(MBBlock *blocks, int num_blocks, char* mem_block, int total_size_MB, PCB process, int segment_num);
void cleanup(int sig);
void print_memory_overview(MBBlock *blocks, int num_blocks);

int main() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGQUIT, cleanup);

    int memory_size;
    int block_sizes = 16;
    int num_blocks;
    int num_processes;

    do {
        printf("Enter the size of your preferred memory divisible by 16 (96 - 1024 MB): ");
        scanf("%d", &memory_size);
    } while (memory_size < 96 || memory_size > 1024 || memory_size % 16 != 0);

    num_blocks = memory_size / block_sizes;

    char* mem_block = (char*)malloc(memory_size * 1024 * 1024);
    if (mem_block == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    key_t key = ftok(".", 'a');
    int shmid = shmget(key, num_blocks * sizeof(MBBlock), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    MBBlock* blocks = (MBBlock*)shmat(shmid, NULL, 0);
    if (blocks == (MBBlock*)-1) {
        perror("shmat");
        return 1;
    }

    char* current_address = mem_block;
    for (int i = 0; i < num_blocks; i++) {
        blocks[i].id = i + 1;
        blocks[i].size = block_sizes;
        blocks[i].start_address = current_address;
        current_address += block_sizes * 1024 * 1024;
    }

    printf("Memory Overview prior to any allocation:\n");
    print_memory_overview(blocks, num_blocks);

    do {
        printf("Enter the number of processes (between 5 and 10): ");
        scanf("%d", &num_processes);
    } while (num_processes < 5 || num_processes > 10);

    PCB process[num_processes];

    srand(time(NULL));
    int* process_sizes = generate_values(num_processes, memory_size);

    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Fork failed\n");
            shmdt(blocks);
            shmctl(shmid, IPC_RMID, NULL);
            free(mem_block);
            return 1;
        } else if (pid == 0) {
            process[i].pid = getpid();
            process[i].size = process_sizes[i];
            int num_segments = process[i].size / 16 + (process[i].size % 16 != 0);
            int* segments_sizes = determine_segment_sizes(process[i].size, num_segments);
            process[i].segments = (Segment*)malloc(num_segments * sizeof(Segment));
            for (int j = 0; j < num_segments; j++) {
                process[i].segments[j].process_id = getpid();
                process[i].segments[j].segment_id = j + 1;
                process[i].segments[j].size = segments_sizes[j] * SIZE_OF_KB;
            }

            if (i == 0) {
                blocks = memory_allocation(blocks, num_blocks, mem_block, memory_size, process[i], num_segments);
            } else {
                blocks = bestFit(blocks, num_blocks, mem_block, memory_size, process[i], num_segments);
            }
            printf("Memory Overview after allocation:\n");
            print_memory_overview(blocks, num_blocks);
            shmdt(blocks);
            exit(0);
        } else {
            wait(NULL);
        }
    }

    shmdt(blocks);
    shmctl(shmid, IPC_RMID, NULL);
    free(mem_block);

    return 0;
}

int* generate_values(int n, int m) {
    int* values = (int*)malloc(n * sizeof(int));
    int sum = 0;

    for (int i = 0; i < n - 1; i++) {
        values[i] = rand() % (m - sum - (n - i - 1)) + 1;
        sum += values[i];
    }

    values[n - 1] = m - sum;

    return values;
}


int* determine_segment_sizes(int process_size, int num_segments) {
    int* segment_sizes = (int*)malloc(num_segments * sizeof(int));
    if (segment_sizes == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    int remaining_size = process_size;
    for (int i = 0; i < num_segments - 1; i++) {
        segment_sizes[i] = 16;
        remaining_size -= segment_sizes[i];
    }
    segment_sizes[num_segments - 1] = remaining_size;

    return segment_sizes;
}

MBBlock* bestFit(MBBlock *blocks, int num_blocks, char* mem_block, int total_size_MB, PCB process, int segment_num) {
    for (int i = 0; i < segment_num; i++) {
        MBBlock* best_fit_block = NULL;
        int segment_size_MB = process.segments[i].size / SIZE_OF_KB;
        int best_fit_size_diff = total_size_MB;
        for (int j = 0; j < num_blocks; j++) {
            if (blocks[j].size >= segment_size_MB) {
                int size_diff = blocks[j].size - segment_size_MB;
                if (size_diff < best_fit_size_diff) {
                    best_fit_size_diff = size_diff;
                    best_fit_block = &blocks[j];
                }
            }
        }

        if (best_fit_block != NULL) {
            int initial_block_size = best_fit_block->size;
            best_fit_block->size -= segment_size_MB;
            if (best_fit_block->size < 0) {
                best_fit_block->size = 0;
            }
            
            process.segments[i].block_id = best_fit_block->id;
            process.segments[i].start_address = best_fit_block->start_address;
            best_fit_block->start_address += segment_size_MB * SIZE_OF_KB * SIZE_OF_KB; 
            printf("Memory Allocation Details for process segment:\n");
            printf("%-14s%-13s%-18s%-11s%-23s%-30s%-25s\n", "Process No.", "Segment No.", "Segment Size", "Block No.", "Initial Block Size", "Block Size After Allocation", "Block Start Address");
            printf("%-14d%-13d%-18d %-11d%-23d %-30d %-25p\n", process.pid, process.segments[i].segment_id, process.segments[i].size, best_fit_block->id, initial_block_size, best_fit_block->size, process.segments[i].start_address);
            printf("\n");
        }
        else{
            printf("%-14d%-13d%-14d %-11s%-23s%-30s%-25s\n", process.pid, process.segments[i].segment_id, process.segments[i].size, "N/A", "N/A", "N/A", "N/A");
        }
    }
    return blocks;
}

MBBlock* memory_allocation(MBBlock *blocks, int num_blocks, char* mem_block, int total_size_MB, PCB process, int segment_num) {
    int total_available_memory_MB = 0;
    for (int k = 0; k < num_blocks; k++) {
        total_available_memory_MB += blocks[k].size;
    }

    if (total_available_memory_MB < total_size_MB) {
        printf("Switching to Best Fit algorithm for memory allocation...\n");
        return bestFit(blocks, num_blocks, mem_block, total_size_MB, process, segment_num);
    }

    for (int i = 0; i < segment_num; i++) {
        MBBlock* random_block = NULL;
        int segment_size_MB = process.segments[i].size / SIZE_OF_KB;
        int j = rand() % num_blocks;
        random_block = &blocks[j];

        if (random_block->size >= segment_size_MB) {
            int initial_block_size = random_block->size;
            random_block->size -= segment_size_MB;
            if (random_block->size < 0) {
                random_block->size = 0;
            }
            
            process.segments[i].block_id = random_block->id;
            process.segments[i].start_address = random_block->start_address;
            random_block->start_address += segment_size_MB * SIZE_OF_KB * SIZE_OF_KB;
            printf("Memory Allocation Details for process segment:\n");
            printf("%-14s%-13s%-18s%-11s%-23s%-30s%-25s\n", "Process No.", "Segment No.", "Segment Size(KB)", "Block No.", "Initial Block Size(MB)", "Block Size After Allocation(MB)", "Block Start Address");
            printf("%-14d%-13d%-18d %-11d%-23d %-30d %-25p\n", process.pid, process.segments[i].segment_id, process.segments[i].size, random_block->id, initial_block_size, random_block->size, process.segments[i].start_address);
            printf("\n");
        }   
    }
    return blocks;
}

void cleanup(int sig) {
    printf("Cleaning up...\n");
    key_t key = ftok(".", 'a');
    int shmid = shmget(key, 0, 0);
    if (shmid >= 0) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    exit(1);
}

void print_memory_overview(MBBlock *blocks, int num_blocks) {

    printf("%-11s%-18s%-25s\n", "Block No.", "Block Size(MB)", "Start Address");
    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i].size > 0) {
            printf("%-11d%-18d%-25p\n", blocks[i].id, blocks[i].size, blocks[i].start_address);
        } else {
            printf("%-11d%-18d%-25s\n", blocks[i].id, blocks[i].size, "allocated");
        }
    }
    printf("\n");
}
