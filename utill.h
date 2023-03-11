#ifndef UTILL
#define UTILL

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MASTER 0

struct Element
{
    int id;
    int n;
    int **matrix;
    int count;
};
typedef struct Element Element;
struct Result
{
    int picID;
    int patID;
    int row;
    int col;
    double value;
};
typedef struct Result Result;
struct Manager
{
    double matchingValueFromFile;
    int num_pictures;
    Element *pictures;
    int num_patterns;
    Element *patterns;
    Result *results;
    int resultsCount;
};
typedef struct Manager Manager;

#endif