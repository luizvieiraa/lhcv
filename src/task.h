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

    char arquivo_entrada[200];
    char arquivo_saida[200];
    int modo_append;
} Task;

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

pid_t iniciar_task(
    Task *task,
    char *diretorio_trabalho
);

int executar_task(
    Task *task,
    char *diretorio_trabalho
);

int definir_input(Task *task, char *arquivo);

int definir_output(
    Task *task,
    char *arquivo,
    int append
);

int executar_pipe(
    Task *tasks[],
    int quantidade,
    char *diretorio_trabalho
);

void liberar_tasks(Task tarefas[], int quantidade);
void informar_status_processo(pid_t pid, int status);

#endif
