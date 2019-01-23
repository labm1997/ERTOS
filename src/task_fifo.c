#include <stdint.h>
#include "task_fifo.h"
#include "task.h"

int16_t task_fifo_put(task_fifo *fifo, task element){
    // Se passar limite da fifo retorna código de erro
    if(fifo->length >= FIFO_LIMIT) return -1;
    fifo->tail = (fifo->tail+1) % FIFO_LIMIT;
    // Copia struct passada para a fifo
    fifo->tasks[fifo->tail] = element;
    // Incrementa o tamanho da fifo
    fifo->length++;
    // Retorna ok
    return 0;
}

task task_fifo_get(task_fifo *fifo){
    // Se a fifo é vazia retorna uma tarefa de PID nulo
    if(fifo->length == 0) {
      task ret = {0};
      return ret;
    }
    // Calcula posição e incrementa head
    fifo->head = (fifo->head+1) % FIFO_LIMIT;
    // Decrementa tamanho da fifo
    fifo->length--;
    // Retorna a tarefa na posição calculada
    return fifo->tasks[fifo->head];
}
