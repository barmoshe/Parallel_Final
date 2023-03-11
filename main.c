#include "utill.h"

// Function to print output for all pictures in the results array to a file
void print_results(Result *results, int resultsCount, int num_pictures)
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

int main()
{
    FILE *file = fopen("input.txt", "r");
    if (file == NULL)
    {
        printf("Error opening file \n");
    }
    struct Manager m = {0};
    m = readManager(file);
    fclose(file);
    printManagerInfo(m);
    clock_t start = clock();
    Result *results = (Result *)calloc(3 * m.num_pictures, sizeof(Result));
    results = getMatchingResult(&m);
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("getMatching function took %lf seconds to execute.\n", time_taken);
    for (int i = 0; i < m.resultsCount; i++)
    {
        printResult(results[i]);
    }
    print_results(results, m.resultsCount, m.num_pictures);
    return 0;
}