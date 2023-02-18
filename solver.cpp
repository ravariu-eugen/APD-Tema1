#include "solver.h"
#include "mapReduce.h"
#include <fstream>
#include <iostream>


Solver::Solver(
            int mapperCount, int reducerCount, 
            char *task_file, 
            MAPFUNC mapFunction, REDUCEFUNC reduceFunction
            ){
    this->mapperCount = mapperCount;
    this->reducerCount = reducerCount;
    this->mapFunction = mapFunction;
    this->reduceFunction = reduceFunction;

    // citire nume fisiere
    std::ifstream fin(task_file);
    int task_count;
    fin >> task_count;
    std::string s;
    for(int i = 0; i < task_count; i++){
        fin >> s;
        task_files.push_back(s);
    }
    fin.close();
    this->current_task = task_files.begin();

    // initializare vector si array
    for(int i = 0; i < reducerCount; i++){
        partialResults.push_back(new std::vector<void*>());
    }
    mappers = (pthread_t *)calloc(mapperCount, sizeof(pthread_t));
    reducers = (pthread_t *)calloc(reducerCount, sizeof(pthread_t));
    partialResult_mutex = (pthread_mutex_t *)calloc(reducerCount, sizeof(pthread_mutex_t));
 
    // initializare mutex si bariera
    pthread_mutex_init(&getTaskFile_mutex, NULL);
    pthread_barrier_init(&finishMap_barrier, NULL, mapperCount + reducerCount);
    for(int i = 0; i < reducerCount; i++){
        pthread_mutex_init(&partialResult_mutex[i], NULL);
    }
    
}

/**
 * @brief obtine numele unui fisier neprocesat
 * 
 * @return const char* \n numele fiserului daca mai exista un fisier 
 *                      \n null daca nu mai exista  
 */
const char *Solver::getTaskFile(){
    const char *ans;
    pthread_mutex_lock(&getTaskFile_mutex);
    if(current_task == task_files.end()){
        ans = NULL;
    }else{
        ans = (*current_task).c_str();
        current_task++;
    }
    pthread_mutex_unlock(&getTaskFile_mutex);
    return ans;
}

Solver::~Solver(){
    // distrugere mutex si bariera
    pthread_barrier_destroy(&finishMap_barrier);
    
    pthread_mutex_destroy(&getTaskFile_mutex);
    for(int i = 0; i < reducerCount; i++){
        pthread_mutex_destroy(&partialResult_mutex[i]);
    }

    // eliberare memorie
    for(auto partialVector : partialResults)
    {
        delete partialVector;
    }
    free(partialResult_mutex);
    free(mappers);
    free(reducers);    
}

int Solver::run() {
    // creare thread-uri
    threadArg_t mapArgs[mapperCount], reduceArgs[reducerCount];
    for(int i = 0; i < mapperCount; i++) {
        mapArgs[i].ID = i;
        mapArgs[i].solver = this;
        pthread_create(&mappers[i], NULL, mapThread, &mapArgs[i]);
    }
    for(int i = 0; i < reducerCount; i++) {
        reduceArgs[i].ID = i;
        reduceArgs[i].solver = this;
        pthread_create(&reducers[i], NULL, reduceThread, &reduceArgs[i]);
    }

    // asteptare thread-uri
    void *status;
    int r = 0;
    for(int i = 0; i < mapperCount; i++) {
        pthread_join(mappers[i], &status);
        if(status != (void *)0)
            r = 1;
    }
    for(int i = 0; i < reducerCount; i++) {
        pthread_join(reducers[i], &status);
        if(status != (void *)0)
            r = 1;
    }
    return r;
}

/**
 * @brief adauga un rezultat partial pentru reducer-ul reducerID
 * 
 */
void Solver::addPartialResult(int reducerID, void *data){
    pthread_mutex_lock(&partialResult_mutex[reducerID]);
    
    partialResults[reducerID]->push_back(data);

    pthread_mutex_unlock(&partialResult_mutex[reducerID]);
}
/**
 * @brief intoarce vectorul de date partiale pentru reducer-ul reducerID
 * 
 * @param reducerID 
 * @return std::vector<void *>* 
 */
std::vector<void *> *Solver::getPartialResult(int reducerID){
    return partialResults[reducerID];
}

int Solver::getReducerCount(){
    return reducerCount;
}

MAPFUNC Solver::getMapFunction(){
    return mapFunction;
}

REDUCEFUNC Solver::getReduceFunction(){
    return reduceFunction;
}

/**
 * @brief bariera la care astepta thread-urile
 * 
 */
void Solver::barrier(){
    pthread_barrier_wait(&finishMap_barrier);
}

/**
 * @brief thread pentru MAP
 * 
 */
void * mapThread(void *arg){

    threadArg_t * threadArg = (threadArg_t *)arg;
    Solver *solver = threadArg->solver;
    MAPFUNC map = solver->getMapFunction();
    while(true){
        // obtine urmatorul fisier
        const char *next_task_file = solver->getTaskFile();
        if(next_task_file == NULL)
            break; // daca nu mai sunt fisiere, termina thread-ul
        
        // ruleaza functia de MAP
        int reducerCount = solver->getReducerCount();
        std::vector<void *> *partialData = map(reducerCount, next_task_file);
        for(int i = 0; i < reducerCount; i++){
            // adauga rezultatele partiale obtine
            solver->addPartialResult(i, partialData->at(i));
        }
        delete partialData; 
    }

    // asteapta sa se termine toti mapperii
    solver->barrier();
    return (void *)0;
}
/**
 * @brief thread pentru REDUCE
 * 
 */
void * reduceThread(void *arg){
    threadArg_t * threadArg = (threadArg_t *)arg;
    Solver *solver = threadArg->solver;
    int ID = threadArg->ID;

    // astepta sa se termine toti mapperii
    solver->barrier();

    // ruleaza functia de reduce
    REDUCEFUNC reduce = solver->getReduceFunction();
    reduce(ID, solver->getPartialResult(ID));
    // sterge datele partiale primite
    
    return (void *)0;
}