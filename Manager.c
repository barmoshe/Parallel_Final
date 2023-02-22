#include "Manager.h"

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
int getMatching(struct Manager m)
{
    for (int pic = 0; pic < m.num_pictures; pic++)
    {
        for (int pat = 0; pat < m.num_patterns; pat++)
        {
            for (int i = 0; i < m.pictures[pic].n; i++)
            {
                for (int j = 0; j < m.pictures[pic].n; j++)
                {
                    double matching = getMatchingInPlace(i, j, m.pictures[pic], m.patterns[pat]);
                    if (matching <= m.matching_value)
                        printf("Picture %d: found Object %d in [%d][%d]\twith matching: %lf\n", (pic + 1), (pat + 1), i, j, matching*1000);
                }
            }
        }
    }
    return 0;
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

struct Manager readManager(FILE *file)
{
    struct Manager manager;

    fscanf(file, "%lf", &manager.matching_value);
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

void printElement(struct Element element)
{
    printf("Size: %d\n", element.n);
    // printf("Matrix:\n");
    // for (int i = 0; i < element.n; i++)
    // {
    //     for (int j = 0; j < element.n; j++)
    //     {
    //         printf("%d ", element.matrix[getI(i,j,element.n)]);
    //     }
    //     printf("\n");
    // }
}

void printManagerInfo(struct Manager manager)
{
    printf("[Matching Value: %lf]\n", manager.matching_value);
    printf("*****%d Pictures:\n", manager.num_pictures);
    for (int i = 0; i < manager.num_pictures; i++)
    {
        printf("-----[Picture %d]-----\n", manager.pictures[i].id);
        printElement(manager.pictures[i]);
    }
    printf("*****%d Patterns:\n", manager.num_patterns);
    for (int i = 0; i < manager.num_patterns; i++)
    {
        printf("-----[Pattern %d]-----\n", manager.patterns[i].id);
        printElement(manager.patterns[i]);
    }
}


int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);               // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the current process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes in the communicator
    struct Manager *manager = (struct Manager *)malloc(sizeof(struct Manager));
    if (rank == 0)
    {
        FILE *file = fopen("input.txt", "r");
        *manager = readManager(file);
        printManagerInfo(*manager);

        fclose(file);
    }
    clock_t start = clock();
   
        if (rank == 0)
            getMatching(*manager);

  
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("getMatching function took %lf seconds to execute.\n", time_taken);
    return 0;
}