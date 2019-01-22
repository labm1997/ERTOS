#ifndef TASK_FIFO_H_
#define TASK_FIFO_H_

#include "task.h"

#define FIFO_LIMIT 10

typedef struct {
    uint16_t head, tail;
    uint16_t length;
    task tasks[FIFO_LIMIT];
} task_fifo;

int16_t task_fifo_put(task_fifo *fifo, task element);
task task_fifo_get(task_fifo *fifo);

#endif /* TASK_FIFO_H_ */
