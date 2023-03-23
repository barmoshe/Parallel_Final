#include "Manager.h"

double getDiff(int p, int o)
{
    return abs((p - o) / p);
}

void printResult(struct Result result)
{
    printf("Picture %d: found Object %d in [%d][%d]\twith matching: %lf \t \t\n",result.picID, result.patID, result.col, result.row,result.value);        
}

//function to print array of struct Result
void printResultArray(struct Result* result, int size)
{
    for (int i = 0; i < size; i++)
    {
        printResult(result[i]);
    }
              printf("\n");

  
}

double getMatchingInPlace(int row, int col, struct Element picture, struct Element pat)
{
    int startCol = col;
    int startRow = row;
    if (col+pat.n>=picture.n)
        return 1.0;
    if(row+pat.n>=picture.n)
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
    return matching/(pat.n*pat.n);
}
int getMatching(struct Manager m)
{
    for (int pic = 0; pic < m.num_pictures; pic++)
    {
        int count=0;
        for (int pat = 0;pat< m.num_patterns; pat++)
        {
            for (int i = 0; i < m.pictures[pic].n; i++)
            {
                for (int j = 0; j < m.pictures[pic].n; j++)
                {
                    if(count<=2){
                    double matching =getMatchingInPlace(i, j, m.pictures[pic], m.patterns[pat]) ;
                    if (matching<= m.matchingValueFromFile){
                       printf("Picture %d: found Object %d in [%d][%d]\twith matching: %lf\n",(pic + 1), (pat + 1), i, j,matching);
              count++;
                }
                }}
            }
        }
    }
    return 0;
}
 struct Result* getMatchingResult(struct Manager m)
{
    struct Result* result = (struct Result*)malloc(sizeof(struct Result));
    int count = 0;
    for (int pic = 0; pic < m.num_pictures; pic++)
    {
        for (int pat = 0;pat< m.num_patterns; pat++)
        {
            for (int i = 0; i < m.pictures[pic].n; i++)
            {
                for (int j = 0; j < m.pictures[pic].n; j++)
                {
                    if(count<=2){
                    double matching =getMatchingInPlace(i, j, m.pictures[pic], m.patterns[pat]) ;
                    if (matching<= m.matchingValueFromFile)
                    {
                        result[count].picID = m.pictures[pic].id;
                        result[count].patID = m.patterns[pat].id;
                        result[count].col = i;
                        result[count].row = j;
                        result[count].value = matching;
                        count++;
                    }}
                }
            }
        }
    }
    return result;
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

void printElement(struct Element element)
{
    printf("ID: %d\n", element.id);
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
    printf("Matching Value: %lf\n", manager.matchingValueFromFile);
    printf("%d Pictures:\n", manager.num_pictures);
    for (int i = 0; i < manager.num_pictures; i++)
    {
        printf("Picture %d:\n", i + 1);
        printElement(manager.pictures[i]);
    }
    printf("%d Patterns:\n", manager.num_patterns);
    for (int i = 0; i < manager.num_patterns; i++)
    {
        printf("Pattern %d:\n", i + 1);
        printElement(manager.patterns[i]);
    }
}

int main()
{
    FILE *file = fopen("input.txt", "r");
    struct Manager manager = readManager(file);
    fclose(file);
    printManagerInfo(manager);
    int size =manager.num_pictures*3;
    struct Result * results = (struct Result *)malloc(sizeof(struct Result)*size);
    clock_t start = clock();
    getMatching(manager);
    clock_t end = clock();
    double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("getMatching function took %lf seconds to execute.\n", time_taken);
    
    return 0;
}