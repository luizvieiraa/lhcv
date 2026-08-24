# ProcessFlow

O **ProcessFlow** é um interpretador de comandos desenvolvido em **C para ambiente Linux** como projeto da disciplina de Sistemas Operacionais.

O programa permite cadastrar e executar tarefas utilizando processos do sistema operacional, explorando conceitos como `fork()`, `exec()`, `waitpid()`, pipes, redirecionamento de entrada e saída e execução em background.

## Funcionalidades

O ProcessFlow possui suporte a:

- Cadastro de tarefas
- Execução simples
- Execução sequencial
- Execução paralela
- Pipes entre tarefas
- Redirecionamento de entrada
- Redirecionamento de saída
- Escrita em modo append
- Definição de diretório de trabalho
- Execução em background
- Gerenciamento de jobs
- Espera por jobs
- Execução através de arquivos workflow `.pf`
- Tratamento de erros

## Estrutura do Projeto

```text
processflow/
├── src/
│   ├── main.c
│   ├── task.c
│   ├── task.h
│   ├── job.c
│   └── job.h
├── Makefile
├── README.md
├── evidencias.log
└── teste.pf
```

### Arquivos principais

- `main.c`: interpreta os comandos e controla os modos interativo e workflow.
- `task.c`: implementa cadastro e execução das tarefas, redirecionamentos, pipes e diretório de trabalho.
- `task.h`: define a estrutura `Task` e as funções relacionadas às tarefas.
- `job.c`: implementa o gerenciamento de processos executados em background.
- `job.h`: define a estrutura `Job` e as funções relacionadas aos jobs.
- `Makefile`: automatiza a compilação e limpeza do projeto.
- `teste.pf`: exemplo de arquivo workflow.
- `evidencias.log`: registro da sessão de testes do projeto.

## Requisitos

O projeto deve ser executado em um ambiente Linux.

Para compilar, é necessário possuir:

- GCC
- GNU Make
- Bibliotecas padrão POSIX utilizadas pelo Linux

O projeto foi desenvolvido e testado utilizando Linux através do WSL.

## Compilação

Na raiz do projeto, execute:

```bash
make clean
make
```

O executável `processflow` será criado na raiz do projeto.

## Execução

### Modo interativo

Execute:

```bash
./processflow
```

O programa exibirá:

```text
processflow>
```

Os comandos podem então ser digitados diretamente.

Para encerrar:

```text
exit
```

### Modo Workflow

Também é possível executar comandos armazenados em um arquivo `.pf`.

Exemplo:

```bash
./processflow teste.pf
```

Exemplo de conteúdo de `teste.pf`:

```text
task listar /bin/ls
task data /bin/date
run listar
run data
exit
```

No modo workflow, as linhas do arquivo são executadas automaticamente.

## Comandos

### Cadastrar uma tarefa

```text
task <nome> <programa> [argumentos]
```

Exemplo:

```text
task listar /bin/ls -l
```

### Executar uma tarefa

```text
run <nome>
```

Exemplo:

```text
run listar
```

### Execução sequencial

```text
run sequential <tarefa1> <tarefa2> [...]
```

Exemplo:

```text
run sequential data listar
```

Cada tarefa termina antes da próxima ser executada.

### Execução paralela

```text
run parallel <tarefa1> <tarefa2> [...]
```

Exemplo:

```text
run parallel tarefa1 tarefa2
```

As tarefas são iniciadas em processos diferentes e podem executar simultaneamente.

### Pipeline

```text
run pipe <tarefa1> <tarefa2> [...]
```

Exemplo:

```text
task listar /bin/ls
task contar /usr/bin/wc -l

run pipe listar contar
```

A saída de uma tarefa é conectada à entrada da próxima.

### Redirecionamento de entrada

```text
input <tarefa> <arquivo>
```

Exemplo:

```text
task mostrar /bin/cat
input mostrar nomes.txt
run mostrar
```

### Redirecionamento de saída

```text
output <tarefa> <arquivo>
```

Exemplo:

```text
output listar resultado.txt
run listar
```

### Append

Para adicionar a saída ao final de um arquivo existente:

```text
append <tarefa> <arquivo>
```

Exemplo:

```text
append data resultado.txt
run data
```

### Diretório de trabalho

```text
workdir <diretorio>
```

Exemplo:

```text
workdir /tmp
```

As tarefas posteriores serão executadas utilizando o diretório informado como diretório de trabalho.

### Execução em background

```text
start <tarefa>
```

Exemplo:

```text
task dormir /bin/sleep 5
start dormir
```

O ProcessFlow retorna imediatamente ao prompt enquanto o processo continua executando.

### Listar jobs

```text
jobs
```

Exemplo de saída:

```text
[1] 1234 dormir RUNNING
```

Após o processo terminar, seu estado passa para `DONE`.

### Esperar um job

```text
wait <jobId>
```

Exemplo:

```text
wait 1
```

O ProcessFlow aguarda a finalização do job informado.

## Exemplo de Uso

```text
processflow> task listar /bin/ls
Task 'listar' cadastrada.

processflow> task data /bin/date
Task 'data' cadastrada.

processflow> run data
Mon Aug 24 18:34:42 -03 2026

processflow> run listar
Makefile README.md src teste.pf

processflow> exit
```

## Tratamento de Erros

O ProcessFlow realiza tratamento para situações como:

- Tarefa inexistente
- Job inexistente
- Programa inexistente ou não executável
- Arquivo de entrada ou saída inválido
- Diretório de trabalho inexistente
- Workflow inexistente
- Quantidade incorreta de argumentos
- Comandos desconhecidos
- Falhas em chamadas de sistema
- Processos terminando com código diferente de zero
- EOF no modo interativo ou workflow

Exemplo:

```text
processflow> run inexistente
Erro: task 'inexistente' não encontrada.
```

## Testes

Uma sessão de testes foi registrada no arquivo:

```text
evidencias.log
```

Entre os testes realizados estão:

- Compilação do zero
- Cadastro e execução de tasks
- Múltiplos espaços
- Execução sequencial
- Execução paralela
- Pipes
- Redirecionamentos
- Diretório de trabalho
- Background
- `jobs`
- `wait`
- Workflow `.pf`
- Workflow sem `exit`
- Casos inválidos e tratamento de erros

Para realizar uma compilação limpa:

```bash
make clean
make
```

E executar o programa:

```bash
./processflow
```

## Limpeza

Para remover o executável gerado:

```bash
make clean
```

## Conceitos Utilizados

Durante o desenvolvimento foram utilizadas chamadas e conceitos de sistemas operacionais como:

- `fork()`
- `exec()`
- `waitpid()`
- `pipe()`
- `dup2()`
- processos pai e filho
- processos em background
- gerenciamento de jobs
- descritores de arquivos
- redirecionamento de entrada e saída

O projeto não utiliza `system()` ou `popen()` para executar as tarefas.