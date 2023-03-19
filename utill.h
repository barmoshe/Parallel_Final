#ifndef UTILL
#define UTILL

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>

#define MASTER 0

struct Element
{
    int id;
    int n;
    int **matrix;
};
typedef struct Element Element;
struct Manager
{
    double matchingValueFromFile;
    int num_pictures;
    Element *pictures;
    int num_patterns;
    Element *patterns;
};
typedef struct Manager Manager;

#endif