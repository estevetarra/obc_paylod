#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define CIRC_BUFF_TOTAL_SIZE    40 * 1024

/**
 * @brief Circular buffer handler.
 *
 * Each circ_buff structure contains pointer to a memory region,
 * amount of elements, element size and total queue size
 * finally pointers for write/read and the amount of items in the queue
 */
typedef struct circ_buff_s {
    void *              data;
    uint16_t             element_size;
    uint16_t             element_count;
    volatile uint16_t    queued_items;
    volatile uint16_t    read_ptr;
    volatile uint16_t    write_ptr;
    uint32_t             queue_size;
}circ_buff_t;

/**
 * @brief Initilizes a queue, executed only once per handler/queue
 * 
 * It points the handler data towards a memory region (malloc-like), 
 * initializes internal variables and computes the total queue size
 *
 * In case of funciton fail, handler->data will point towards NULL
 *
 * @param[in/out] The handler to be filled and initialised
 * @param[in] Element size, can be a byte, a float, a struct...
 * @param[in] Amount of elements inside the queue (1, 2 , 100...)
 * @return void
 *
 */
void queue_init(circ_buff_t * handler, uint16_t element_size, uint16_t element_count);

/**
 * @brief Returns if the queue pointed by handler is full
 *
 * @param[in] The handler to the queue
 * @returns True if the queue is full, false otherwise
 */
bool is_full(circ_buff_t * handler);

/**
 * @brief Returns if the queue pointed by handler is empty
 *
 * @param[in] The handler to the queue
 * @returns True if the queue is empty, false otherwise
 */
bool is_empty(circ_buff_t * handler);

/**
 * @brief Gets first item from the queue if any
 *
 * @param[in] The handler to the queue
 * @param[out] Pointer to a object of the size of one element from the queue
 *  an object will be drained from the queue as a FIFO
 * @returns True if the call success, false otherwise
 */
bool dequeue(circ_buff_t * handler, void * val);

/**
 * @brief Puts an item into the queue if enough space available
 *
 * @param[in] The handler to the queue
 * @param[in] Pointer to a object of the size of one element from the queue
 *     the object will be put into the queue as a FIFO
 * @returns True if the call success, false otherwise
 */
bool enqueue(circ_buff_t * handler, void * val);

/**
 * @brief Returns amount of available space in element size
 *
 * @param[in] The handler to the queue
 * @returns Amount of space for items (item sized)
 */
uint16_t available_space(circ_buff_t * handler);

/**
 * @brief Returns amount of available items the queue in element size
 *
 * @param[in] The handler to the queue
 * @returns Amount of items in the queue
 */
uint16_t available_items(circ_buff_t * handler);

#endif
