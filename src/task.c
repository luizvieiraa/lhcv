#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "task.h"

static int copiar_string(char *destino, size_t capacidade, const char *origem,
                         const char *campo) {
    size_t tamanho = strlen(origem);
    if (tamanho >= capacidade) {
        printf("Erro: %s excede o limite de %zu caracteres.\n",
               campo, capacidade - 1);
        return -1;
    }
    memcpy(destino, origem, tamanho + 1);
    return 0;
}

void informar_status_processo(pid_t pid, int status) {
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("Processo %ld terminou com codigo %d.\n",
               (long)pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Processo %ld terminou pelo sinal %d.\n",
               (long)pid, WTERMSIG(status));
    }
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

    if (quantidade_tokens - 2 >= MAX_ARGS) {
        printf("Erro: task aceita no maximo %d argumentos incluindo o programa.\n",
               MAX_ARGS - 1);
        return;
    }

    Task *task = &tarefas[*quantidade];

    task->arquivo_entrada[0] = '\0';
    task->arquivo_saida[0] = '\0';
    task->modo_append = 0;

    if (copiar_string(task->nome, sizeof(task->nome), tokens[1], "nome da task") == -1 ||
        copiar_string(task->programa, sizeof(task->programa), tokens[2], "programa") == -1) {
        return;
    }

    task->quantidade_argumentos = 0;

    for (int i = 2; i < quantidade_tokens; i++) {
        task->argumentos[task->quantidade_argumentos] =
            malloc(strlen(tokens[i]) + 1);

        if (task->argumentos[task->quantidade_argumentos] == NULL) {
            printf("Erro: memória insuficiente.\n");
            for (int j = 0; j < task->quantidade_argumentos; j++) {
                free(task->argumentos[j]);
                task->argumentos[j] = NULL;
            }
            task->quantidade_argumentos = 0;
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

    informar_status_processo(pid, status);

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
                _exit(EXIT_FAILURE);
            }
        }

        if (task->arquivo_entrada[0] != '\0') {

            int fd = open(
                task->arquivo_entrada,
                O_RDONLY
            );

            if (fd == -1) {
                perror("open input");
                _exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2 input");
                close(fd);
                _exit(EXIT_FAILURE);
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
                _exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2 output");
                close(fd);
                _exit(EXIT_FAILURE);
            }

            close(fd);
        }

        execv(
            task->programa,
            task->argumentos
        );

        perror("execv");
        _exit(EXIT_FAILURE);
    }

    return pid;
}

int definir_input(Task *task, char *arquivo) {
    return copiar_string(task->arquivo_entrada, sizeof(task->arquivo_entrada),
                         arquivo, "caminho de entrada");
}

int definir_output(
    Task *task,
    char *arquivo,
    int append
) {
    if (copiar_string(task->arquivo_saida, sizeof(task->arquivo_saida),
                      arquivo, "caminho de saida") == -1) {
        return -1;
    }

    task->modo_append = append;

    return 0;
}

void liberar_tasks(Task tarefas[], int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        for (int j = 0; j < tarefas[i].quantidade_argumentos; j++) {
            free(tarefas[i].argumentos[j]);
            tarefas[i].argumentos[j] = NULL;
        }
        tarefas[i].quantidade_argumentos = 0;
    }
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
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return -1;
        }
    }

    pid_t pids[quantidade];

    for (int i = 0; i < quantidade; i++) {

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            for (int j = 0; j < quantidade - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            for (int j = 0; j < i; j++) {
                int status;
                if (waitpid(pids[j], &status, 0) != -1) {
                    informar_status_processo(pids[j], status);
                }
            }
            return -1;
        }

        if (pid == 0) {

            if (
                diretorio_trabalho != NULL &&
                diretorio_trabalho[0] != '\0'
            ) {

                if (chdir(diretorio_trabalho) == -1) {
                    perror("chdir");
                    _exit(EXIT_FAILURE);
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
                    _exit(EXIT_FAILURE);
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
                    _exit(EXIT_FAILURE);
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
            _exit(EXIT_FAILURE);
        }

        pids[i] = pid;
    }

    for (int i = 0; i < quantidade - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < quantidade; i++) {

        int status;

        pid_t resultado = waitpid(
                pids[i],
                &status,
                0
            );
        if (resultado == -1) {

            perror("waitpid");
        } else {
            informar_status_processo(pids[i], status);
        }
    }

    return 0;
}
