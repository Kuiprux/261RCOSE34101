#include "message_buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int shm_fd = -1;
void *memory_segment = NULL;

int init_buffer(MessageBuffer **buffer) {
    /*---------------------------------------*/
    /* TODO 1 : init buffer                  */
    /* (1) create POSIX shared memory        */
    /*     using shm_open()                  */
    /* (2) set size using ftruncate()        */
    /* (3) map memory using mmap()           */
    /* (4) initialize MessageBuffer fields   */

    if ((shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666)) == -1) return -1;

    if (ftruncate(shm_fd, sizeof(MessageBuffer)) == -1) return -1;

    memory_segment = mmap(NULL, sizeof(MessageBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (memory_segment == MAP_FAILED) return -1;

    *buffer = (MessageBuffer*)memory_segment;
    (*buffer)->message.sender_id = 0;
    (*buffer)->message.data = 0;
    (*buffer)->account_id = 0;
    (*buffer)->is_empty = 1;

    /* TODO 1 : END                          */
    /*---------------------------------------*/

    printf("init buffer\n");
    return 0;
}

int attach_buffer(MessageBuffer **buffer) {
    /*---------------------------------------*/
    /* TODO 2 : attach buffer                */
    /* do not consider "no buffer situation" */
    /* (1) open POSIX shared memory          */
    /*     using shm_open()                  */
    /* (2) map memory using mmap()           */
    /* (3) assign mapped address to *buffer  */

    if ((shm_fd = shm_open(SHM_NAME, O_RDWR, 0666)) == -1) return -1;

    memory_segment = mmap(NULL, sizeof(MessageBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (memory_segment == MAP_FAILED) return -1;

    *buffer = (MessageBuffer *)memory_segment;

    /* TODO 2 : END                          */
    /*---------------------------------------*/

    printf("attach buffer\n");
    printf("\n");
    return 0;
}

int detach_buffer(void) {
    if (munmap(memory_segment, sizeof(MessageBuffer)) == -1) {
        printf("munmap error!\n\n");
        return -1;
    }

    if (shm_fd != -1) close(shm_fd);

    printf("detach buffer\n\n");
    return 0;
}

int destroy_buffer(void) {
    if(shm_unlink(SHM_NAME) == -1) {
        printf("shm_unlink error!\n\n");
        return -1;
    }

    printf("destroy shared_memory\n\n");
    return 0;
}

int produce(MessageBuffer **buffer, int sender_id, int data, int account_id) {

    /*---------------------------------------*/
    /* TODO 3 : produce message              */
    /* write sender_id, data, and account_id */
    /* to the shared single-slot buffer      */

    if ((*buffer)->is_empty == 0) return -1;

    (*buffer)->message.sender_id = sender_id;
    (*buffer)->message.data = data;
    (*buffer)->account_id = account_id;
    (*buffer)->is_empty = 0;

    /* TODO 3 : END                          */
    /*---------------------------------------*/

    printf("produce message\n");

    return 0;
}

int consume(MessageBuffer **buffer, Message **message) {

    /*---------------------------------------*/
    /* TODO 4 : consume message              */
    /* read the message from shared memory   */
    /* and update the buffer state           */

    if ((*buffer)->is_empty == 1) return -1;

    *message = &((*buffer)->message);
    (*buffer)->is_empty = 1;
    
    /* TODO 4 : END                          */
    /*---------------------------------------*/
    return 0;
}
