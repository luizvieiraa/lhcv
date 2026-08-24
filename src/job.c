#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "job.h"

int adicionar_job(
    Job jobs[],
    int *quantidade_jobs,
    pid_t pid,
    char *nome
) {

    if (*quantidade_jobs >= MAX_JOBS) {

        printf(
            "Erro: limite máximo de jobs atingido.\n"
        );

        return -1;
    }

    int id = *quantidade_jobs + 1;

    Job *job = &jobs[*quantidade_jobs];

    job->id = id;
    job->pid = pid;

    strcpy(
        job->nome,
        nome
    );

    job->finalizado = 0;
    job->status = 0;

    (*quantidade_jobs)++;

    return id;
}

Job *buscar_job(
    Job jobs[],
    int quantidade_jobs,
    int id
) {

    for (int i = 0; i < quantidade_jobs; i++) {

        if (jobs[i].id == id) {
            return &jobs[i];
        }
    }

    return NULL;
}

void atualizar_jobs(
    Job jobs[],
    int quantidade_jobs
) {

    for (int i = 0; i < quantidade_jobs; i++) {

        if (jobs[i].finalizado) {
            continue;
        }

        int status;

        pid_t resultado = waitpid(
            jobs[i].pid,
            &status,
            WNOHANG
        );

        if (resultado > 0) {

            jobs[i].finalizado = 1;
            jobs[i].status = status;
        }
    }
}

void listar_jobs(
    Job jobs[],
    int quantidade_jobs
) {

    atualizar_jobs(
        jobs,
        quantidade_jobs
    );

    for (int i = 0; i < quantidade_jobs; i++) {

        printf(
            "[%d] %d %s %s\n",
            jobs[i].id,
            jobs[i].pid,
            jobs[i].nome,
            jobs[i].finalizado
                ? "DONE"
                : "RUNNING"
        );
    }
}

int aguardar_job(
    Job *job
) {

    if (job->finalizado) {
        return 0;
    }

    int status;

    if (
        waitpid(
            job->pid,
            &status,
            0
        ) == -1
    ) {

        perror("waitpid");
        return -1;
    }

    job->finalizado = 1;
    job->status = status;

    return 0;
}