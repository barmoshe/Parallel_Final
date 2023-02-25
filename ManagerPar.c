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
//funtion that gets  picture array(element) the size of it and rank number (0 or 1) if the rank is 1 return new array with odd indexs and if the rank is 0 return new array with even indexs
struct Element *getPictureArray(struct Element *pictures,int size,int rank)
{
    struct Element *newArray = (struct Element *)malloc(size * sizeof(struct Element));
    int j = 0;
    for (int i = rank; i < size; i = i + 2)
    {
        newArray[j] = pictures[i];
        j++;
    }
    return newArray;
}
//funtion to  get size of picture array
int getSizeOfPictureArray(int size,int rank)
{
    int newSize = 0;
    for (int i = rank; i < size; i = i + 2)
    {
        newSize++;
    }
    return newSize;
}

int getMatchingPartial(struct Manager m,int rank)
{
    struct Element * picturesPart=getPictureArray(m.pictures,m.num_pictures,rank);  
    int sizeOfPicturesPart=getSizeOfPictureArray(m.num_pictures,rank);
    for (int pic = 0; pic <= sizeOfPicturesPart; pic++)
    {
        for (int pat = 0; pat < m.num_patterns; pat++)
        {
            for (int i = 0; i <picturesPart[pic].n; i++)
            {
                for (int j = 0; j < picturesPart[pic].n; j++)
                {
                    double matching = getMatchingInPlace(i, j, picturesPart[pic], m.patterns[pat]);
                    if (matching <= m.matching_value)
                        printf("%d) Picture %d: found Object %d in [%d][%d]\twith matching: %lf\n",rank, picturesPart[pic].id, (pat + 1), i, j, matching);
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
int main(int argc, char** argv) {
    int num_procs, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    clock_t start = clock();

    if (num_procs != 2) {
        fprintf(stderr, "This example code is meant to run with 2 processes only\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }
   
    if (rank == 0) { // master process
        // initialize the Manager struct
        struct Manager m;
        FILE *file = fopen("input.txt", "r");
        m = readManager(file);

        // pack the Manager struct into a buffer
        MPI_Aint matching_value_size, num_pictures_size, num_patterns_size;
        MPI_Type_get_extent(MPI_DOUBLE, &matching_value_size, &matching_value_size);
        MPI_Type_get_extent(MPI_INT, &num_pictures_size, &num_pictures_size);
        MPI_Type_get_extent(MPI_INT, &num_patterns_size, &num_patterns_size);

        MPI_Aint pictures_size = 0, patterns_size = 0;
        for (int i = 0; i < m.num_pictures; i++) {
            pictures_size += sizeof(int) + sizeof(int) + m.pictures[i].n * m.pictures[i].n * sizeof(int);
        }
                for (int i = 0; i < m.num_patterns; i++) {
            patterns_size += sizeof(int) + sizeof(int) + m.patterns[i].n * m.patterns[i].n * sizeof(int);
        }

        int total_size = matching_value_size + num_pictures_size + pictures_size + num_patterns_size + patterns_size;
        void *buf = malloc(total_size);
        int pos = 0;

        MPI_Pack(&m.matching_value, 1, MPI_DOUBLE, buf, total_size, &pos, MPI_COMM_WORLD);
        MPI_Pack(&m.num_pictures, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);

        for (int i = 0; i < m.num_pictures; i++) {
            MPI_Pack(&m.pictures[i].id, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            MPI_Pack(&m.pictures[i].n, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            for (int j = 0; j < m.pictures[i].n; j++) {
                MPI_Pack(m.pictures[i].matrix[j], m.pictures[i].n, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            }
        }

        MPI_Pack(&m.num_patterns, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);

        for (int i = 0; i < m.num_patterns; i++) {
            MPI_Pack(&m.patterns[i].id, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            MPI_Pack(&m.patterns[i].n, 1, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            for (int j = 0; j < m.patterns[i].n; j++) {
                MPI_Pack(m.patterns[i].matrix[j], m.patterns[i].n, MPI_INT, buf, total_size, &pos, MPI_COMM_WORLD);
            }
        }

        // send the packed data to the slave process
        MPI_Send(buf, total_size, MPI_PACKED, 1, 0, MPI_COMM_WORLD);
        getMatchingPartial(m,rank);

    }
    else if (rank == 1) { // slave process
        // receive the packed data from the master process
        MPI_Status status;
        int count;
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_PACKED, &count);
        void *buf = malloc(count);
        MPI_Recv(buf, count, MPI_PACKED, 0, 0, MPI_COMM_WORLD, &status);

        // unpack the Manager struct from the buffer
        int pos = 0;
        struct Manager m;
        MPI_Unpack(buf, count, &pos, &m.matching_value, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(buf, count, &pos, &m.num_pictures, 1, MPI_INT, MPI_COMM_WORLD);

        m.pictures = malloc(m.num_pictures * sizeof(struct Element));
        for (int i = 0; i < m.num_pictures; i++) {
            MPI_Unpack(buf, count, &pos, &m.pictures[i].id, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buf, count, &pos, &m.pictures[i].n, 1, MPI_INT, MPI_COMM_WORLD);
            m.pictures[i].matrix = malloc(m.pictures[i].n * sizeof(int *));
            for (int j = 0; j < m.pictures[i].n; j++) {
                m.pictures[i].matrix[j] = malloc(m.pictures[i].n * sizeof(int));
                MPI_Unpack(buf, count, &pos, m.pictures[i].matrix[j], m.pictures[i].n, MPI_INT, MPI_COMM_WORLD);
            }
        }

        MPI_Unpack(buf, count, &pos, &m.num_patterns, 1, MPI_INT, MPI_COMM_WORLD);

        m.patterns = malloc(m.num_patterns * sizeof(struct Element));
        for (int i = 0; i < m.num_patterns; i++) {
            MPI_Unpack(buf, count, &pos, &m.patterns[i].id, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buf, count, &pos, &m.patterns[i].n, 1, MPI_INT, MPI_COMM_WORLD);
            m.patterns[i].matrix = malloc(m.patterns[i].n * sizeof(int *));
            for (int j = 0; j < m.patterns[i].n; j++) {
                m.patterns[i].matrix[j] = malloc(m.patterns[i].n * sizeof(int));
                MPI_Unpack(buf, count, &pos, m.patterns[i].matrix[j], m.patterns[i].n, MPI_INT, MPI_COMM_WORLD);
            }
        }
        getMatchingPartial(m,rank);
        // free the buffer
       
        free(buf);
       
    }
   MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
         clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("getMatching function took %lf seconds to execute.\n", time_taken);
    }
    MPI_Finalize();
    return 0;
}

