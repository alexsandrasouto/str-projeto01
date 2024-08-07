#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_BARBEIROS 2
#define NUM_CADEIRAS_ESPERA 1

sem_t sem_clientes; // Conta o número de clientes esperando
sem_t sem_barbeiros; // Conta o número de barbeiros disponíveis

sem_t sem_bin_barbeiros[NUM_BARBEIROS];

pthread_mutex_t mutex_cadeiras_espera; // Exclusão mútua para as cadeiras de espera
pthread_mutex_t mutex_cadeira_barbeiro;

int cadeiras_espera_livres[NUM_CADEIRAS_ESPERA];
int in = 0, out = 0;

void* cliente(void* id) {

    int cliente_id = *(int*)id;
    printf("Cliente %d chegou.\n", cliente_id);

    pthread_mutex_lock(&mutex_cadeiras_espera);
    if (cadeiras_espera_livres > 0) {
        cadeiras_espera_livres--;
        printf("Cliente %d sentou na cadeira de espera. Cadeiras de espera livres: %d\n", cliente_id, cadeiras_espera_livres);
        pthread_mutex_unlock(&mutex_cadeiras_espera);

            sem_wait(&sem_barbeiros);

            pthread_mutex_lock(&mutex_cadeira_barbeiro);
            printf("Cliente %d está sendo atendido.\n", cliente_id);

            sleep(2); // Simula o tempo do corte de cabelo
            printf("Cliente %d terminou o corte de cabelo e saiu.\n", cliente_id);

            pthread_mutex_unlock(&mutex_cadeira_barbeiro);

            sem_wait(&sem_clientes);

            free(id);
            pthread_exit(NULL);
        
     }

     else {
        pthread_mutex_unlock(&mutex_cadeiras_espera);
        printf("Cliente %d foi embora sem ser atendido, não há cadeiras de espera disponíveis.\n", cliente_id);
    }

    free(id);
    pthread_exit(NULL);

}


void* barbeiro(void* id) {
    int barbeiro_id = *(int*)id;

    while(1){

        sem_wait(&sem_barbeiros);
        sem_wait(&sem_clientes);

        sem_post(&sem_bin_barbeiros[barbeiro_id-1]);

        // pthread_mutex_lock(&mutex_cadeiras_espera);
        cadeiras_espera_livres++;
        printf("Barbeiro %d está atendendo um cliente. Cadeiras de espera livres: %d\n", barbeiro_id, cadeiras_espera_livres);
        
        sleep(2); // Simula o tempo do corte de cabelo
        printf("Barbeiro %d terminou de atender e está pronto para o próximo cliente.\n", barbeiro_id);

        sem_wait(&sem_bin_barbeiros[barbeiro_id-1]);


    }

    free(id);
    pthread_exit(NULL);

}

int main() {
     srand(time(NULL));
    
    pthread_t barbeiros[NUM_BARBEIROS];
    pthread_t clientes[10];

    sem_init(&sem_clientes, 0, 0);
    sem_init(&sem_barbeiros, 0, NUM_BARBEIROS);
    sem_init(&sem_bin_barbeiros[NUM_BARBEIROS], 0, 0);
    pthread_mutex_init(&mutex_cadeiras_espera, NULL);
    pthread_mutex_init(&mutex_cadeira_barbeiro, NULL);

    for (int i = 0; i < NUM_BARBEIROS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&barbeiros[i], NULL, barbeiro, id);
    }

    for (int i = 0; i < 10; i++) {

        sleep(1); // Clientes chegam aleatoriamente
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&clientes[i], NULL, cliente, id);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(clientes[i], NULL);
    }


    sem_destroy(&sem_clientes);
    sem_destroy(&sem_barbeiros);
    pthread_mutex_destroy(&mutex_cadeiras_espera);
    pthread_mutex_destroy(&mutex_cadeira_barbeiro);

    return 0;

}

