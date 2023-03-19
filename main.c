#include "utill.h"

// Function to print output for all pictures in the results array to a file
void printResultsToFile(Result *results, int resultsCount, int num_pictures)
{
    FILE *fp;
    fp = fopen("Output.txt", "w");
    int picturesInfo[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 1; i <= num_pictures; i++)
    {
        int count = 0;
        int index = 0;
        for (int j = 0; j < resultsCount; j++)
        {
            if (results[j].picID == i)
            {
                picturesInfo[index] = results[j].patID;
                picturesInfo[index + 1] = results[j].row;
                picturesInfo[index + 2] = results[j].col;
                index += 3;
                count++;
            }
        }
        if (count >= 3)
        {
            // print to file
            fprintf(fp, "Picture %d: found Object %d in [%d][%d] , Object %d in [%d][%d] , Object %d in [%d][%d]\n", i, picturesInfo[0], picturesInfo[1], picturesInfo[2], picturesInfo[3], picturesInfo[4], picturesInfo[5], picturesInfo[6], picturesInfo[7], picturesInfo[8]);
        }
        else
        {
            fprintf(fp, "picture %d: no objects found||less then 3 objects found\n", i);
        }
    }
    fclose(fp);
}

double getDiff(int p, int o)
{
    return abs((p - o) / p);
}

