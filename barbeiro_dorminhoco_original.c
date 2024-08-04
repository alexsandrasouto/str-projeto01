#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Definições
#define NUM_BARBEIROS 3
#define NUM_CADEIRAS_ESPERA 5

// Semáforos e mutex
sem_t sem_clientes; // Conta o número de clientes esperando
sem_t sem_barbeiros; // Conta o número de barbeiros disponíveis
pthread_mutex_t mutex_cadeiras_espera; // Exclusão mútua para as cadeiras de espera

int cadeiras_espera_livres = NUM_CADEIRAS_ESPERA;

void* funcao_cliente(void* id) {
    int cliente_id = *(int*)id;
    printf("Cliente %d chegou.\n", cliente_id);

    pthread_mutex_lock(&mutex_cadeiras_espera);
    if (cadeiras_espera_livres > 0) {
        cadeiras_espera_livres--;
        printf("Cliente %d sentou na cadeira de espera. Cadeiras de espera livres: %d\n", cliente_id, cadeiras_espera_livres);
        sem_post(&sem_clientes); // Sinaliza que há um cliente esperando
        pthread_mutex_unlock(&mutex_cadeiras_espera);

        sem_wait(&sem_barbeiros); // Espera por um barbeiro disponível

        printf("Cliente %d está sendo atendido.\n", cliente_id);
        sleep(rand() % 3 + 1); // Simula o tempo do corte de cabelo
        printf("Cliente %d terminou o corte de cabelo e saiu.\n", cliente_id);
    } else {
        pthread_mutex_unlock(&mutex_cadeiras_espera);
        printf("Cliente %d foi embora sem ser atendido, não há cadeiras de espera disponíveis.\n", cliente_id);
    }

    free(id);
    pthread_exit(NULL);
}

void* funcao_barbeiro(void* id) {
    int barbeiro_id = *(int*)id;

    while (1) {
        sem_wait(&sem_clientes); // Espera por um cliente
        pthread_mutex_lock(&mutex_cadeiras_espera);
        cadeiras_espera_livres++;
        printf("Barbeiro %d está atendendo um cliente. Cadeiras de espera livres: %d\n", barbeiro_id, cadeiras_espera_livres);
        pthread_mutex_unlock(&mutex_cadeiras_espera);

        sem_post(&sem_barbeiros); // Sinaliza que o barbeiro está ocupado

        sleep(rand() % 3 + 1); // Simula o tempo do corte de cabelo
        printf("Barbeiro %d terminou de atender e está pronto para o próximo cliente.\n", barbeiro_id);
    }

    free(id);
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    
    pthread_t barbeiros[NUM_BARBEIROS];
    pthread_t clientes[10]; // Definimos 10 clientes para teste

    // Inicializa os semáforos e mutex
    sem_init(&sem_clientes, 0, 0);
    sem_init(&sem_barbeiros, 0, NUM_BARBEIROS);
    pthread_mutex_init(&mutex_cadeiras_espera, NULL);

    // Cria as threads dos barbeiros
    for (int i = 0; i < NUM_BARBEIROS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&barbeiros[i], NULL, funcao_barbeiro, id);
    }

    // Cria as threads dos clientes
    for (int i = 0; i < 10; i++) {
        sleep(rand() % 3); // Clientes chegam aleatoriamente
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&clientes[i], NULL, funcao_cliente, id);
    }

    // Espera todas as threads de clientes terminarem
    for (int i = 0; i < 10; i++) {
        pthread_join(clientes[i], NULL);
    }

    // Cancelar barbeiros após os clientes serem atendidos (opcional)
    for (int i = 0; i < NUM_BARBEIROS; i++) {
        pthread_cancel(barbeiros[i]);
    }

    // Destrói os semáforos e mutex
    sem_destroy(&sem_clientes);
    sem_destroy(&sem_barbeiros);
    pthread_mutex_destroy(&mutex_cadeiras_espera);

    return 0;
}
