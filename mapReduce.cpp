
#include "solver.h"
#include <fstream>
#include <pthread.h>
#include <unordered_set>

/**
 * @brief Calculeza a n-a radacina intreaga
 * 
 * @param x 
 * @param n 
 * @return -1, daca a n-a radacina nu e intreaga\n
 *         x^(1/n), daca a n-a radacina e intreaga 
 */
int findNthRoot(int x, int n)
{  
 
    int low = 1;
    int high = x;
 
    // binary search
    int guess = (low + high) / 2;
    while(low <= high)
    {   
        long long p = 1;
        for(int i=0; i<n; i++){
            p*=guess;
            // daca depaseste x, atunci inmultind tot va depasi x
            if(p > x)
            break;
        }
        if(p == x)
            return guess; // integer root found
        else if(p > x)
            high = guess - 1;
        else
            low = guess + 1;
        guess = (low + high) / 2;
    }
    // integer root not found
    return -1;
}


std::vector<void *> *map(int reducerCount, const char *inputFileName){
    
    // initializam vectorul de set-uri
    // in fiecare set-ul i punem valorile ce sunt puteri perfecte ale lui i+2
    // folosim set pentru a gasi valorile unice din fisier
    std::vector<std::unordered_set<int> *> perfectPowers(reducerCount);
    for(int i = 0; i < reducerCount; i++){
        perfectPowers[i] = new std::unordered_set<int>();
    }

    std::ifstream fin(inputFileName);
    // citim din fisier
    int n,x;
    fin >> n;
    for(int i = 0; i < n; i++){
        fin >> x;
        for(int j = 0; j < reducerCount; j++){
            int r = findNthRoot(x, j + 2);
            if(r != -1){
                perfectPowers[j]->insert(x);
            }
        }
    }
    fin.close();
    
    // intoarcem valoarea partiala obtinuta pentru fiecare reducer
    std::vector<void *> *partialResults = new std::vector<void *>(reducerCount);
    for(int i = 0; i < reducerCount; i++){  
        (*partialResults)[i] = (void *)perfectPowers[i];
    }  
    return partialResults;
}

void reduce(int ID, std::vector<void *> *data){
    
    // punem toate numerele obtinute intr-un set pentru a obtine valorile unice
    std::unordered_set<int> finalValues;
    for(void *x: (*data)){
        std::unordered_set<int> *set = (std::unordered_set<int> *)x;
        for(int val : (*set)){
            finalValues.insert(val);
        }
        // sterge valoarea partiala 
        delete set;
    }

    char buffer[100];
    sprintf(buffer, "out%d.txt", ID + 2);
    std::ofstream fout(buffer);
    // scriem fisier numarul de valori unice
    fout << finalValues.size();


}