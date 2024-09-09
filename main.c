
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define N 5
#define FIN_PROB 0.1
#define MIN_INTER_ARRIVAL_IN_NS 8000000
#define MAX_INTER_ARRIVAL_IN_NS 9000000
#define INTER_MOVES_IN_NS 100000
#define SIM_TIME 2
#define NUM_OF_CARS (int)(((SIM_TIME*1000000000)/(MIN_INTER_ARRIVAL_IN_NS))+50)  // 1.0e9 = 1000000000 for nano sec



// function declartion

void* GenerateCars(void *arg);
void* DrivingCars(void*arg);
void* PrintCircle(void *arg);

//Mutex
pthread_mutexattr_t attr;
pthread_mutex_t car_cell[(4*N)-4]={0};

//Thread
pthread_t print;
pthread_t cars[4][(int)NUM_OF_CARS]={0};
pthread_t generatorCars[4];




struct timespec initial_time;


//global variebles
int generatorFlag[4]={0,0,0,0};
int generatorNum[4]={0};
char printCircle[(4*N)-4]={" "};


int main()
{

    srand(time(NULL));
    int i;

    // Init Mutex for the program
    if(pthread_mutexattr_init(&attr)!=0)
    {
        perror("pthread_mutexattr_init error!!");
        exit(1);
    }

    // Init Mutex for program and for car cells !!
    for (i=0;i<(4*N)-4;i++)
    {
        if(pthread_mutex_init(&car_cell[i],&attr)!=0)
        {
            perror("pthread_mutex_init error in cell !!");
            exit(1);
        }
    }

    // Initial Clock for the program !!
    clock_gettime(CLOCK_REALTIME,&initial_time);

    // Init Print Thread !!
    if (pthread_create(&print,NULL,PrintCircle,NULL)!=0)
    {
        perror("error in creating pthread for print !!");
        exit(1);
    }

    // Init Thread for each Generator !!

    for (i=0;i<4;i++)
    {
        generatorNum[i]=i;
        if (pthread_create(&generatorCars[i],NULL,GenerateCars,(void *)&generatorNum[i])!=0)
        {
            perror("error in creating pthread for genarate !!");
            exit(1);
        }
    }

    //Wait for print function to finish execution

    pthread_join(print,NULL);

    for (i=0;i<4;i++)
    {
        pthread_join(generatorCars[i],NULL);
    }

    //Free Mutex Resources

    for (i=0;i<(4*N)-4;i++)
    {
        if(pthread_mutex_destroy(&car_cell[i])!=0)
        {
            perror("pthread_mutex_destroy error in cell !!");
            exit(1);
        }
    }

    return 0;
}



void* GenerateCars(void *arg)
{
    /*
     This function Generate cars into the circle
     */
    int i=0;
    int flag=0,carFlag=0,numberOfTheCar=0;
    int indexGenNum = (int)(*(int *)arg);
    struct timespec current_time;
    struct timespec car_time;
    double SimInterval = SIM_TIME * 1.0e9 ;  // we are using the * 1.0e9 because we work with nano second here !!
    double CarInterval;
    double timeSampling = 0 ;

    while (!flag)
    {
        clock_gettime(CLOCK_REALTIME,&current_time);
        timeSampling = (double)((current_time.tv_sec - initial_time.tv_sec)) * 1.0e9 +
                (double) (current_time.tv_nsec - initial_time.tv_nsec) ;

        if (timeSampling < SimInterval )     //If the simulation not over yet
            {
            CarInterval = (rand() % (MAX_INTER_ARRIVAL_IN_NS + 1 -MIN_INTER_ARRIVAL_IN_NS )) +
                    MIN_INTER_ARRIVAL_IN_NS ;//number = (rand() % (upper - lower + 1)) + lower
                    clock_gettime(CLOCK_REALTIME,&car_time);

                    // Generate Car Interval Sample - wait until we have reach the time interval to generate new car
                    while(!carFlag)
                    {
                        clock_gettime(CLOCK_REALTIME,&current_time);
                        timeSampling = (double)((current_time.tv_sec - car_time.tv_sec)) * 1.0e9 +
                                (double) (current_time.tv_nsec - car_time.tv_nsec) ;

                        if (timeSampling >= CarInterval) // we reach the time interval
                            {
                            carFlag=1;
                            }
                    }
                    //Creating new car
                    if (pthread_create(&cars[indexGenNum][numberOfTheCar],NULL,DrivingCars,((void *)&indexGenNum))!=0)
                    {
                        perror("error in creating pthread for car !!");
                        exit(1);
                    }
                    numberOfTheCar++;
                    carFlag=0;
            }
        else
        {
            flag = 1 ;
        }
    }
    // Simulation over in this Generator
    generatorFlag[indexGenNum]=1;
    for (i=0;i<numberOfTheCar;i++)
    {
        pthread_join(cars[indexGenNum][i],NULL);
    }

    return NULL;
}








