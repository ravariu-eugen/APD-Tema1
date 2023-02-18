#include <string>
#include <vector>
#include <pthread.h>



typedef std::vector<void *> *(*MAPFUNC)(int reducerCount, const char *inputFileName);
typedef void (*REDUCEFUNC)(int ID, std::vector<void *> *data);



class Solver{
    public:
        Solver(int mapperCount, int reducerCount, char *task_file, MAPFUNC mapFunction, REDUCEFUNC reduceFunction);
        ~Solver();
        int run();
        int getReducerCount();
        const char *getTaskFile();
        void addPartialResult(int reducerID, void *data);
        std::vector<void *> *getPartialResult(int reducerID);
        MAPFUNC getMapFunction();
        REDUCEFUNC getReduceFunction();
        void barrier();

    private:
        int mapperCount, reducerCount;

        std::vector<std::string> task_files;
        std::vector<std::string>::iterator current_task;
        std::vector<std::vector<void *> *> partialResults;
        pthread_t *mappers, *reducers;
        MAPFUNC mapFunction;
        REDUCEFUNC reduceFunction;
        pthread_barrier_t finishMap_barrier;
        pthread_mutex_t getTaskFile_mutex;
        pthread_mutex_t *partialResult_mutex;
        
        

};

typedef struct{
    int ID;
    Solver *solver;
} threadArg_t;

void *mapThread(void *arg);
void *reduceThread(void *arg);





