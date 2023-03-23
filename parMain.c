#include "utill.h"

// This function calculates the difference between two numbers as a percentage.
// The numbers are passed in as parameters, p and o.
// The function returns the difference between the two numbers as a percentage.
double getDiff(int p, int o)
{
    return abs((p - o) / p);
}

// This function calculates the matching percentage between picture and pattern.
double getMatchingInPlace(int row, int col, struct Element picture, struct Element pat)
{
    int startCol = col;
    int startRow = row;
    if (col + pat.n >= picture.n)
        return 1.0;
    if (row + pat.n >= picture.n)
        return 1.0;
    double matching = 0.0;
    for (int pRow = 0; pRow < pat.n; pRow++)
    {
        col = startCol;
        row = startRow + pRow;
        //#pragma omp parallel for reduction(+:matching)
        for (int pCol = 0; pCol < pat.n; pCol++)
        {
            matching += getDiff(picture.matrix[row][col], pat.matrix[pRow][pCol]);
            col++;
        }
    }
    return matching / (pat.n * pat.n);
}
int *getMatchingResultSlaves(Manager *m, int picID)
// This function calculates the matching percentage between specific picture and the patterns.
{
    int *results = (int *)malloc(sizeof(int) * 3 * 4);
    int resultCount = 0;
#pragma omp parallel for shared(resultCount)
    for (int pat = 0; pat < m->num_patterns; pat++)
    {
        for (int i = 0; i < m->pictures[picID - 1].n; i++)
        {
                           
            for (int j = 0; j < m->pictures[picID - 1].n; j++)
            {
                if (resultCount <= 2)
                { 
                    double matching = getMatchingInPlace(i, j, m->pictures[picID - 1], m->patterns[pat]);
                    if (matching <= m->matchingValueFromFile)
                    {
                        results[resultCount * 4] = m->pictures[picID - 1].id;
                        results[resultCount * 4 + 1] = m->patterns[pat].id;
                        results[resultCount * 4 + 2] = i;
                        results[resultCount * 4 + 3] = j;
                        resultCount++;
                        pat++;
                        i = 0;
                        j = 0;
                        
                    }
                }
            }
        }
    }
    results[0] = picID;
     if (resultCount <= 2)
                
    for (int i = 1; i < 12; i++)
    {
        results[i] = -1;
    }
    return results;
}

void printElement(struct Element element)
{
    printf("ID: %d\n", element.id);
    printf("Size: %d\n", element.n);
}

void printManagerInfo(struct Manager manager)
{
    printf("Matching Value: %lf\n", manager.matchingValueFromFile);
    printf("%d Pictures:\n", manager.num_pictures);
    for (int i = 0; i < manager.num_pictures; i++)
    {
        printf("Picture %d:\n", i + 1);
        printElement(manager.pictures[i]);
        printf("-------\n");
    }
    printf("%d Patterns:\n", manager.num_patterns);
    for (int i = 0; i < manager.num_patterns; i++)
    {
        printf("Pattern %d:\n", i + 1);
        printElement(manager.patterns[i]);
        printf("-------\n");
    }
}
Element readElement(FILE *file)
{
    struct Element element;

    fscanf(file, "%d", &element.id);
    fscanf(file, "%d", &element.n);

    element.matrix = (int **)malloc(element.n * sizeof(int *));

    for (int i = 0; i < element.n; i++)
    {
        element.matrix[i] = (int *)malloc(element.n * sizeof(int));
        for (int j = 0; j < element.n; j++)
        {
            fscanf(file, "%d ", &element.matrix[i][j]);
        }
    }

    return element;
}

Manager readManager(FILE *file)
{
    struct Manager manager;

    fscanf(file, "%lf", &manager.matchingValueFromFile);
    fscanf(file, "%d", &manager.num_pictures);
    manager.pictures = (struct Element *)malloc(manager.num_pictures * sizeof(struct Element));
    for (int i = 0; i < manager.num_pictures; i++)
    {
        manager.pictures[i] = readElement(file);
    }

    fscanf(file, "%d", &manager.num_patterns);
    manager.patterns = (struct Element *)malloc(manager.num_patterns * sizeof(struct Element));
    for (int i = 0; i < manager.num_patterns; i++)
    {
        manager.patterns[i] = readElement(file);
    }

    return manager;
}