void printResult(struct Result result)
{
    printf("Picture %d: found Object %d in [%d][%d]\twith matching: %lf \t \t\n", result.picID, result.patID, result.col, result.row, result.value);
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
struct Element readElement(FILE *file)
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
void initResult(Result *result, int size)
{
    for (int i = 0; i < size; i++)
    {
        result[i].picID = 0;
        result[i].patID = 0;
        result[i].row = 0;
        result[i].col = 0;
        result[i].value = 0.2;
    }
}
struct Manager readManager(FILE *file)
{
    struct Manager manager;

    fscanf(file, "%lf", &manager.matchingValueFromFile);
    fscanf(file, "%d", &manager.num_pictures);
    manager.pictures = (struct Element *)malloc(manager.num_pictures * sizeof(struct Element));
    for (int i = 0; i < manager.num_pictures; i++)
    {
        manager.pictures[i] = readElement(file);
        manager.pictures[i].count = 0;
    }

    fscanf(file, "%d", &manager.num_patterns);
    manager.patterns = (struct Element *)malloc(manager.num_patterns * sizeof(struct Element));
    for (int i = 0; i < manager.num_patterns; i++)
    {
        manager.patterns[i] = readElement(file);
        manager.patterns[i].count = 0;
    }
    manager.results = (struct Result *)malloc(3 * manager.num_pictures * sizeof(struct Result));
    initResult(manager.results, 3 * manager.num_pictures);
    manager.resultsCount = 0;
    return manager;
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
// printMatrixWithin pretty prints the matrix within the given range
void printMatrixWithin(int row, int col, struct Element picture, int n)
{
    int startCol = col;
    int startRow = row;
    for (int pRow = 0; pRow < n; pRow++)
    {
        col = startCol;
        row = startRow + pRow;
        for (int pCol = 0; pCol < n; pCol++)
        {
            printf("[%03d] ", picture.matrix[row][col]);
            col++;
        }
        printf("\n");
    }
}
void printPictureAndCount(struct Element picture)
{
    printf("Picture %d:\n", picture.id);
    printf("Count: %d\n", picture.count);
}
Result *getMatchingResult(Manager *m)
{
    Result *results = (Result *)malloc(sizeof(Result) * 3 * m->num_pictures);

    for (int pic = 0; pic < m->num_pictures; pic++)
    {
        for (int pat = 0; pat < m->num_patterns; pat++)
        {
            if (m->pictures[pic].count < 3)
                for (int i = 0; i < m->pictures[pic].n; i++)
                {
                    for (int j = 0; j < m->pictures[pic].n; j++)
                    {
                        double matching = getMatchingInPlace(i, j, m->pictures[pic], m->patterns[pat]);
                        if (matching <= m->matchingValueFromFile)
                        {
                            // printMatrixWithin(i, j, m->pictures[pic], m->patterns[pat]->n);//test print
                            results[m->resultsCount].picID = m->pictures[pic].id;
                            results[m->resultsCount].patID = m->patterns[pat].id;
                            results[m->resultsCount].col = i;
                            results[m->resultsCount].row = j;
                            results[m->resultsCount].value = matching;
                            m->pictures[pic].count++;
                            printPictureAndCount(m->pictures[pic]);

                            // printResult(results[m.resultsCount]);
                            m->resultsCount = m->resultsCount + 1;
                        }
                    }
                }
        }
    }
    return results;
}
Result *getMatchingResultSlaves(Manager *m, int picID)
{
    Result *results = (Result *)malloc(sizeof(Result) * 3 * m->num_pictures);

    for (int pat = 0; pat < m->num_patterns; pat++)
    {
        if (m->pictures[picID - 1].count < 3)
            for (int i = 0; i < m->pictures[picID - 1].n; i++)
            {
                for (int j = 0; j < m->pictures[picID - 1].n; j++)
                {
                    double matching = getMatchingInPlace(i, j, m->pictures[picID - 1], m->patterns[pat]);
                    if (matching <= m->matchingValueFromFile)
                    {
                        // printMatrixWithin(i, j, m->pictures[pic], m->patterns[pat]->n);//test print
                        results[m->resultsCount].picID = m->pictures[picID - 1].id;
                        results[m->resultsCount].patID = m->patterns[picID - 1].id;
                        results[m->resultsCount].col = i;
                        results[m->resultsCount].row = j;
                        results[m->resultsCount].value = matching;
                        m->pictures[picID - 1].count++;
                        printPictureAndCount(m->pictures[picID - 1]);

                        // printResult(results[m.resultsCount]);
                        m->resultsCount = m->resultsCount + 1;
                    }
                }
            }
    }
    return results;
}

// function to return array of managers in which each manager has a  different picture
Manager *getManagers(Manager *m)
{
    Manager *managers = (Manager *)malloc(sizeof(Manager) * m->num_pictures);
    for (int i = 0; i < m->num_pictures; i++)
    {
        managers[i] = *m;
        for (int j = 0; j < m->num_pictures; j++)
        {
            if (i != j)
            {
                managers[i].pictures[j].id = -1;
                managers[i].pictures[j].n = -1;
                managers[i].pictures[j].matrix = NULL;
            }
        }
    }
    return managers;
}
void *recivePackedManager()
{
    // get the total size of the packed data
    int total_size;
    MPI_Status status;
    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_PACKED, &total_size);

    // allocate a buffer to receive the packed data
    void *buf = malloc(total_size);

    // receive the packed data from the master
    MPI_Recv(buf, total_size, MPI_PACKED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return buf;
}
void UnpackManager(void *buf, Manager *m)
{
    int pos = 0;

    MPI_Unpack(buf, sizeof(double), &pos, &m->matchingValueFromFile, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    MPI_Unpack(buf, sizeof(int), &pos, &m->num_pictures, 1, MPI_INT, MPI_COMM_WORLD);

    m->pictures = malloc(sizeof(Element) * m->num_pictures);
    for (int i = 0; i < m->num_pictures; i++)
    {
        MPI_Unpack(buf, sizeof(int), &pos, &m->pictures[i].id, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buf, sizeof(int), &pos, &m->pictures[i].n, 1, MPI_INT, MPI_COMM_WORLD);

        m->pictures[i].matrix = malloc(sizeof(int *) * m->pictures[i].n);
        for (int j = 0; j < m->pictures[i].n; j++)
        {
            m->pictures[i].matrix[j] = malloc(sizeof(int) * m->pictures[i].n);
            MPI_Unpack(buf, sizeof(int) * m->pictures[i].n, &pos, m->pictures[i].matrix[j], m->pictures[i].n, MPI_INT, MPI_COMM_WORLD);
        }
    }

    MPI_Unpack(buf, sizeof(int), &pos, &m->num_patterns, 1, MPI_INT, MPI_COMM_WORLD);

    m->patterns = malloc(sizeof(Element) * m->num_patterns);
    for (int i = 0; i < m->num_patterns; i++)
    {
        MPI_Unpack(buf, sizeof(int), &pos, &m->patterns[i].id, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buf, sizeof(int), &pos, &m->patterns[i].n, 1, MPI_INT, MPI_COMM_WORLD);

        m->patterns[i].matrix = malloc(sizeof(int *) * m->patterns[i].n);
        for (int j = 0; j < m->patterns[i].n; j++)
        {
            m->patterns[i].matrix[j] = malloc(sizeof(int) * m->patterns[i].n);
            MPI_Unpack(buf, sizeof(int) * m->patterns[i].n, &pos, m->patterns[i].matrix[j], m->patterns[i].n, MPI_INT, MPI_COMM_WORLD);
        }
    }

    MPI_Unpack(buf, sizeof(int), &pos, &m->resultsCount, 1, MPI_INT, MPI_COMM_WORLD);

    m->results = malloc(sizeof(Result) * m->resultsCount);
    for (int i = 0; i < m->resultsCount; i++)
    {
        MPI_Unpack(buf, sizeof(int), &pos, &m->results[i].picID, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buf, sizeof(int), &pos, &m->results[i].patID, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buf, sizeof(int), &pos, &m->results[i].col, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buf, sizeof(int), &pos, &m->results[i].row, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buf, sizeof(double), &pos, &m->results[i].value, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    }
}

void *PackManager(Manager *m, int *size)
{
    // pack the Manager struct into a buffer
    MPI_Aint matching_value_size, num_pictures_size, num_patterns_size, resultsCount_size;
    MPI_Type_get_extent(MPI_DOUBLE, &matching_value_size, &matching_value_size);
    MPI_Type_get_extent(MPI_INT, &num_pictures_size, &num_pictures_size);
    MPI_Type_get_extent(MPI_INT, &num_patterns_size, &num_patterns_size);
    MPI_Type_get_extent(MPI_INT, &resultsCount_size, &resultsCount_size);
    MPI_Aint pictures_size = 0, patterns_size = 0, results_size = 0;
    for (int i = 0; i < m->num_pictures; i++)
    {
        pictures_size += sizeof(int) + sizeof(int) + sizeof(int) + (m->pictures[i].n * m->pictures[i].n * sizeof(int));
    }
    for (int i = 0; i < m->num_patterns; i++)
    {
        patterns_size += sizeof(int) + sizeof(int) + sizeof(int) + (m->patterns[i].n * m->patterns[i].n * sizeof(int));
    }
    for (int i = 0; i < m->resultsCount; i++)
    {
        results_size += (sizeof(int) * 4) + sizeof(double);
    }
    int total_size = matching_value_size + num_pictures_size + pictures_size + num_patterns_size + patterns_size + resultsCount_size + results_size;
    *size = total_size;
    void *buf = malloc(total_size);
    int pos = 0;

    MPI_Pack(&m->matchingValueFromFile, 1, MPI_DOUBLE, buf, total_size, &pos, MPI_COMM_WORLD);
    MPI_Pack(&m->num_pictures, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);

    for (int i = 0; i < m->num_pictures; i++)
    {
        MPI_Pack(&m->pictures[i].id, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m->pictures[i].n, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        for (int j = 0; j < m->pictures[i].n; j++)
        {
            MPI_Pack(m->pictures[i].matrix[j], m->pictures[i].n, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        }
    }

    MPI_Pack(&m->num_patterns, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);

    for (int i = 0; i < m->num_patterns; i++)
    {
        MPI_Pack(&m->patterns[i].id, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m->patterns[i].n, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        for (int j = 0; j < m->patterns[i].n; j++)
        {
            MPI_Pack(m->patterns[i].matrix[j], m->patterns[i].n, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        }
    }
    MPI_Pack(&m->resultsCount, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
    for (int i = 0; i < m->resultsCount; i++)
    {
        MPI_Pack(&m->results[i].picID, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m->results[i].patID, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m->results[i].col, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m->results[i].row, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m->results[i].value, 1, MPI_DOUBLE, buf, total_size, &pos, MPI_COMM_WORLD);
    }

    return buf;
}
int main(int argc, char *argv[])
{
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    int total_size = 0;
    int resultIndex = 0;
    if (size < 2)
    {
        fprintf(stderr, "This example code is meant to run with 2 processes or more\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }
    struct Manager m = {0};
    if (rank == MASTER)
    {
        Result *results = (Result *)calloc(3 * m.num_pictures, sizeof(Result));
        // Master process
        int next_i = 0;
        FILE *file = fopen("input.txt", "r");
        if (file == NULL)
        {
            printf("Error opening file \n");
        }
        m = readManager(file);
        printf("first time\n");
        printManagerInfo(m);
        fclose(file);
        // int *PictureIDs = (int *)malloc(sizeof(int) * m.num_pictures);
        // // init PictureIDs array of task ids
        // for (int i = 0; i < m.num_pictures; i++)
        // {
        //     PictureIDs[i] = m.pictures[i].id;
        // }
        // clock_t start = clock();
        // pack manager and send it to all the slaves into the variable m
        void *buf = PackManager(&m, &total_size);
        printf("total size is %d", total_size);
        // send the packed data to all the slaves
        for (int i = 1; i < size; i++)
        {
            MPI_Send(buf, total_size, MPI_PACKED, i, 0, MPI_COMM_WORLD);
        }
        // for (int i = 1; i < size; i++)
        // {
        //      MPI_Send(&next_i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        //     next_i++;
        // }
        free(buf);

        // // Receive results from slaves and send new tasks until all tasks are done
        // // while (next_i < m.num_pictures)
        // // {
        // //     Result result = {0};
        // //     MPI_Recv(&result.picID, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // //     MPI_Recv(&result.patID, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // //     MPI_Recv(&result.col, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // //     MPI_Recv(&result.row, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // //     MPI_Recv(&result.value, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // //     results[resultIndex] = result;
        // //     resultIndex++;
        // //     MPI_Send(&next_i, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
        // //     next_i++;
        // // }
        // // results = getMatchingResult(&m);

        // // clock_t end = clock();
        // // double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        // // printf("getMatching function took %lf seconds to execute.\n", time_taken);
        // for (int i = 0; i < m.resultsCount; i++)
        // {
        //     // printResult(results[i]);
        // }
        // // printResultsToFile(results, m.resultsCount, m.num_pictures);
    }
    else
    {
        int next_i;

        Result *currentResults;
        // Slave process
        void *buf = recivePackedManager();
        // MPI_Recv(&next_i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // printf("rank is %d, next_i is %d\n", rank, next_i);
        UnpackManager(&m, buf);
        printManagerInfo(m);
        // // Receive tasks from master and compute results
        // while (1)
        // {
        //     printManagerInfo(m);
        //     MPI_Recv(&next_i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        //     if (next_i >= m.num_pictures)
        //     {
        //         break;
        //     }
        //     printf("--------------current picture i------------------is %d\n\n\n", next_i);
        //     currentResults = getMatchingResultSlaves(&m, next_i);
        //     for (int i = 0; i < m.resultsCount; i++)
        //     {
        //         MPI_Send(&currentResults[i].picID, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        //         MPI_Send(&currentResults[i].patID, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        //         MPI_Send(&currentResults[i].col, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        //         MPI_Send(&currentResults[i].row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        //         MPI_Send(&currentResults[i].value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        //     }
        // }
    }
    MPI_Finalize();
    return 0;
}