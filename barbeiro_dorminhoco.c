#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Definições
#define NUM_BARBEIROS 1
#define NUM_CADEIRAS_ESPERA 1
#define NUM_CLIENTES 5

// Semáforos e mutex
sem_t sem_clientes; // Conta o número de clientes esperando
sem_t sem_barbeiros;
sem_t sem_cadeiras_espera;

pthread_mutex_t  mutex_cadeiras_espera, mutex_cadeira_barbeiro; // Exclusão mútua para as cadeiras de espera

int barbeiros_ocupados[NUM_BARBEIROS] = {0};

void* cliente(void* id) {

    int cliente_id = *(int*)id;
    printf("Cliente %d chegou.\n", cliente_id);
    int cadeiras_livres = 0;

    while(1){

        pthread_mutex_lock(&mutex_cadeiras_espera); 
        sem_getvalue(&sem_cadeiras_espera, &cadeiras_livres);
        // printf("%d", cadeiras_livres);

        if(cadeiras_livres > 0){

            sem_wait(&sem_cadeiras_espera);

            sem_getvalue(&sem_cadeiras_espera, &cadeiras_livres);
            printf("Cliente %d sentou na cadeira de espera. Cadeiras de espera livres: %d \n", cliente_id, cadeiras_livres);

            sem_post(&sem_clientes);
            sem_wait(&sem_barbeiros);
            pthread_mutex_unlock(&mutex_cadeiras_espera);

            pthread_mutex_lock(&mutex_cadeira_barbeiro);

            sem_getvalue(&sem_cadeiras_espera, &cadeiras_livres);

            for (int i = 0; i < NUM_BARBEIROS; i++) {
                if (barbeiros_ocupados[i] == 0) {
                    barbeiros_ocupados[i] = cliente_id;
                    printf("Cliente %d está sendo atendido pelo barbeiro %d. Cadeiras de espera livres: %d \n", cliente_id, i + 1, cadeiras_livres);
                    sem_post(&sem_cadeiras_espera);
                    break;
                }
            }

            pthread_mutex_unlock(&mutex_cadeira_barbeiro); // desbloqueia aqui mesmo??

            sleep(3);

            printf("Cliente %d terminou o corte de cabelo e saiu. \n", cliente_id);

            for (int i = 0; i < NUM_BARBEIROS; i++) {
                if (barbeiros_ocupados[i] == cliente_id) {
                    barbeiros_ocupados[i] = 0;
                    break;
                }   
            }

            sem_post(&sem_barbeiros);

            free(id);
            pthread_exit(NULL);

        }
        else{
            printf("Cliente %d foi embora sem ser atendido. Não há cadeiras de espera livres. \n", cliente_id);
            free(id);
            pthread_exit(NULL);
        
        }

    }
}




void* barbeiro(void* id) {

    int barbeiro_id = *(int*)id;
    int clientes = 0;
 
    while(1){

        sem_getvalue(&sem_clientes, &clientes);

        if(clientes == 0){
            printf("O barbeiro %d está dormindo. \n", barbeiro_id);
        }

        sem_post(&sem_barbeiros);
        sem_wait(&sem_clientes);

        printf("O barbeiro %d está acordado. Atentendo um cliente. \n", barbeiro_id);
        sleep(3);
        printf("Barbeiro %d terminou de atender e está pronto para o próximo cliente.\n", barbeiro_id);

        sem_post(&sem_barbeiros);
    }

    free(id);
    pthread_exit(NULL);

}

int main() {
    srand(time(NULL));

    sem_init(&sem_clientes, 0, 0);
    sem_init(&sem_barbeiros, 0, NUM_BARBEIROS);
    sem_init(&sem_cadeiras_espera, 0, NUM_CADEIRAS_ESPERA);

    pthread_mutex_init(&mutex_cadeiras_espera, NULL);
    pthread_mutex_init(&mutex_cadeira_barbeiro, NULL);

    pthread_t barbeiros[NUM_BARBEIROS];
    pthread_t clientes[NUM_CLIENTES];

    for (int i = 0; i < NUM_BARBEIROS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&barbeiros[i], NULL, barbeiro, id);
    }

    for (int i = 0; i < NUM_CLIENTES; i++) {

        sleep(1);// Clientes chegam aleatoriamente
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&clientes[i], NULL, cliente, id);
    }

    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(clientes[i], NULL);
    }


    sem_destroy(&sem_clientes);
    sem_destroy(&sem_barbeiros);
    sem_destroy(&sem_cadeiras_espera);
    pthread_mutex_destroy(&mutex_cadeiras_espera);
    pthread_mutex_destroy(&mutex_cadeira_barbeiro);

    return 0;
}
