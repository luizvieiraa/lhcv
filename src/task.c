#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "task.h"

void cadastrar_task(
    Task tarefas[],
    int *quantidade,
    char *tokens[],
    int quantidade_tokens
) {

    if (*quantidade >= MAX_TASKS) {
        printf("Erro: limite de tarefas atingido.\n");
        return;
    }

    if (quantidade_tokens < 3) {
        printf("Erro: uso correto: task <nome> <programa> [argumentos...]\n");
        return;
    }

    Task *task = &tarefas[*quantidade];

    strcpy(task->nome, tokens[1]);
    strcpy(task->programa, tokens[2]);

    task->quantidade_argumentos = 0;

    for (int i = 2; i < quantidade_tokens; i++) {

        task->argumentos[task->quantidade_argumentos] =
            malloc(strlen(tokens[i]) + 1);

        if (task->argumentos[task->quantidade_argumentos] == NULL) {
            printf("Erro: memória insuficiente.\n");
            return;
        }

        strcpy(
            task->argumentos[task->quantidade_argumentos],
            tokens[i]
        );

        task->quantidade_argumentos++;
    }

    task->argumentos[task->quantidade_argumentos] = NULL;

    (*quantidade)++;

    printf("Task '%s' cadastrada.\n", task->nome);
}


Task *buscar_task(
    Task tarefas[],
    int quantidade,
    char *nome
) {

    for (int i = 0; i < quantidade; i++) {

        if (strcmp(tarefas[i].nome, nome) == 0) {
            return &tarefas[i];
        }
    }

    return NULL;
}

int executar_task(Task *task) {

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {

        execv(task->programa, task->argumentos);

        perror("execv");
        exit(EXIT_FAILURE);
    }

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }

    return 0;
}