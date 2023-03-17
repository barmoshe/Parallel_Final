#include "utill.h"

double getDiff(int p, int o)
{
    return abs((p - o) / p);
}
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
        for (int pCol = 0; pCol < pat.n; pCol++)
        {
            matching += getDiff(picture.matrix[row][col], pat.matrix[pRow][pCol]);
            // printf("pat number : %d\t%d,%d\n",pat.id,row,col);
            col++;
        }
    }
    return matching / (pat.n * pat.n);
}
int *getMatchingResultSlaves(Manager *m, int picID)
{
    int *results = (int *)malloc(sizeof(int) * 3 * 4);
    int resultCount = 0;
    for (int pat = 0; pat < m->num_patterns; pat++)
    {
        printf("pattern %d\n", pat + 1);
        for (int i = 0; i < m->pictures[picID - 1].n; i++)
        {
            for (int j = 0; j < m->pictures[picID - 1].n; j++)
            {
                if (resultCount > 2)
                {
                    printf("in func Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", results[0], results[1], results[2], results[3], results[5], results[6], results[7], results[9], results[10], results[11]);
                    return results;
                }
                double matching = getMatchingInPlace(i, j, m->pictures[picID - 1], m->patterns[pat]);
                if (matching <= m->matchingValueFromFile)
                {
                    // printMatrixWithin(i, j, m->pictures[pic], m->patterns[pat]->n);//test print
                    results[resultCount * 4] = m->pictures[picID - 1].id;
                    results[resultCount * 4 + 1] = m->patterns[pat].id;
                    results[resultCount * 4 + 2] = i;
                    results[resultCount * 4 + 3] = j;
                    printf("in func Picture %d: found Object %d in [%d][%d]\n", results[resultCount * 4], results[resultCount * 4 + 1], results[resultCount * 4 + 2], results[resultCount * 4 + 3]);
                    resultCount++;
                }
            }
        }
    }
    results[0] = picID;
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
void printResultstoFile(int *results, FILE *file)
{
    if (results[1] != -1)
    {
        fprintf(file, "Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", results[0], results[1], results[2], results[3], results[5], results[6], results[7], results[9], results[10], results[11]);
        printf("printed to file:\n");
        printf("in func Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", results[0], results[1], results[2], results[3], results[5], results[6], results[7], results[9], results[10], results[11]);
    }
    else
    {
        fprintf(file, "Picture %d: no objects found\n", results[0]);
        printf("printed to file:\n");
        printf("in func Picture %d: no objects found\n", results[0]);
    }
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
    void *buf;
    int total_size = 0;
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    Manager m = {0};
    int nextPicIdToCalculate = size;

    if (rank == 0)

    { // master process
      // initialize the Manager struct

        Manager m;
        FILE *file = fopen("input.txt", "r");
        m = readManager(file);

        // pack the Manager struct into a buffer
        MPI_Aint matching_value_size, num_pictures_size, num_patterns_size;
        MPI_Type_get_extent(MPI_DOUBLE, &matching_value_size, &matching_value_size);
        MPI_Type_get_extent(MPI_INT, &num_pictures_size, &num_pictures_size);
        MPI_Type_get_extent(MPI_INT, &num_patterns_size, &num_patterns_size);

        MPI_Aint pictures_size = 0, patterns_size = 0;
        for (int i = 0; i < m.num_pictures; i++)
        {
            pictures_size += sizeof(int) + sizeof(int) + m.pictures[i].n * m.pictures[i].n * sizeof(int);
        }
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
        printf("rank %d: unpacked Manager struct from buffer\n", rank);
        int current = rank;
        // get results for the RANK id picture
        int *resultsArr = (int *)malloc(sizeof(int) * 4 * 3); // 4 ints per result, 3 results
        resultsArr = getMatchingResultSlaves(&m, current);

        // send resultsArr to master
        MPI_Send(resultsArr, 4 * 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
        // while there is still work to do, get the next picture id from master
        while (1)
        {
            MPI_Recv(&current, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if (current <= m.num_pictures)
            {
                resultsArr = getMatchingResultSlaves(&m, current);
                MPI_Send(resultsArr, 4 * 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
            else
                break;
        }
    }
    else
    {
        int *resultsArr = (int *)malloc(sizeof(int) * 4 * 3);
        FILE *fp;
        fp = fopen("Output.txt", "w");
        if (fp == NULL)
        {
            printf("Error opening file!");
            exit(1);
        }
        while (nextPicIdToCalculate <= m.num_pictures)
        {
            printf("test\n");
            // receive the results from the slave process
            MPI_Recv(resultsArr, 4 * 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            printf("in main Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", resultsArr[0], resultsArr[1], resultsArr[2], resultsArr[3], resultsArr[5], resultsArr[6], resultsArr[7], resultsArr[9], resultsArr[10], resultsArr[11]);
            printResultstoFile(resultsArr, fp);
            // send the next picture to the slave process if there is still work to do
            MPI_Send(&nextPicIdToCalculate, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            nextPicIdToCalculate++;
        }
    }

    MPI_Finalize();
    return 0;
}