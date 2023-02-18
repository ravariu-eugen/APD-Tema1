#pragma once
#include <vector>


std::vector<void *> *map(int reducerCount, const char *inputFileName);

void reduce(int ID, std::vector<void *> *data);

