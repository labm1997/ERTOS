#ifndef TASK_H_
#define TASK_H_

#include <stdint.h>

#define DEFAULT_QUANTUM 100

typedef enum {
    READY,
    RUNNING,
    SLEEPING,
    DONE
} task_state;

typedef enum {
    LOW,
    MEDIUM,
    HIGH
} task_priority;

typedef struct {
    void (*entryPoint)(void);
    uint16_t *stackPointer;
    uint16_t pid;
    task_state state;
    task_priority priority;
    uint16_t quantum;
    uint16_t defaultQuantum;
} task;

#endif /* TASK_H_ */
