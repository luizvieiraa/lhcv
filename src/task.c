#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

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

    task->arquivo_entrada[0] = '\0';
    task->arquivo_saida[0] = '\0';
    task->modo_append = 0;

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

int executar_task(
    Task *task,
    char *diretorio_trabalho
) {

    pid_t pid = iniciar_task(
        task,
        diretorio_trabalho
    );

    if (pid == -1) {
        return -1;
    }

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }

    return 0;
}

pid_t iniciar_task(
    Task *task,
    char *diretorio_trabalho
) {

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {

        if (
            diretorio_trabalho != NULL &&
            diretorio_trabalho[0] != '\0'
        ) {

            if (chdir(diretorio_trabalho) == -1) {
                perror("chdir");
                exit(EXIT_FAILURE);
            }
        }

        if (task->arquivo_entrada[0] != '\0') {

            int fd = open(
                task->arquivo_entrada,
                O_RDONLY
            );

            if (fd == -1) {
                perror("open input");
                exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2 input");
                close(fd);
                exit(EXIT_FAILURE);
            }

            close(fd);
        }

        if (task->arquivo_saida[0] != '\0') {

            int flags = O_WRONLY | O_CREAT;

            if (task->modo_append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            int fd = open(
                task->arquivo_saida,
                flags,
                0644
            );

            if (fd == -1) {
                perror("open output");
                exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2 output");
                close(fd);
                exit(EXIT_FAILURE);
            }

            close(fd);
        }

        execv(
            task->programa,
            task->argumentos
        );

        perror("execv");
        exit(EXIT_FAILURE);
    }

    return pid;
}

int definir_input(Task *task, char *arquivo) {
    strcpy(task->arquivo_entrada, arquivo);

    return 0;
}

int definir_output(
    Task *task,
    char *arquivo,
    int append
) {
    strcpy(task->arquivo_saida, arquivo);

    task->modo_append = append;

    return 0;
}

int executar_pipe(
    Task *tasks[],
    int quantidade,
    char *diretorio_trabalho
) {

    if (quantidade < 2) {
        printf(
            "Erro: pipe precisa de pelo menos duas tarefas.\n"
        );
        return -1;
    }

    int pipes[quantidade - 1][2];

    for (int i = 0; i < quantidade - 1; i++) {

        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    pid_t pids[quantidade];

    for (int i = 0; i < quantidade; i++) {

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {

            if (
                diretorio_trabalho != NULL &&
                diretorio_trabalho[0] != '\0'
            ) {

                if (chdir(diretorio_trabalho) == -1) {
                    perror("chdir");
                    exit(EXIT_FAILURE);
                }
            }

            if (i > 0) {

                if (
                    dup2(
                        pipes[i - 1][0],
                        STDIN_FILENO
                    ) == -1
                ) {

                    perror("dup2 input pipe");
                    exit(EXIT_FAILURE);
                }
            }

            if (i < quantidade - 1) {

                if (
                    dup2(
                        pipes[i][1],
                        STDOUT_FILENO
                    ) == -1
                ) {

                    perror("dup2 output pipe");
                    exit(EXIT_FAILURE);
                }
            }

            for (
                int j = 0;
                j < quantidade - 1;
                j++
            ) {

                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execv(
                tasks[i]->programa,
                tasks[i]->argumentos
            );

            perror("execv");
            exit(EXIT_FAILURE);
        }

        pids[i] = pid;
    }

    for (int i = 0; i < quantidade - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < quantidade; i++) {

        int status;

        if (
            waitpid(
                pids[i],
                &status,
                0
            ) == -1
        ) {

            perror("waitpid");
        }
    }

    return 0;
}