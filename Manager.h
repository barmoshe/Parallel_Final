#ifndef MANAGER_H
#define MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>

struct Element
{
    int id;
    int n;
    int **matrix;
};

struct Manager
{
    double matching_value;
    int num_pictures;
    struct Element *pictures;
    int num_patterns;
    struct Element *patterns;
};

enum methodology getMethod();

double getDiff(int p, int o);

struct Element readElement(FILE *file);

struct Manager readManager(FILE *file);

void printElement(struct Element element);

void printManagerInfo(struct Manager manager);

double getMatchingInPlace(int row, int col, struct Element picture, struct Element pat);
int getMatchingPartial(struct Manager m, int rank);
int getMatching(struct Manager m);

#endif