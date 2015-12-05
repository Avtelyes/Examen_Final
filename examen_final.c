//
//  main.c
//  mpi_basico
//
//  Created by Josue Garcia Puig on 4/12/15.
//  Copyright © 2015 Josue Garcia Puig. All rights reserved.
//

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define N 8
#define CORES 4

pthread_mutex_t * cuadrantes;
pthread_cond_t * espera_t;

int **creacionMatriz();
void impresionTablero(int **);

void * procesa_camino(void *);
int **matrix;
void encuentraCamino(int, int, int);
void gestor(int);

pid_t Principal;

int main(int argc, char * argv[])
{
    int  numtasks, rank, rc;
    
    rc = MPI_Init(&argc,&argv);
    
    if (rc != MPI_SUCCESS) {
        
        printf ("Error al iniciar MPI \n");
        
        MPI_Abort(MPI_COMM_WORLD, rc);
        
    }
    
    // Obteniendo el número de tareas o procesos
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    
    // Obteniendo el número de la tarea
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    
    // Obteniendo el nombre del nodo
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    
    printf ("[%s] Proceso %d de %d \n", processor_name, rank, 	numtasks);
    
    
    /*******  hacer algo *******/
    srand((int) time(NULL));
    matrix = creacionMatriz();
    
    if (signal(SIGALRM, gestor) == SIG_ERR) {
        printf("ERROR: No se pudo establecer el manejador de la señal\n");
    }
    
    Principal = getpid();
    
    pthread_t * vecinos = (pthread_t *) malloc(sizeof(pthread_t) * CORES);
    cuadrantes = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * CORES);
    espera_t = (pthread_cond_t *) malloc(sizeof(pthread_cond_t) * CORES);
    
    int i;
    
    for (i=0;i<CORES;++i){
        pthread_mutex_init(cuadrantes+i,NULL);
        pthread_cond_init(espera_t+i,NULL);
    }
    
    pthread_t * aux;
    int indice = 0;
    
    for(aux = vecinos; aux<(vecinos+CORES); ++aux)
    {
        pthread_create(aux, NULL, procesa_camino, (void *) ++indice);
    }
    
    //pthread_kill(*vecinos, SIGUSR1);
    
    /* Adjuntar los hilos */
    for (aux = vecinos; aux < (vecinos+CORES); ++aux) {
        pthread_join(*aux, NULL);
    }
    
    //raise(SIGUSR1);
    //impresionTablero(matrix);
    kill(Principal, SIGALRM);
    
    for(i=0; i<N; ++i)
        free(*(matrix+i));
    
    free(matrix);
    free(vecinos);
    free(cuadrantes);
    free(espera_t);
    
    
    MPI_Finalize();
    
    return 0;
    
}

void * procesa_camino(void * arg)
{
    int id = (int) arg;
    
    if(id == 1)
    {
        
        pthread_mutex_lock(cuadrantes+1);
        printf("Hola soy el proceso 1\n");
        encuentraCamino(0, 0, 4);
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+1);
    }
    else if (id == 2)
    {
        pthread_mutex_lock(cuadrantes+2);
        printf("Hola soy el proceso 2\n");
        encuentraCamino(0, N/2, 5);
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+2);
    }
    else if (id == 3)
    {
        pthread_mutex_lock(cuadrantes+3);
        printf("Hola soy el proceso 3\n");
        encuentraCamino(N/2, 0, 3);
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+3);
    }
    else if (id == 4)
    {
        pthread_mutex_lock(cuadrantes+4);
        printf("Hola soy el proceso 4\n");
        encuentraCamino(N/2, N/2, 6);
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+4);
    }
    
    pthread_exit(NULL);
}

void encuentraCamino(int in, int fn, int num)
{
    /*int **inicio = matrix+in;
     int **fin = (inicio+fn);
     
     int *inicio2;
     int *fin2;
     
     for(; inicio<fin; ++inicio)
     {
     inicio2 = *inicio+inX;
     fin2 = (inicio2+inY);
     for(; inicio2<fin2; ++inicio2)
     {
     if(*inicio2 != 1)
     {
     *inicio2 = num;
     }
     }
     }*/
    
    int i, j;
    for(i=0; i<4; ++i)
    {
        for(j=0; j<4; ++j)
        {
            if(*(*(matrix+in+i)+j+fn) != 1)
                *(*(matrix+in+i)+j+fn) = num;
        }
    }
    
}

int **creacionMatriz()
{
    int **matrix;
    
    int i, obstaculos = N;
    
    matrix = (int **) malloc(N * sizeof(int *));
    
    for(i = 0; i<N; ++i)
        *(matrix+i) = (int *) malloc(N * sizeof(int));
    
    int **inicio = matrix;
    int **fin = (matrix+N);
    
    int *inicio2;
    int *fin2;
    
    for(; inicio<fin; ++inicio)
    {
        inicio2 = *inicio;
        fin2 = (inicio2+N);
        for(; inicio2<fin2; ++inicio2)
        {
            if((rand() % 54 + 1) % 3 == 0  && obstaculos>0 && inicio2!=*inicio)
            {
                *inicio2 = 1;
                obstaculos--;
            }
            else
                *inicio2 = 0;
        }
    }
    
    inicio = matrix;
    fin = (matrix+N);
    
    for(; inicio<fin; ++inicio)
    {
        inicio2 = *inicio;
        fin2 = (inicio2+N);
        for(; inicio2<fin2; ++inicio2)
        {
            printf("[%d]",*inicio2);
        }
        printf("\n");
    }
    
    return matrix;
}

void impresionTablero(int ** matrix)
{
    int **inicio = matrix;
    int **fin = (matrix+N);
    
    int *inicio2;
    int *fin2;
    
    for(; inicio<fin; ++inicio)
    {
        inicio2 = *inicio;
        fin2 = (inicio2+N);
        for(; inicio2<fin2; ++inicio2)
        {
            printf("[%d]",*inicio2);
        }
        printf("\n");
    }
}

void gestor(int senial)
{
    impresionTablero(matrix);
}

