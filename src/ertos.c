#include <msp430.h> 
#include <stdint.h>
#include "clock.h"
#include "task.h"
#include "task_fifo.h"

#define TASKS_LIMIT 10
#define SCHEDULER_STACK 0x2500

// Ponteiro para a pilha do escalonador
uint16_t *scheduler_stack = (uint16_t *)SCHEDULER_STACK;
// Tarefa atual sendo executada
task currentTask;
// Contador do PID 
uint16_t pid_counter = 0;
// Tarefas registradas
uint16_t registeredTasks = 0;

// Filas de prioridade
task_fifo high_priority   = {0};
task_fifo medium_priority = {0};
task_fifo low_priority    = {0};

/*
  Insere uma tarefa na pilha adequada para sua prioridade
 */
void task_fifo_priority_insert(task element){
  switch(element.priority){
      case HIGH:
          task_fifo_put(&high_priority, element);
          break;
      case MEDIUM:
          task_fifo_put(&medium_priority, element);
          break;
      case LOW:
          task_fifo_put(&low_priority, element);
          break;
  }
}

/*
  Encontra a próxima tarefa a ser executada por meio das filas 
 */
task get_next_task(){
    task ret;
    // Procura uma tarefa de alta prioridade
    ret = task_fifo_get(&high_priority);
    // Se tiver encontrado retorna
    if(ret.pid != 0) return ret;
    // Procura uma tarefa de média prioridade
    ret = task_fifo_get(&medium_priority);
    // Se tiver encontrado retorna
    if(ret.pid != 0) return ret;
    // Procura uma tarefa de baixa prioridade
    ret = task_fifo_get(&low_priority);
    // Se tiver encontrado retorna
    if(ret.pid != 0) return ret;
    // Retorna uma tarefa de pid nulo caso não encontre
    ret.pid = 0;
    return ret;
}

/*
  Escalonador de tarefas
 */
void priority_scheduler(){
  // Verifica se o quantum da tarefa atual terminou
  if(!currentTask.quantum){
    // Redefine quantum da tarefa
    currentTask.quantum = currentTask.defaultQuantum;
    // Busca próxima tarefa
    task nextTask = get_next_task();
    // Salva tarefa atual na fifo adequada a sua prioridade
    task_fifo_priority_insert(currentTask);
    // Faz a próxima tarefa ser a tarefa atual via copia
    currentTask = nextTask;
  }
  // Decrementa quantum da tarefa atual
  else currentTask.quantum--;
}

/*
  Rotina de interrupção do WDT para fazer o escalonamento de tarefas
 */
__attribute__((naked))
__attribute__((__interrupt__(WDT_VECTOR)))
static void WDTISR(){
  // Salva o contexto
  asm("PUSHM.A #12,R15");
  // Salva o stack pointer da tarefa
  asm("MOVX.A R1,%0" : "=m" (currentTask.stackPointer));
  // Move o ponteiro da pilha para o escalonador
  asm("MOVX.A %0,R1" : : "m" (scheduler_stack));
  
  // Chama o escalonador de prioridades
  priority_scheduler();
  
  // Salva o ponteiro da pilha do escalonador
  //asm("MOVX.A R1,%0" : "=m" (scheduler_stack));
  // Restaura o ponteiro da pilha para a nova tarefa
  asm("MOVX.A %0,R1" : : "m" (currentTask.stackPointer));
  // Restaura o contexto dessa nova tarefa
  asm("POPM.A #12,R15");
  asm("RETI");
}


/*
  Registra uma tarefa dado o entryPoint e a prioridade
 */
void registerTask(void (*entryPoint)(void), task_priority priority){
    uint16_t *stackPointer;
    task tmp;
    // Evita ultrapassar vetor de tarefa
    if(registeredTasks < TASKS_LIMIT){
        // Calcula posição na memória para nova tarefa
        stackPointer = (uint16_t *)(0x2800 + 0x80 * registeredTasks);
        // Salva na pilha PC[15:0]
        *(stackPointer-1) = (uint16_t)((int32_t)entryPoint & 0x0ffff);
        // Salva na pilha PC[19:16] | SR[11:0]
        *(stackPointer-2) = (uint16_t)(((int32_t)entryPoint >> 4) & 
0xf000) | GIE;

        // Salva o ponto de entrada da tarefa em sua estrutura
        tmp.entryPoint = entryPoint;
        // Salva o ponteiro da pilha na sua estrutura (26 são espaços alocados para os registradores R4:R15 e PC+SR)
        tmp.stackPointer = stackPointer-26;
        
        // Define as propriedades do processo
        tmp.priority = priority;
        tmp.pid = pid_counter++;
        tmp.state = READY;
        tmp.quantum = DEFAULT_QUANTUM;
        tmp.defaultQuantum = DEFAULT_QUANTUM;

        // Coloca na fila adequada
        task_fifo_priority_insert(tmp);

        registeredTasks++;
    }
}

/*
  Inicializa o ERTOS
 */
void startERTOS(){

    // Configura os relógios
    clockInit();

    // Não há tarefas registradas
    if(!registeredTasks) return;

    // Configura o watchdog 32.768ms
    WDTCTL = WDTPW | WDTSSEL__ACLK | WDTTMSEL | WDTIS_6;
    SFRIE1 |= WDTIE;
    SFRIFG1 &= ~WDTIFG;
    
    // Procura primeira tarefa a ser executada
    currentTask = get_next_task();

    // Define o ponteiro da pilha para a primeira tarefa
    uint16_t *taskStackPointer = (uint16_t *) 
currentTask.stackPointer+26;
    asm("MOVX.A %0,R1" : : "m" (taskStackPointer));

    __enable_interrupt();

    // Chama primeira tarefa
    currentTask.entryPoint();

}

void task1(){
    volatile uint32_t i;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
    while(1){
        i = 25000;
        while(i--);
        P4OUT ^= BIT7;
    }
}

void task2(){
    volatile uint32_t i;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    while(1){
        i = 25000;
        while(i--);
        P1OUT ^= BIT0;
    }
}

int main(void)
{
  registerTask(task1, HIGH);
  registerTask(task2, LOW);
  startERTOS();
  while(1);
}

