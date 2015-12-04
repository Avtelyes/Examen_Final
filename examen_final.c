//
//  main.c
//  mpi_basico
//
//  Created by Josue Garcia Puig on 04/12/15.
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
void encuentraCamino(int, int, int, int, int);
void gestor(int);

int **pool2;
int **pool3;
int **pool4;

int tamPool2;
int tamPool3;
int tamPool4;

int **creacionPool();

void desplazo(int, int, int, int, int);

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
    pool2 = creacionPool();
    pool3 = creacionPool();
    pool4 = creacionPool();
    
    tamPool2 = 0;
    tamPool3 = 0;
    tamPool4 = 0;
    
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
    
    printf("[%d][%d]\n[%d][%d]\n[%d][%d]\n", pool2[0][0], pool2[0][1], pool2[1][0], pool2[1][1], pool2[2][0], pool2[2][1]);
    printf("Tamaño del pool %d\n", tamPool2);
    
    //raise(SIGUSR1);
    //impresionTablero(matrix);
    kill(Principal, SIGALRM);
    
    for(i=0; i<N; ++i)
        free(*(matrix+i));
    
    free(matrix);
    free(vecinos);
    free(cuadrantes);
    free(espera_t);
    free(pool2);
    free(pool3);
    free(pool4);
    
    
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
        encuentraCamino(0, 0, 1, 0, 0);
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+1);
    }
    else if (id == 2)
    {
        pthread_mutex_lock(cuadrantes+2);
        if(tamPool2 < 0)
        {
            pthread_cond_wait((espera_t+2), (cuadrantes+2));
            //wait(1);
        }
        printf("Hola soy el proceso 2\n");
        sleep(1);
        //printf("Iniciando en la coordena %d,%d\n",*(*(pool2+0)+0),*(*(pool2+0)+1));
        int i = 0;
        for(; i<tamPool2; ++i)
        {
            int num1 = *(*(pool2+0)+0);
            int num2 = *(*(pool2+0)+1);
            encuentraCamino(num1, num2, 2, 0, 4);
        }
        
        //encuentraCamino(2, 4, 5);
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+2);
    }
    else if (id == 3)
    {
        pthread_mutex_lock(cuadrantes+3);
        if(tamPool3 < 0)
        {
            pthread_cond_wait((espera_t+3), (cuadrantes+3));
            //wait(1);
        }
        printf("Hola soy el proceso 3\n");
        sleep(1);
        int i = 0;
        for(; i<tamPool3; ++i)
        {
            int num1 = *(*(pool3+0)+0);
            int num2 = *(*(pool3+0)+1);
            encuentraCamino(num1, num2, 3, 4, 0);
        }
        //impresionTablero(matrix);
        pthread_mutex_unlock(cuadrantes+3);
    }
    else if (id == 4)
    {
        pthread_mutex_lock(cuadrantes+4);
        if(tamPool4 < 0)
        {
            pthread_cond_wait((espera_t+4), (cuadrantes+4));
            //wait(1);
        }
        printf("Hola soy el proceso 4\n");
        sleep(1);
        int i = 0;
        for(; i<tamPool4; ++i)
        {
            int num1 = *(*(pool4+0)+0);
            int num2 = *(*(pool4+0)+1);
            encuentraCamino(num1, num2, 4, 4, 4);
        }
        //impresionTablero(matrix);
        if(*(*(matrix+N-1)+N-1) == 0)
            *(*(matrix+N-1)+N-1) = 8;
        pthread_mutex_unlock(cuadrantes+4);
    }
    
    pthread_exit(NULL);
}

void encuentraCamino(int in, int fn, int num, int iX, int iY)
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
    
    /*int derecha = 0, izquierda = 1, abajo = 2, arriba = 3;
     
     
     
     int i, j;
     for(i=0; i<4; ++i)
     {
     for(j=0; j<4; ++j)
     {
     if(*(*(matrix+in+i)+j+fn) != 1)
     *(*(matrix+in+i)+j+fn) = num;
     }
     }*/
    
    desplazo(in, fn, iX, iY, num);
    
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

int **creacionPool()
{
    int **pool;
    int i;
    
    pool = (int **) malloc(8 * sizeof(int *));
    
    for(i = 0; i<N; ++i)
        *(pool+i) = (int *) malloc(2 * sizeof(int));
    
    return pool;
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

void desplazo(int x, int y, int inicioX, int inicioY, int numCuadrante)
{
    if (*(*(matrix+x)+y) == 1)
    {
        return;
    }
    
    
    if (*(*(matrix+x)+y) == 0)
    {
        *(*(matrix+x)+y) = 8;
        desplazo(x, y, inicioX, inicioY, numCuadrante);
    }
    
    if(x >= inicioX+3)
    {
        if(numCuadrante != 3 && numCuadrante != 4)
        {
            if(*(*(matrix+x+1)+y) != 1)
            {
                //printf("Coordenadas: [%d][%d]\n", x, y);
                if(numCuadrante == 1)
                {
                    tamPool3++;
                    *(*(pool3+tamPool3-1)+0) = x;
                    *(*(pool3+tamPool3-1)+1) = y+1;
                    pthread_cond_broadcast((espera_t+3));
                }
            }
        }
        
        return;
    }
    
    if (y >= inicioY+3)
    {
        //printf("Coordenadas: [%d][%d]\n", x, y);
        if(numCuadrante == 1)
        {
            tamPool2++;
            *(*(pool2+tamPool2-1)+0) = x;
            *(*(pool2+tamPool2-1)+1) = y+1;
            pthread_cond_broadcast((espera_t+2));
        }
        if(numCuadrante == 3)
        {
            tamPool4++;
            *(*(pool4+tamPool4-1)+0) = x;
            *(*(pool4+tamPool4-1)+1) = y+1;
            pthread_cond_broadcast((espera_t+4));
        }
        return;
    }
    
    if(*(*(matrix+x+1)+y) == 0)
        desplazo(x+1, y, inicioX, inicioY, numCuadrante);
    
    if(*(*(matrix+x)+y+1) == 0)
        desplazo(x, y+1, inicioX, inicioY, numCuadrante);
    
}

