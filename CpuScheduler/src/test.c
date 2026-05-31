#include "job.h"
#include "data_structures.h"
#include <stdio.h>

#define JOB_QUEUE_SIZE 100

data_structure test_queue = { (Job[JOB_QUEUE_SIZE]) { 0 }, 0, 0, JOB_QUEUE_SIZE };

void check_data_structures()
{
    printf("max heap: \n");
    data_structure_reset(&test_queue);
    for (int i = 0; i < JOB_QUEUE_SIZE + 10; i++) {
        Job job = { rand() % 100, 0 };
        int result = tree_insert(&test_queue, &job);
        if (!result)
            printf("Failed to push %d\n", i);
    }
    for (int i = 0; i < JOB_QUEUE_SIZE + 10; i++) {
        Job job;
        int result = tree_pop_max(&test_queue, &job);
        if (result)
            printf("Popped: %d\n", job.first_priority);
        else
            printf("Failed to pop\n");
    }

    printf("queue: \n");
    data_structure_reset(&test_queue);
    for (int i = 0; i < JOB_QUEUE_SIZE + 10; i++) {
        Job job = { i, 0 };
        int result = round_queue_push(&test_queue, &job);
        if (!result)
            printf("Failed to push %d\n", i);
    }
    for (int i = 0; i < JOB_QUEUE_SIZE + 10; i++) {
        Job job;
        int result = round_queue_pop(&test_queue, &job);
        if (result)
            printf("Popped: %d\n", job.first_priority);
        else
            printf("Failed to pop\n");
    }

}