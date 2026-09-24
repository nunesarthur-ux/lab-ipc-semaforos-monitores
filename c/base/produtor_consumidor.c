/*
 * produtor_consumidor.c -- CODIGO BASE (sem sincronizacao)
 * Laboratorio de Comunicacao Interprocessos - Parte 2 (Semaforos POSIX)
 *
 * Este programa implementa o problema do produtor-consumidor com um
 * buffer circular compartilhado, mas AINDA SEM NENHUMA SINCRONIZACAO
 * entre as threads produtoras e consumidoras.
 *
 * Sua tarefa: seguir os TODOs numerados (TODO 0 a TODO 9) e inserir as
 * diretivas de semaforos necessarias para tornar o acesso ao buffer
 * seguro. Nao altere a logica de negocio (insercao/remocao), apenas
 * adicione as chamadas de sincronizacao nos pontos indicados.
 *
 * Compilar:  gcc -Wall -Wextra -pthread produtor_consumidor.c -o pc
 * Executar:  ./pc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define TAM_BUFFER          5
#define N_PRODUTORES        2
#define N_CONSUMIDORES      2
#define ITENS_POR_PRODUTOR  10
#define TOTAL_ITENS         (N_PRODUTORES * ITENS_POR_PRODUTOR)

/* ---------------------------------------------------------------
 * Pontos de injecao de atraso, usados nos experimentos da Parte 2.4
 * do roteiro. Deixe os dois em 0 ate que o roteiro peça para alterar.
 * ------------------------------------------------------------- */
#define ATRASO_PRODUTOR_US    500000
#define ATRASO_CONSUMIDOR_US  500000

typedef struct {
    int dados[TAM_BUFFER];
    int inicio;
    int fim;
    int contador;   /* quantidade de itens atualmente no buffer */
} Buffer;

Buffer buffer = { .inicio = 0, .fim = 0, .contador = 0 };

sem_t mutex; //exclusao mutua no acesso ao buffer
sem_t vazio; //numero de posicoes livres no buffer
sem_t cheio; //numero de posicoes preenchidas no buffer 

/* TODO 0: declare aqui os semaforos necessarios.
 *   sem_t mutex;   -> exclusao mutua no acesso ao buffer
 *   sem_t vazio;   -> conta quantos espacos livres existem no buffer
 *   sem_t cheio;   -> conta quantos itens prontos para consumo existem
 */



/* ---- Infraestrutura auxiliar de log/verificacao. NAO faz parte do
 *      exercicio de sincronizacao: serve apenas para voce enxergar o
 *      resultado (correto ou corrompido) ao final da execucao. ---- */
int total_produzido = 0;
int total_consumido = 0;
int proximo_id = 0;
int foi_consumido[TOTAL_ITENS] = {0};
pthread_mutex_t aux_mutex = PTHREAD_MUTEX_INITIALIZER;

void inserir_item(int item, int id_produtor) {
    /* ================= INICIO DA SECAO CRITICA ================= */
    buffer.dados[buffer.fim] = item;
    buffer.fim = (buffer.fim + 1) % TAM_BUFFER;
    buffer.contador++;
    total_produzido++;
    /* ================== FIM DA SECAO CRITICA ==================== */

    pthread_mutex_lock(&aux_mutex);
    printf("[Produtor %d] produziu item %-3d (buffer.contador=%d)\n",
           id_produtor, item, buffer.contador);
    if (buffer.contador < 0 || buffer.contador > TAM_BUFFER) {
        printf("*** ERRO DE CONSISTENCIA: buffer.contador=%d fora de [0,%d] ***\n",
               buffer.contador, TAM_BUFFER);
    }
    pthread_mutex_unlock(&aux_mutex);
}

int remover_item(int id_consumidor) {
    /* ================= INICIO DA SECAO CRITICA ================= */
    int item = buffer.dados[buffer.inicio];
    buffer.inicio = (buffer.inicio + 1) % TAM_BUFFER;
    buffer.contador--;
    total_consumido++;
    /* ================== FIM DA SECAO CRITICA ==================== */

    pthread_mutex_lock(&aux_mutex);
    printf("[Consumidor %d] consumiu item %-3d (buffer.contador=%d)\n",
           id_consumidor, item, buffer.contador);
    if (buffer.contador < 0 || buffer.contador > TAM_BUFFER) {
        printf("*** ERRO DE CONSISTENCIA: buffer.contador=%d fora de [0,%d] ***\n",
               buffer.contador, TAM_BUFFER);
    }
    if (item < 0 || item >= TOTAL_ITENS) {
        printf("*** ERRO: item invalido consumido: %d ***\n", item);
    } else if (foi_consumido[item]) {
        printf("*** ERRO: item %d CONSUMIDO EM DUPLICATA! ***\n", item);
    } else {
        foi_consumido[item] = 1;
    }
    pthread_mutex_unlock(&aux_mutex);

    return item;
}

