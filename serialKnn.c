#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>


//load Data drom a txt file
void loadData(double **Data,int NumberOfPoints,int Dimensions)
{
    
    int i ,j,n;   
    FILE *file = fopen("train_X.txt", "r"); //open for read txt file

    for (i=0;i<NumberOfPoints;i++)
    {
        for (j=0;j<Dimensions;j++)
        {
            
           n = fscanf(file,"%lf",&Data[i][j]);
        }
    }
    
    fclose(file);// close file
}


//test function
void test(int NumberOfPoints,int **index)
{
    int i,j,n,temp;
    int **testIndex;
    
    testIndex = malloc(sizeof(int*)*NumberOfPoints);
    
    for (i=0;i<NumberOfPoints;i++)
    {
        testIndex[i] = malloc(sizeof(int)*4 );
    }

    FILE *file = fopen("test.txt", "r"); //open for read txt
    //read the data from kknsearch function of matlab
    for (i=0;i<NumberOfPoints;i++)
    {
        for (j=0;j<4;j++)
        {
            n = fscanf(file,"%d",&temp);
            testIndex[i][j] = temp - 1;
        }
    }
    
    //compare the arrays
    for (i=0;i<NumberOfPoints;i++)
    {
        for (j=0;j<4;j++)
        {
            if ( index[i][j] == testIndex[i][j] )
            {
                continue;
            }
            else
            {
                printf("Test Failed!!! \n");   
                return;         
            }
        }
    }
    
    printf("Test Passed!!!! \n");
    
    fclose(file);
}


//compute Euclidean Distance
double euclideanDistance(int i, int j,double **Data1,double **Data2,int Dimensions)
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
void kNearestNeighbours(int k,double **distances,int **index,double **Data1,double **Data2,int NumberOfPoints,int Dimensions)
{
    double dis;
    int i,j,m,p;
    
    for (i=0;i<NumberOfPoints;i++)
    {
        for (j=0;j<NumberOfPoints;j++)
        {
                dis = euclideanDistance(i,j,Data1,Data2, Dimensions);//compute Euclidean Distance

                for (m =0 ;m<(k+1);m++ )
                {
                    if (dis<distances[i][m])
                    {
                        for (p = k;p>m;p--)
                        {
                            distances[i][p] = distances[i][p-1];
                            index[i][p] = index[i][p-1];
                        }
                        distances[i][m] = dis;
                        index[i][m] = j;
                        
                        break;
                    }
                }
        }
    }
}

int main(int argc , char **argv)
{
    
    int NumberOfPoints = 60000;
    int Dimensions = 30;
    
    int k; // k nearest neighbours
    
    if (argc != 2) 
    {
    printf("Usage: %s k \n  where k is number of nearest neighbours \n", argv[0]);
    exit(1);
    }
    
    k = atoi(argv[1]);
    
    struct timeval startwtime, endwtime;
    double time;

    int i,j,l,m;
    
    double **Data;
    Data = malloc(sizeof(double*)*NumberOfPoints);
   
    for (i=0;i<NumberOfPoints;i++)
    {
        Data[i] = malloc(sizeof(double)*Dimensions);
    }
    
    int **index;
    double **distances;
    index = malloc(sizeof(int*)*NumberOfPoints);
    distances = malloc(sizeof(double*)*NumberOfPoints);
    
    for (i=0;i<NumberOfPoints;i++)
    {
        index[i] = malloc(sizeof(int)*(k+1));
    }
    
    for (i=0;i<NumberOfPoints;i++)
    {
        distances[i] = malloc(sizeof(double)*(k+1));
    }

    for (i=0;i<NumberOfPoints;i++)
        for (j=0;j<(k+1);j++)
            distances[i][j] = 100;
    
    loadData(Data,NumberOfPoints,Dimensions);
    
    gettimeofday (&startwtime, NULL);
    kNearestNeighbours(k,distances,index,Data,Data,NumberOfPoints,Dimensions);
    gettimeofday (&endwtime, NULL);
   
    time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);
              
    printf("serial Knn time: %f \n", time);
    
    
    //test(NumberOfPoints,index);
    
    
    return 0;

}
