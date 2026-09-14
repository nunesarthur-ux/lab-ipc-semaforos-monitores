/*
 * produtor_consumidor.c -- SOLUCAO DE REFERENCIA
 * Laboratorio de Comunicacao Interprocessos - Parte 3 (Semaforos POSIX)
 *
 * Versao com os TODOs do codigo base resolvidos. So consulte este
 * arquivo depois de ter tentado resolver o codigo base sozinho(a) --
 * ver Parte 2 do roteiro (README.md).
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

/* Pontos de injecao de atraso usados nos experimentos da Parte 2.4. */
#define ATRASO_PRODUTOR_US    0
#define ATRASO_CONSUMIDOR_US  0

typedef struct {
    int dados[TAM_BUFFER];
    int inicio;
    int fim;
    int contador;
} Buffer;

Buffer buffer = { .inicio = 0, .fim = 0, .contador = 0 };

/* TODO 0 resolvido: */
sem_t mutex;   /* exclusao mutua no acesso ao buffer               */
sem_t vazio;   /* conta espacos livres no buffer                    */
sem_t cheio;   /* conta itens prontos para consumo                  */

int total_produzido = 0;
int total_consumido = 0;
int proximo_id = 0;
int foi_consumido[TOTAL_ITENS] = {0};
pthread_mutex_t aux_mutex = PTHREAD_MUTEX_INITIALIZER;

void inserir_item(int item, int id_produtor) {
    buffer.dados[buffer.fim] = item;
    buffer.fim = (buffer.fim + 1) % TAM_BUFFER;
    buffer.contador++;
    total_produzido++;

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
    int item = buffer.dados[buffer.inicio];
    buffer.inicio = (buffer.inicio + 1) % TAM_BUFFER;
    buffer.contador--;
    total_consumido++;

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

        sem_wait(&vazio);   /* TODO 1 */
        sem_wait(&mutex);   /* TODO 2 */

        if (ATRASO_PRODUTOR_US > 0) usleep(ATRASO_PRODUTOR_US);

        inserir_item(item, id);

        sem_post(&mutex);   /* TODO 3 */
        sem_post(&cheio);   /* TODO 4 */
    }
    return NULL;
}

void *consumidor(void *arg) {
    int id = *(int *) arg;
    for (int i = 0; i < TOTAL_ITENS / N_CONSUMIDORES; i++) {
        sem_wait(&cheio);   /* TODO 5 */
        sem_wait(&mutex);   /* TODO 6 */

        if (ATRASO_CONSUMIDOR_US > 0) usleep(ATRASO_CONSUMIDOR_US);

        remover_item(id);

        sem_post(&mutex);   /* TODO 7 */
        sem_post(&vazio);   /* TODO 8 */
    }
    return NULL;
}

int main(void) {
    pthread_t prod[N_PRODUTORES], cons[N_CONSUMIDORES];
    int ids_prod[N_PRODUTORES], ids_cons[N_CONSUMIDORES];

    /* TODO 0b resolvido: */
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

    sem_destroy(&mutex);   /* TODO 9 */
    sem_destroy(&vazio);
    sem_destroy(&cheio);

    return 0;
}