void* DrivingCars(void*arg)
{
    /*
     This function Moving the cars from the genartor to the circle
     and in the circle itself
     */
    int CircleFlag = 0 ;
    struct timespec current_time;
    struct timespec initialCarTime;
    double timeSampling ;
    int CarMove = 0 ;
    int indexGenNum = (int)(*(int *)arg);
    int StartGenCell = indexGenNum * (N-1) ; // get the Gen index in the circle
    int CurrntCellIndex = StartGenCell ;
    int MutexCase = 0;

    //While loop -  Check if The Car can enter the circle !!

    // If the nearest to generator cell  is empty and the  generator Sim time not over yet
    while (!CircleFlag && !(generatorFlag[indexGenNum]) )
    {
        // only one car can get into circle each time then we lock the start cell for each genrator (green indexes)
        if (pthread_mutex_lock(&car_cell[(StartGenCell - 1 + 4*N-4) % (4*N-4) ])!=0)
        {
            perror("pthread_mutex_ error for locking the cell !!");
            exit(1);
        }
        MutexCase = pthread_mutex_trylock(&car_cell[StartGenCell]);

        switch (MutexCase)
        {
            //the car can move into the circle and we can unlock the start cell for the next car
            case 0:
            {
                CircleFlag = 1 ;
                printCircle[StartGenCell] = '*';
                clock_gettime(CLOCK_REALTIME,&initialCarTime);
                if (pthread_mutex_unlock(&car_cell[(StartGenCell- 1 + 4*N-4) % (4*N-4) ])!=0)
                {
                    perror("pthread_mutex_ error for Unlocking the cell !!");
                    exit(1);
                }
                break;
            }
            //The cell is busy with a car then we free our cell
            case EBUSY :
            {

                if (pthread_mutex_unlock(&car_cell[(StartGenCell- 1 + 4*N-4) % (4*N-4) ])!=0)
                {
                    perror("pthread_mutex_ error for Unlocking the cell !!");
                    exit(1);
                }
                break;
            }
            default:
            {
                perror("MutexCase error for trylock the cell !!");
                exit(1);
            }
        }

    }

    // Moving cars in the circle !!!

    // Check if simulation not over yet
    while (!(generatorFlag[indexGenNum]))
    {
        // While we can move the car to next cell
        while (!CarMove)
        {
            clock_gettime(CLOCK_REALTIME,&current_time);
            timeSampling = (double)((current_time.tv_sec - initialCarTime.tv_sec)) * 1.0e9 +
                    (double) (current_time.tv_nsec - initialCarTime.tv_nsec) ;
            if (timeSampling >=INTER_MOVES_IN_NS)
            {
                CarMove=1;
            }
        }
        clock_gettime(CLOCK_REALTIME,&initialCarTime);
        CarMove = 0 ; //for the next move to this car
        CurrntCellIndex +=1; // Mark the next index that this car move to

        if (pthread_mutex_lock(&car_cell[(CurrntCellIndex) % ((4*N-4)) ])!=0)
        {
            perror("pthread_mutex_ error for locking the cell !!");
            exit(1);
        }
        printCircle[(CurrntCellIndex-1) % ((4*N-4)) ] = ' ';
        printCircle[CurrntCellIndex % ((4*N-4)) ] = '*';

        if (pthread_mutex_unlock(&car_cell[(CurrntCellIndex-1) % ((4*N-4)) ])!=0)
        {
            perror("pthread_mutex_ error for ulocking the cell !!");
            exit(1);
        }

        // We make a car disappears from the circle with some propability
        if (CurrntCellIndex % (4) == 0)
        {
            if ((rand() % 100 ) <= 100 * FIN_PROB) // The car will disappear from
                {
                printCircle[(CurrntCellIndex) % (4*N-4) ] = ' ';
                if (pthread_mutex_unlock(&car_cell[(CurrntCellIndex) % (4*N-4) ])!=0)
                {
                    perror("pthread_mutex_ error for ulocking the cell !!");
                    exit(1);
                }
                return NULL; // This Thread car disappears
                }
        }

    }

    // If car get into the circle and  the simultaion over , we need to unlock the cell !!
    if (CircleFlag)
    {
        if (pthread_mutex_unlock(&car_cell[(CurrntCellIndex) % (4*N-4) ])!=0)
        {
            perror("pthread_mutex_ error for ulocking the cell !!");
            exit(1);
        }
    }
    return  NULL ;
}





void* PrintCircle(void *arg)
{
    /*
     This function print the circle at each time
     */
    int i=0;
    struct timespec current_time;
    struct timespec start_time;
    double PrintInterval = SIM_TIME * 1.0e9/12 ;
    double timeSampling = 0  ;
    int Simflag =0,numberOfSnap=0;
    char CircleLine [N-1] ;
    //here we make the middle of the traffic
    for (i=0;i<N-2;i++)
    {
        CircleLine[i]='@';

    }
    CircleLine[N-2] = '\0';
    clock_gettime(CLOCK_REALTIME,&start_time);
    while(!Simflag)
    {
        clock_gettime(CLOCK_REALTIME,&current_time);
        timeSampling = (double)((current_time.tv_sec - start_time.tv_sec)) * 1.0e9 +
                (double) (current_time.tv_nsec - start_time.tv_nsec) ;
        //reach the time interval and we take the snapshot
        if (timeSampling >=PrintInterval)
        {
            clock_gettime(CLOCK_REALTIME,&start_time); // for the next sample
            printf("\n \n");
            /*
             * In this loops we print all the rows
             */
            for (i=N-1;i>=0;i--)
            {
                printf("%c",printCircle[i]);

            }
            printf("\n");
            for (i=0;i<N-2;i++)
            {
                printf("%c",printCircle[i+N]);
                printf("%s",CircleLine);
                printf("%c\n",printCircle[4*N -5 -i]);
            }
            for (i= 2*N -2 ;i < 3*N -2 ;i++)
            {
                printf("%c",printCircle[i]);
            }
            printf("\n \n");
            numberOfSnap++;
            if (numberOfSnap == 10) // we finish to take all the snapshots
                {
                Simflag=1;
                }
        }
    }
    return NULL;

}


