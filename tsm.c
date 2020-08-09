#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

//global variables
int **arr;
int num, num_thrds, num_pths;
int *shrtst_path;
int shrtst_dist, shrtst_thrd;

int *cities;
int pts_per_thrd, tc_id;

void one_by_one()
{
    //omp_set_num_threads(num_thrds);
    #pragma omp parallel num_threads(num-1) private(cities, tc_id)
    {
        cities = (int *) malloc ((num) *sizeof(int));
        int i;

        //computing cities to permute in each thread
        int index = 0;
        tc_id = omp_get_thread_num()+1;
        for(i=1; i<num; i++)
        {
            if(i != tc_id)
            {
                cities[index++]=i;
                
            }
            printf("%d\n", i);
        }

        printf("nznznz\n");

        //computing base distance from 0 to tc_id+1(thread id)
        int base_dist = arr[0][tc_id];
        int my_dist;
        /*Loop gets next permutation and if it is smaller than
        current distance:
        if shrtst_dist=-1; uses any of them
        if my_dist becomes longer than shrtst_dist at any time
            drop it and continue*/
        for(i=0; i<pts_per_thrd; i++)
        {
            my_dist = base_dist;
            my_dist+=arr[tc_id][cities[0]];
            for(i=1; i<num-2; i++)
            {
                my_dist+=arr[cities[i-1]][cities[i]];
                if(my_dist >= shrtst_dist)
                    break;
            }
            if(my_dist < shrtst_dist)
            {
                #pragma omp critical
                {
                    if (my_dist < shrtst_dist)//test twice to avoid artificial race condition
                    {
                        shrtst_dist = my_dist;
                        shrtst_path[0] = 0;
                        shrtst_path[1] = tc_id;
                        for(i=0; i<num-2; i++)
                            shrtst_path[i+2] = cities[i];
                        shrtst_thrd = omp_get_thread_num();
                    }
                }
            }

            int m,n,temp;
            m = num-2;
            while(m>0 && cities[m-1] >= cities[m])
                m--;

            if(m==0)//last permutation
                break;

            n = num-3;
            while(cities[n] <= cities[m-1])
                n--;

            temp = cities[m-1];
            cities[m-1] = cities[n];
            cities[n] = temp;

            n = num-3;
            while(m<n)
            {
                temp = cities[m-1];
                cities[m] = cities[n];
                cities[n] = temp;
                m++;
                n--;
            }
        }
        #pragma omp barrier
    }

    printf("Best path:");
    int j;
    for(j=0; j<num; j++)
        printf("%d", shrtst_path[j]);
    printf("\nDistance: %d\n", shrtst_dist);
}

void get_input(char filename[])
{
    FILE *fptr;
    int i,j;

    fptr = fopen(filename, "r");

    if(!fptr)
    {
        printf("Cannot open file %s\n", filename);
        exit(1);   
    }

    //allocate matricies and vectors
    arr = (int**)malloc(num * sizeof(int*));
    if(!arr)
    {
        printf("Cannot allocate array!\n");
        exit(1);
    }


    for(i=0; i<num; i++)
    {
        arr[i] = (int*)malloc(num * sizeof(int));
        if(!arr[i])
        {
            printf("Cannot allocate arr[%d]\n", i);
            exit(1);
        }
    }
    for(i=0; i<num; i++)
    {
        for(j=0; j<num; j++)
        {
            fscanf(fptr, "%d", &arr[i][j]);
        }
    }

    //free(arr);
    //free(cities);

    fclose(fptr);
}



void main(int argc, char *argv[])
{
    num=atoi(argv[1]);
    num_thrds=strtol(argv[2], 0, 10);

    get_input(argv[3]);

    int m = num-1;
    num_pths = 1;
    //(num-1)!
    while(m > 0)
    {
        num_pths *=m;
        m--;
    }
    pts_per_thrd = num_pths/(num-1);

    shrtst_path = (int *)malloc(num * sizeof(int));
    shrtst_dist = 999999; //unset

    one_by_one();

}

