#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

static char *duplicar_string(const char *texto) {
    char *copia = malloc(strlen(texto) + 1);

    if (copia != NULL) {
        strcpy(copia, texto);
    }

    return copia;
}

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
            duplicar_string(tokens[i]);

        task->quantidade_argumentos++;
    }

    task->argumentos[task->quantidade_argumentos] = NULL;

    (*quantidade)++;

    printf("Task '%s' cadastrada.\n", task->nome);
}