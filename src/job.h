#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#define MAX_JOBS 100

typedef struct {

    int id;
    pid_t pid;

    char nome[50];

    int finalizado;
    int status;

} Job;

int adicionar_job(
    Job jobs[],
    int *quantidade_jobs,
    pid_t pid,
    char *nome
);

Job *buscar_job(
    Job jobs[],
    int quantidade_jobs,
    int id
);

void atualizar_jobs(
    Job jobs[],
    int quantidade_jobs
);

void listar_jobs(
    Job jobs[],
    int quantidade_jobs
);

int aguardar_job(
    Job *job
);

#endif