void *produtor(void *arg) {
    int id = *(int *) arg;
    for (int i = 0; i < ITENS_POR_PRODUTOR; i++) {
        int item = __sync_fetch_and_add(&proximo_id, 1);
        /* TODO 1: aguardar que haja espaco livre no buffer
        *   sem_wait(&vazio);
        */
        sem_wait(&vazio);
        
        /* TODO 2: entrar na secao critica
        *   sem_wait(&mutex);
        */
        sem_wait(&mutex);
        
        if (ATRASO_PRODUTOR_US > 0) usleep(ATRASO_PRODUTOR_US);

        inserir_item(item, id);
        sem_post(&mutex); 
        /* TODO 3: sair da secao critica
         *   sem_post(&mutex);
         */
        sem_post(&cheio);

        /* TODO 4: sinalizar que ha um novo item disponivel para consumo
         *   sem_post(&cheio);
         */
    }
    return NULL;
}

void *consumidor(void *arg) {
    int id = *(int *) arg;
    for (int i = 0; i < TOTAL_ITENS / N_CONSUMIDORES; i++) {
        /* TODO 5: aguardar que haja pelo menos um item disponivel
         *   sem_wait(&cheio);
         */
        sem_wait(&cheio);

        /* TODO 6: entrar na secao critica
         *   sem_wait(&mutex);
         */
        sem_wait(&mutex);

        if (ATRASO_CONSUMIDOR_US > 0) usleep(ATRASO_CONSUMIDOR_US);

        remover_item(id);

        /* TODO 7: sair da secao critica
         *   sem_post(&mutex);
         */
        sem_post(&mutex);

        /* TODO 8: sinalizar que uma vaga ficou livre no buffer
         *   sem_post(&vazio);
         */
        sem_post(&vazio);
    }
    return NULL;
}

int main(void) {
    pthread_t prod[N_PRODUTORES], cons[N_CONSUMIDORES];
    int ids_prod[N_PRODUTORES], ids_cons[N_CONSUMIDORES];

    /* TODO 0b: inicialize aqui os semaforos declarados no TODO 0.
     *   sem_init(&mutex, 0, 1);          // binario, comeca "livre"
     *   sem_init(&vazio, 0, TAM_BUFFER); // comeca com todo o buffer livre
     *   sem_init(&cheio, 0, 0);          // comeca sem nenhum item pronto
     */
    sem_init(&mutex, 0, 1);
    sem_init(&vazio, 0, TAM_BUFFER);
    sem_init(&cheio, 0, 0);

    for (int i = 0; i < N_PRODUTORES; i++) {
        ids_prod[i] = i;
        pthread_create(&prod[i], NULL, produtor, &ids_prod[i]);
    }
    for (int i = 0; i < N_CONSUMIDORES; i++) {
        ids_cons[i] = i;
        pthread_create(&cons[i], NULL, consumidor, &ids_cons[i]);
    }

    for (int i = 0; i < N_PRODUTORES; i++) pthread_join(prod[i], NULL);
    for (int i = 0; i < N_CONSUMIDORES; i++) pthread_join(cons[i], NULL);

    int nao_consumidos = 0;
    for (int i = 0; i < TOTAL_ITENS; i++) {
        if (!foi_consumido[i]) nao_consumidos++;
    }

    printf("\n===== RESULTADO FINAL =====\n");
    printf("Esperado ......... %d itens\n", TOTAL_ITENS);
    printf("Total produzido .. %d\n", total_produzido);
    printf("Total consumido .. %d\n", total_consumido);
    printf("Nunca consumidos . %d\n", nao_consumidos);
    printf("buffer.contador final = %d (deveria ser 0)\n", buffer.contador);

    /* TODO 9 (opcional, mas recomendado): destrua os semaforos
     *   sem_destroy(&mutex);
     *   sem_destroy(&vazio);
     *   sem_destroy(&cheio);
     */

    return 0;
}