int main(int argc, char *argv[])
{
    clock_t start = clock();
    void *buf;
    int total_size = 0;
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    Manager m = {0};
     int num_pictures = 0;

    if (rank == 0)

    { // master process
      // initialize the Manager struct

        Manager m;
        FILE *file = fopen("input.txt", "r");
        m = readManager(file);
        num_pictures = m.num_pictures;
        // pack the Manager struct into a buffer
        MPI_Aint matching_value_size, num_pictures_size, num_patterns_size;
        MPI_Type_get_extent(MPI_DOUBLE, &matching_value_size, &matching_value_size);
        MPI_Type_get_extent(MPI_INT, &num_pictures_size, &num_pictures_size);
        MPI_Type_get_extent(MPI_INT, &num_patterns_size, &num_patterns_size);

        MPI_Aint pictures_size = 0, patterns_size = 0;
        #pragma omp parallel for reduction(+:pictures_size)
        for (int i = 0; i < m.num_pictures; i++)
        {
            pictures_size += sizeof(int) + sizeof(int) + m.pictures[i].n * m.pictures[i].n * sizeof(int);
        }
        #pragma omp parallel for reduction(+:patterns_size)
        for (int i = 0; i < m.num_patterns; i++)
        {
            patterns_size += sizeof(int) + sizeof(int) + m.patterns[i].n * m.patterns[i].n * sizeof(int);
        }

        total_size = matching_value_size + num_pictures_size + pictures_size + num_patterns_size + patterns_size;
        buf = malloc(total_size);
        int pos = 0;

        MPI_Pack(&m.matchingValueFromFile, 1, MPI_DOUBLE, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m.num_pictures, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);

        for (int i = 0; i < m.num_pictures; i++)
        {
            MPI_Pack(&m.pictures[i].id, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            MPI_Pack(&m.pictures[i].n, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            for (int j = 0; j < m.pictures[i].n; j++)
            {
                MPI_Pack(m.pictures[i].matrix[j], m.pictures[i].n, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            }
        }

        MPI_Pack(&m.num_patterns, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);

        for (int i = 0; i < m.num_patterns; i++)
        {
            MPI_Pack(&m.patterns[i].id, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            MPI_Pack(&m.patterns[i].n, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            for (int j = 0; j < m.patterns[i].n; j++)
            {
                MPI_Pack(m.patterns[i].matrix[j], m.patterns[i].n, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            }
        }
        MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
    {

        MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        buf = malloc(total_size);
    }

    // send the packed data to the slave process
    MPI_Bcast(buf, total_size, MPI_PACKED, 0, MPI_COMM_WORLD);
    // unpack the Manager struct from the buffer in slave process
    if (rank != 0)
    {
        int pos = 0;
        MPI_Unpack(buf, total_size, &pos, &m.matchingValueFromFile, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(buf, total_size, &pos, &m.num_pictures, 1, MPI_INT, MPI_COMM_WORLD);
        m.pictures = malloc(m.num_pictures * sizeof(struct Element));
        for (int i = 0; i < m.num_pictures; i++)
        {
            MPI_Unpack(buf, total_size, &pos, &m.pictures[i].id, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buf, total_size, &pos, &m.pictures[i].n, 1, MPI_INT, MPI_COMM_WORLD);
            m.pictures[i].matrix = malloc(m.pictures[i].n * sizeof(int *));
            for (int j = 0; j < m.pictures[i].n; j++)
            {
                m.pictures[i].matrix[j] = malloc(m.pictures[i].n * sizeof(int));
                MPI_Unpack(buf, total_size, &pos, m.pictures[i].matrix[j], m.pictures[i].n, MPI_INT, MPI_COMM_WORLD);
            }
        }

        MPI_Unpack(buf, total_size, &pos, &m.num_patterns, 1, MPI_INT, MPI_COMM_WORLD);

        m.patterns = malloc(m.num_patterns * sizeof(struct Element));
        for (int i = 0; i < m.num_patterns; i++)
        {
            MPI_Unpack(buf, total_size, &pos, &m.patterns[i].id, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buf, total_size, &pos, &m.patterns[i].n, 1, MPI_INT, MPI_COMM_WORLD);
            m.patterns[i].matrix = malloc(m.patterns[i].n * sizeof(int *));
            for (int j = 0; j < m.patterns[i].n; j++)
            {
                m.patterns[i].matrix[j] = malloc(m.patterns[i].n * sizeof(int));
                MPI_Unpack(buf, total_size, &pos, m.patterns[i].matrix[j], m.patterns[i].n, MPI_INT, MPI_COMM_WORLD);
            }
        }

        free(buf);
        printf("\nrank %d: unpacked Manager struct from buffer\n", rank);
    }
    if (rank == 0)
    {
        FILE *fp;
        fp = fopen("Output.txt", "w");
        if (fp == NULL)
        {
            printf("Error opening file!");
            exit(1);
        }

        // recive num_pictures results arrays
        for (int i = 0; i < num_pictures; i++)
        {
            int *resultsArr = (int *)malloc(sizeof(int) * 4 * 3);
            MPI_Recv(resultsArr, 12, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
             //printf("in main Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", resultsArr[0], resultsArr[1], resultsArr[2], resultsArr[3], resultsArr[5], resultsArr[6], resultsArr[7], resultsArr[9], resultsArr[10], resultsArr[11]);
            if (resultsArr[1] != -1)
                fprintf(fp, "Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d] \n", resultsArr[0], resultsArr[1], resultsArr[2], resultsArr[3], resultsArr[5], resultsArr[6], resultsArr[7], resultsArr[9], resultsArr[10], resultsArr[11]);
            else
                fprintf(fp, "Picture %d: found less then 3 or  no objects\n", resultsArr[0]);
        }
        fclose(fp);
        clock_t end = clock();
        double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
        printf("time spent: %f\n", time_spent);
    }
    else
    {
        int *resultsArr = (int *)malloc(sizeof(int) * 4 * 3);
        for (int i = rank - 1; i < m.num_pictures; i += size - 1)
        {
            resultsArr = getMatchingResultSlaves(&m, i + 1);
             //printf("rank %d Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", rank, resultsArr[0], resultsArr[1], resultsArr[2], resultsArr[3], resultsArr[5], resultsArr[6], resultsArr[7], resultsArr[9], resultsArr[10], resultsArr[11]);
            //  send the results to the master process
            MPI_Send(resultsArr, 12, MPI_INT, 0, resultsArr[0] - 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
