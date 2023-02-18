#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "solver.h"
#include "mapReduce.h"

int main(int argc, char *argv[])
{

    if(argc < 4) {
        printf("Numar insuficient de parametri: ./tema1 mapCount reducerCount cale\n");
        exit(1);
    }

    int mapCount = atoi(argv[1]);
    int reducerCount = atoi(argv[2]);
    char *path = argv[3];
    
    
    Solver s(mapCount, reducerCount, path, map, reduce);
    s.run();

    
    return 0;
}
