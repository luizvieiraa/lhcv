#ifndef TASK_H
#define TASK_H

#include <sys/types.h>

#define MAX_TASKS 50
#define MAX_ARGS 20

typedef struct {

    char nome[50];
    char programa[100];
    char *argumentos[MAX_ARGS];
    int quantidade_argumentos;

}Task;

void cadastrar_task(
    Task tarefas[],
    int *quantidade,
    char *tokens[],
    int quantidade_tokens
);

Task *buscar_task(
    Task tarefas[],
    int quantidade,
    char *nome
);

int executar_task(Task *Task);
pid_t iniciar_task(Task *task);

#endif