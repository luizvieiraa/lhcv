#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include "job.h"

#include "task.h"

#define MAX_INPUT 500
#define MAX_TOKENS 30
#define MAX_WORKDIR 500

int main(int argc, char *argv[]) {

    FILE *entrada = stdin;
    int modo_workflow = 0;

    if(argc > 2){

        printf(
            "Erro: uso correto: ./processflow [workflowFile]\n"
        );

        return 1;
    }

    if (argc == 2) {

    entrada = fopen(
        argv[1],
        "r"
    );

    if (entrada == NULL) {

        perror("Erro ao abrir workflow");

        return 1;
    }

    modo_workflow = 1;
}

    Task tarefas[MAX_TASKS];
    int quantidade_tasks = 0;

    Job jobs[MAX_JOBS];
    int quantidade_jobs = 0;

    char diretorio_trabalho[MAX_WORKDIR] = "";

    char comando[MAX_INPUT];

    while (1) {

        atualizar_jobs(
            jobs,
            quantidade_jobs
        );

        if (!modo_workflow) {

            printf("processflow> ");
            fflush(stdout);
            }

        if (fgets(comando, sizeof(comando), entrada) == NULL) {
            break;
        }

        if (strchr(comando, '\n') == NULL) {
            int proximo = fgetc(entrada);
            if (proximo != '\n' && proximo != EOF) {
                while (proximo != '\n' && proximo != EOF) {
                    proximo = fgetc(entrada);
                }
                printf("Erro: linha de comando excede %d caracteres.\n",
                       MAX_INPUT - 1);
                continue;
            }
        }

        if (modo_workflow) {

            printf("%s", comando);

            /*
            * Caso a última linha do arquivo
            * não tenha \n.
            */
            if (
                strlen(comando) > 0 &&
                comando[strlen(comando) - 1] != '\n'
            ) {

                printf("\n");
            }
        }

        comando[strcspn(comando, "\r\n")] = '\0';

        if (strcmp(comando, "exit") == 0) {
            break;
        }

        if (strlen(comando) == 0) {
            continue;
        }

        char *tokens[MAX_TOKENS];
        int quantidade_tokens = 0;

        char *token = strtok(comando, " \t");

        while (
            token != NULL &&
            quantidade_tokens < MAX_TOKENS
        ) {

            tokens[quantidade_tokens] = token;
            quantidade_tokens++;

            token = strtok(NULL, " \t");
        }

        if (token != NULL) {
            printf("Erro: limite de %d tokens excedido.\n", MAX_TOKENS);
            continue;
        }

        if (quantidade_tokens == 0) {
            continue;
        }

        if (strcmp(tokens[0], "task") == 0) {

            cadastrar_task(
                tarefas,
                &quantidade_tasks,
                tokens,
                quantidade_tokens
            );
        }

        else if (strcmp(tokens[0], "input") == 0) {

            if (quantidade_tokens != 3) {

                printf(
                    "Erro: uso correto: "
                    "input <tarefa> <arquivo>\n"
                );

                continue;
            }

            Task *task = buscar_task(
                tarefas,
                quantidade_tasks,
                tokens[1]
            );

            if (task == NULL) {

                printf(
                    "Erro: task '%s' não encontrada.\n",
                    tokens[1]
                );

                continue;
            }

            definir_input(
                task,
                tokens[2]
            );
        }

        else if (strcmp(tokens[0], "output") == 0) {

            if (quantidade_tokens != 3) {

                printf(
                    "Erro: uso correto: "
                    "output <tarefa> <arquivo>\n"
                );

                continue;
            }

            Task *task = buscar_task(
                tarefas,
                quantidade_tasks,
                tokens[1]
            );

            if (task == NULL) {

                printf(
                    "Erro: task '%s' não encontrada.\n",
                    tokens[1]
                );

                continue;
            }

            definir_output(
                task,
                tokens[2],
                0
            );
        }


        else if (strcmp(tokens[0], "append") == 0) {

            if (quantidade_tokens != 3) {

                printf(
                    "Erro: uso correto: "
                    "append <tarefa> <arquivo>\n"
                );

                continue;
            }

            Task *task = buscar_task(
                tarefas,
                quantidade_tasks,
                tokens[1]
            );

            if (task == NULL) {

                printf(
                    "Erro: task '%s' não encontrada.\n",
                    tokens[1]
                );

                continue;
            }

        
            definir_output(
                task,
                tokens[2],
                1
            );
        }

        else if (strcmp(tokens[0], "workdir") == 0) {

            if (quantidade_tokens != 2) {

                printf(
                    "Erro: uso correto: "
                    "workdir <diretorio>\n"
                );

                continue;
            }

            struct stat info;

    
            if (stat(tokens[1], &info) != 0) {

                printf(
                    "Erro: diretório '%s' não existe.\n",
                    tokens[1]
                );

                continue;
            }


            if (!S_ISDIR(info.st_mode)) {

                printf(
                    "Erro: '%s' não é um diretório.\n",
                    tokens[1]
                );

                continue;
            }

            if (strlen(tokens[1]) >= sizeof(diretorio_trabalho)) {
                printf("Erro: caminho do workdir e muito longo.\n");
                continue;
            }
            memcpy(diretorio_trabalho, tokens[1], strlen(tokens[1]) + 1);
        }

        else if (strcmp(tokens[0], "jobs") == 0) {

            if (quantidade_tokens != 1) {
                printf("Erro: uso correto: jobs\n");
                continue;
            }

            listar_jobs(
                jobs,
                quantidade_jobs
            );
        }

        else if(strcmp(tokens[0], "start") == 0){

            if (quantidade_tokens != 2) {
                 printf("Erro: uso correto: start <tarefa>\n");
                 continue;
            }

        Task *task = buscar_task(
        tarefas,
        quantidade_tasks,
        tokens[1]
    );

            if (task == NULL) {

                printf(
                    "Erro: task '%s' não encontrada.\n",
                    tokens[1]
                );

                continue;
            }

            if (quantidade_jobs >= MAX_JOBS) {
                printf("Erro: limite maximo de jobs atingido.\n");
                continue;
            }

        pid_t pid = iniciar_task(
            task,
            diretorio_trabalho
        );

            if (pid == -1) {
                continue;
            }

        int job_id = adicionar_job(
            jobs,
            &quantidade_jobs,
            pid,
            task -> nome
        );

            if (job_id == -1) {
                continue;
            }

        printf(
            "[%d] %d\n",
            job_id,
            pid
        );
        }

        else if (strcmp(tokens[0], "wait") == 0) {

            if (quantidade_tokens != 2) {
                printf("Erro: uso correto: wait <jobId>\n");
                continue;
            }

            errno = 0;
            char *fim = NULL;
            long valor = strtol(tokens[1], &fim, 10);
            if (errno == ERANGE || fim == tokens[1] || *fim != '\0' ||
                valor <= 0 || valor > INT_MAX) {
                printf("Erro: jobId invalido: '%s'.\n", tokens[1]);
                continue;
            }
            int job_id = (int)valor;

            Job *job = buscar_job(
                jobs,
                quantidade_jobs,
                job_id
            );

            if (job == NULL) {
                printf(
                    "Erro: job %d não existe.\n",
                    job_id
                );
                continue;
            }

            aguardar_job(job);
            }


        else if (strcmp(tokens[0], "run") == 0) {

            if (quantidade_tokens < 2) {

                printf(
                    "Erro: uso correto: run <nome>\n"
                );

                continue;
            }

            if (
                strcmp(
                    tokens[1],
                    "sequential"
                ) == 0
            ) {

                if (quantidade_tokens < 3) {

                    printf(
                        "Erro: uso correto: "
                        "run sequential "
                        "<tarefa1> [tarefa2...]\n"
                    );

                    continue;
                }

                for (
                    int i = 2;
                    i < quantidade_tokens;
                    i++
                ) {

                    Task *task = buscar_task(
                        tarefas,
                        quantidade_tasks,
                        tokens[i]
                    );

                    if (task == NULL) {

                        printf(
                            "Erro: task '%s' "
                            "não encontrada.\n",
                            tokens[i]
                        );

                        break;
                    }

                    executar_task(
                        task,
                        diretorio_trabalho
                    );
                }
            }

            else if (
                strcmp(
                    tokens[1],
                    "parallel"
                ) == 0
            ) {

                if (quantidade_tokens < 3) {

                    printf(
                        "Erro: uso correto: "
                        "run parallel "
                        "<tarefa1> [tarefa2...]\n"
                    );

                    continue;
                }

                pid_t pids[MAX_TOKENS];
                int quantidade_pids = 0;
                Task *tasks_parallel[MAX_TOKENS];
                int quantidade_tasks_parallel = 0;
                int erro_parallel = 0;

                for (
                    int i = 2;
                    i < quantidade_tokens;
                    i++
                ) {

                    Task *task = buscar_task(
                        tarefas,
                        quantidade_tasks,
                        tokens[i]
                    );

                    if (task == NULL) {

                        printf(
                            "Erro: task '%s' "
                            "não encontrada.\n",
                            tokens[i]
                        );

                        erro_parallel = 1;
                        break;
                    }

                    tasks_parallel[quantidade_tasks_parallel] = task;
                    quantidade_tasks_parallel++;
                }

                if (erro_parallel) {
                    continue;
                }

                for (int i = 0; i < quantidade_tasks_parallel; i++) {
                    Task *task = tasks_parallel[i];

                    pid_t pid = iniciar_task(
                        task,
                        diretorio_trabalho
                    );

                    if (pid != -1) {

                        pids[quantidade_pids] = pid;
                        quantidade_pids++;
                    }
                }


                for (
                    int i = 0;
                    i < quantidade_pids;
                    i++
                ) {

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
            }


            else if (
                strcmp(
                    tokens[1],
                    "pipe"
                ) == 0
            ) {

                if (quantidade_tokens < 4) {

                    printf(
                        "Erro: uso correto: "
                        "run pipe "
                        "<tarefa1> <tarefa2> "
                        "[tarefa3...]\n"
                    );

                    continue;
                }

                int quantidade_pipe =
                    quantidade_tokens - 2;

                Task *tasks_pipe[MAX_TOKENS];

                int erro = 0;

                for (
                    int i = 0;
                    i < quantidade_pipe;
                    i++
                ) {

                    tasks_pipe[i] = buscar_task(
                        tarefas,
                        quantidade_tasks,
                        tokens[i + 2]
                    );

                    if (tasks_pipe[i] == NULL) {

                        printf(
                            "Erro: task '%s' "
                            "não encontrada.\n",
                            tokens[i + 2]
                        );

                        erro = 1;
                        break;
                    }
                }

                if (erro) {
                    continue;
                }

                executar_pipe(
                    tasks_pipe,
                    quantidade_pipe,
                    diretorio_trabalho
                );
            }

            else {

                if (quantidade_tokens != 2) {

                    printf(
                        "Erro: uso correto: "
                        "run <nome>\n"
                    );

                    continue;
                }

                Task *task = buscar_task(
                    tarefas,
                    quantidade_tasks,
                    tokens[1]
                );

                if (task == NULL) {

                    printf(
                        "Erro: task '%s' "
                        "não encontrada.\n",
                        tokens[1]
                    );

                    continue;
                }

                executar_task(
                    task,
                    diretorio_trabalho
                );
            }
        }

        else {

            printf(
                "Comando desconhecido.\n"
            );
        }
    }

    if(modo_workflow){
        fclose(entrada);
    }

    liberar_tasks(tarefas, quantidade_tasks);

    return 0;
}
