#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <sys/time.h>


//test function
void test(int id,int valuesPerProcess,int NumberOfPoints,int **index)
{
    int i,j,n;
    int **testIndex;
    
    testIndex = malloc(sizeof(int*)*valuesPerProcess);
    
    for (i=0;i<valuesPerProcess;i++)
    {
        testIndex[i] = malloc(sizeof(int)*4 );
    }
    
    int temp;

    FILE *file = fopen("test.txt", "r"); //open for read txt
    //read the data from kknsearch function of matlab
    for (i=0;i<NumberOfPoints;i++)
    {
        for (j=0;j<4;j++)
        {
            n = fscanf(file,"%d",&temp);
            
            if (i>=(valuesPerProcess*id) && i<(valuesPerProcess*(id+1))) 
            {
                testIndex[i-valuesPerProcess*id][j] = temp -1; //matlab arrays start from 1
            }
        }
    }
   
    //compare the arrays
    for (i=0;i<valuesPerProcess;i++)
    {
        for (j=0;j<4;j++)
        {
            if ( index[i][j] == testIndex[i][j] )
            {
                continue;
            }
            else
            {
        
                printf("id:%d Test Failed!!! \n",id); 
                return;         
            }
            
        }
    }
    
    printf("id:%d Test Passed!!!! \n",id);
        
    fclose(file);
}

//make two arrays equal 
void copy(double **recData,double **newData,int valuesPerProcess,int Dimensions)
{
    int i,j;
    for (i=0;i<valuesPerProcess;i++)
    {
        for (j=0;j<Dimensions;j++)
        {
            recData[i][j] = newData[i][j];
        }
    }
}


//load Data drom a txt file
void loadData(double **Data,int id,int valuesPerProcess,int NumberOfPoints,int Dimensions)
{
    double temp;
    int i ,j,n;
    
    FILE *file = fopen("train_X.txt", "r"); //open for read txt
    
    for (i=0;i<NumberOfPoints;i++)
    {
        for (j=0;j<Dimensions;j++)
        {
            
            n = fscanf(file,"%lf",&temp);
            
            if (i>=(valuesPerProcess*id) && i<(valuesPerProcess*(id+1))) 
            {
                Data[i-valuesPerProcess*id][j] = temp;
            }
        }
    }
    
    fclose(file);
}

//compute Euclidean Distance
double euclideanDistance( int i, int j,double **Data1,double **Data2,int Dimensions)
{
    double sum = 0;
    int l;
    
    for (l=0;l<Dimensions;l++)
    {
        sum = sum + pow((Data1[i][l]-Data2[j][l]),2);
    }
    
    sum = sqrt(sum);
    return sum;
}

//k Nearest Neighbours function
void kNearestNeighbours(int k,double **distances,int **index,double **Data1,double **Data2,int valuesPerProcess,int Dimensions,int previous,int numtasks,int id,int flag1,int flag2)
{
    double dis;
    int i,j,m,p;
    int temp=0;
    
    for (i=0;i<valuesPerProcess;i++)
    {
        for (j=0;j<valuesPerProcess;j++)
        {
            
            dis = euclideanDistance(i,j,Data1,Data2,Dimensions);//compute Euclidean Distance

            for (m =0 ;m<(k+1);m++)
            {
                if (dis<distances[i][m])
                {
                    for (p = k;p>m;p--)
                    {
                        distances[i][p] = distances[i][p-1];
                        index[i][p] = index[i][p-1];
                    }
                    distances[i][m] = dis;
                    
                    if (flag1 == 0)
                    {
                        index[i][m] = j + valuesPerProcess * id ;
                    }
                    else  
                    {
                        
                        temp = previous-flag2;
                        if (temp < 0)
                        {  
                            temp = numtasks + temp;
                        }
                        
                        index[i][m] = j + (temp ) * valuesPerProcess ;
                    }
                    
                    
                    break;
                }
            }
            
        }
    }
}

