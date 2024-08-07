#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Definições
#define NUM_BARBEIROS 3
#define NUM_CADEIRAS_ESPERA 5

sem_t sem_clientes; // Conta o número de clientes esperando
sem_t sem_barbeiros; // Conta os babeiros disponíveis
sem_t sem_cadeiras_espera; // Conta as cadeiras de espera disponíveis

pthread_mutex_t  mutex_cadeiras_espera, mutex_cadeira_barbeiro[NUM_BARBEIROS]; // Exclusão mútua para as cadeiras de espera e cadeiras dos barbeiros

void* cliente(void* id) {

    int cliente_id = *(int*)id;
    printf("Cliente %d chegou.\n", cliente_id);
    int cadeiras_livres = 0;

    while(1){

        pthread_mutex_lock(&mutex_cadeiras_espera); // Mutex de acesso à cadeiras de espera, para evitar acesso simultâneo
        sem_getvalue(&sem_cadeiras_espera, &cadeiras_livres);

        if(cadeiras_livres > 0){ // Verifica se há cadeiras de espera disponíveis

            sem_wait(&sem_cadeiras_espera); // Espera por uma cadeira de espera

            sem_getvalue(&sem_cadeiras_espera, &cadeiras_livres);
            printf("Cliente %d sentou na cadeira de espera. Cadeiras de espera livres: %d \n", cliente_id, cadeiras_livres);

            sem_post(&sem_clientes); // Sinaliza que um cliente chegou
            pthread_mutex_unlock(&mutex_cadeiras_espera); // Libera o mutex de acesso à fila de cadeiras de espera

            sem_wait(&sem_barbeiros);  // Sinaliza que há um cliente esperando por um barbeiro

            // Verifica qual babeiro está disponível para atender
            for(int i = 0; i < NUM_BARBEIROS; i++){
                if (pthread_mutex_trylock(&mutex_cadeira_barbeiro[i]) == 0) {  // Bloqueia o acesso à cadeira do barbeiro disponível
                    // Cliente está sendo atendido por um barbeiro
                    printf("Cliente %d está sendo atendido. \n", cliente_id);

                    sleep(rand() % 2 +1); // Simula o tempo de atendimento

                    printf("Cliente %d terminou o corte de cabelo e saiu. \n", cliente_id);
        
                    pthread_mutex_unlock(&mutex_cadeira_barbeiro[i]); // Libera o acesso à cadeira do barbeiro que ele estava 
                    sem_post(&sem_barbeiros); // Sinaliza que há um barbeiro disponível
                    break;
                }
            }

            free(id);
            pthread_exit(NULL);

        }
        else{
            pthread_mutex_unlock(&mutex_cadeiras_espera); // Libera o mutex de acesso à fila de cadeiras de espera
            printf("Cliente %d foi embora sem ser atendido. Não há cadeiras de espera livres. \n", cliente_id);

            free(id);
            pthread_exit(NULL);
        }
    }
}

void* barbeiro(void* id) {

    int barbeiro_id = *(int*)id;
    int clientes = 0;
    int cadeiras_livres = 0;

    while(1){

        sem_getvalue(&sem_clientes, &clientes);

        if(clientes == 0){ // Verifica se há clientes para serem atendidos, caso não, o babeiro dorme
            printf("O barbeiro %d está dormindo. \n", barbeiro_id);
        }

        sem_wait(&sem_clientes); // Sinaliza que está esperando por um cliente para atender 

        pthread_mutex_lock(&mutex_cadeiras_espera); // Bloqueia o acesso ao mutex de cadeiras de espera
        sem_post(&sem_cadeiras_espera);   // Incrementa o valor de cadeiras de espera, já que está atendendo um cliente
        sem_getvalue(&sem_cadeiras_espera, &cadeiras_livres);

        printf("O barbeiro %d está acordado. Atendendo um cliente. Cadeiras de espera livres: %d  \n", barbeiro_id, cadeiras_livres);
        pthread_mutex_unlock(&mutex_cadeiras_espera);  // Libera o acesso ao mutex de cadeiras de espera
        
        sleep(rand() % 2 +1); // Simula o tempo de atendimento
        
        printf("Barbeiro %d terminou de atender e está pronto para o próximo cliente.\n", barbeiro_id);
    }

    free(id);
    pthread_exit(NULL);
}

void* gerar_clientes(void*) {
    int cliente_id = 1;
    while(1) {
        sleep(rand() % 3); // Clientes chegam aleatoriamente
        int* id = malloc(sizeof(int));
        *id = cliente_id++;
        pthread_t cliente_thread;
        pthread_create(&cliente_thread, NULL, cliente, id);
        pthread_detach(cliente_thread); 
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    // Inicializa os semáforos
    sem_init(&sem_clientes, 0, 0);
    sem_init(&sem_barbeiros, 0, NUM_BARBEIROS);
    sem_init(&sem_cadeiras_espera, 0, NUM_CADEIRAS_ESPERA);

    // Inicializa os mutexes
    pthread_mutex_init(&mutex_cadeiras_espera, NULL);
    for (int i = 0; i < NUM_BARBEIROS; i++) {
        pthread_mutex_init(&mutex_cadeira_barbeiro[i], NULL);
    }

    pthread_t barbeiros[NUM_BARBEIROS];
    pthread_t gerador_clientes;

    // Cria threads para os barbeiros
    for (int i = 0; i < NUM_BARBEIROS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&barbeiros[i], NULL, barbeiro, id);
    }

    // Cria thread para gerar clientes indefinidamente
    pthread_create(&gerador_clientes, NULL, gerar_clientes, NULL);
    pthread_join(gerador_clientes, NULL);

    // Destrói os semáforos e mutexes
    sem_destroy(&sem_clientes);
    sem_destroy(&sem_barbeiros);
    sem_destroy(&sem_cadeiras_espera);
    pthread_mutex_destroy(&mutex_cadeiras_espera);
    for (int i = 0; i < NUM_BARBEIROS; i++) {
        pthread_mutex_destroy(&mutex_cadeira_barbeiro[i]);
    }

    return 0;
}