int main(int argc, char **argv)
{
    
    if (argc != 2) 
    {
    printf("Usage: %s k \n  where k is number of nearest neighbours \n", argv[0]);
    exit(1);
    }
    
    MPI_Init(&argc, &argv); 
    
    int numtasks, id;//number of tasks and id number for every process
       
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    
    int NumberOfPoints = 60000;
    int Dimensions = 30;
    
    int k; // k nearest neighbours
    
    k = atoi(argv[1]);
    
    int i,j,l,next,prev;

    int valuesPerProcess = (NumberOfPoints / (numtasks));

    double time;

    double **Data,**recData,**newData;
     
    Data = malloc(sizeof(double*)*valuesPerProcess);
    recData = malloc(sizeof(double*)*valuesPerProcess);
    newData = malloc(sizeof(double*)*valuesPerProcess);
    
    
    for (i=0;i<valuesPerProcess;i++)
    {
        Data[i] = malloc(sizeof(double)*Dimensions);
    }
    
    for (i=0;i<valuesPerProcess;i++)
    {
        recData[i] = malloc(sizeof(double)*Dimensions);
    }
    for (i=0;i<valuesPerProcess;i++)
    {
        newData[i] = malloc(sizeof(double)*Dimensions);
    }
    
    
    loadData(Data,id,valuesPerProcess,NumberOfPoints,Dimensions);
    
    
    int **index;
    
    index = malloc(sizeof(int*)*valuesPerProcess);
    
    for (i=0;i<valuesPerProcess;i++)
    {
        index[i] = malloc(sizeof(int)*(k+1));
    }
    
    double **distances;
    
    distances = malloc(sizeof(double*)*valuesPerProcess);
    
    for (i=0;i<valuesPerProcess;i++)
    {
        distances[i] = malloc(sizeof(double)*(k+1));
    }
    
    for (i=0;i<valuesPerProcess;i++)
        for (j=0;j<(k+1);j++)
            distances[i][j] = 100;
    
    MPI_Status status;
    
    prev = id - 1;
    next = id + 1;
    
    if (id == 0)  prev = numtasks - 1;
    if (id == (numtasks - 1))  next = 0;
    
    int tag1=1,tag2=1;
    
    int flag1 = 0;
    int flag2 = 0;
    
    MPI_Request reqs[2];   // required variable for non-blocking calls
    MPI_Status stats[2];    // required variable for Waitall routine
    
    double start,end;
    
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    int b=0;
    
    for (i=0;i<valuesPerProcess;i++)
    {
        for (j=0;j<Dimensions;j++)
        {
            MPI_Irecv(&(recData[i][j]), 1, MPI_DOUBLE, prev,tag1 , MPI_COMM_WORLD, &reqs[0]);
            MPI_Isend(&(Data[i][j]), 1, MPI_DOUBLE, next,tag1 , MPI_COMM_WORLD, &reqs[1]);
            MPI_Waitall(2, reqs, stats);
            
        }
    }
    
    kNearestNeighbours(k,distances,index,Data,Data,valuesPerProcess,Dimensions,prev,numtasks,id,flag1,flag2);
    flag1 = 1;
    
    for (l=0;l<(numtasks-1);l++)
    {
        
        for (i=0;i<valuesPerProcess;i++)
        {
            for (j=0;j<Dimensions;j++)
            {
                MPI_Irecv(&(newData[i][j]), 1, MPI_DOUBLE, prev, tag1 , MPI_COMM_WORLD, &reqs[0]);
                MPI_Isend(&(recData[i][j]), 1, MPI_DOUBLE, next, tag1, MPI_COMM_WORLD, &reqs[1]);
                
                MPI_Waitall(2, reqs, stats);
            }
        }
        
        kNearestNeighbours(k,distances,index,Data,recData,valuesPerProcess,Dimensions,prev,numtasks,id,flag1,flag2);
        
        flag2++;
        
        copy(recData,newData,valuesPerProcess,Dimensions);
        
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    
    if (id==0)
        printf("%f \n",end-start);
    /*    
    if (id==0)
    {
        for (i=1;i<(k+1);i++)
            printf("distance: %.15f index: %d \n",distances[27][i],index[27][i]);
    }
    */
    //test(id,valuesPerProcess,NumberOfPoints,index);//test the results for every process

    MPI_Finalize();
    
    return 0;
    
}
