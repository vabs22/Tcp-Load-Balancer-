#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

pthread_t tid;
int counter = 0;
pthread_mutex_t lock;

void* doSomeThing(void *arg)
{
    printf("thread created\n");
    //pthread_mutex_lock(&lock);

    unsigned long i = 0;
    //int *counter = ;
    int num = *((int *)arg);
    free(arg);
    printf("\n Job %d started\n", num);

    sleep(num*2);

    printf("\n Job %d finished\n", num);

    //pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void)
{
    int i = 0;
    int err;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    //pthread_mutex_lock(&lock);

    for(i=0;i<5;i++)
    {
        int * tmp = (int *) malloc(sizeof(int));
        *tmp = i;
        if( pthread_create(&tid, NULL, &doSomeThing, (void *)tmp) < 0)
            printf("\ncan't create thread :[%d]", i);
    }

    
    pthread_join(tid , NULL);
    printf("Going to sleep\n");
    //sleep(20);
    //pthread_mutex_unlock(&lock);



    //pthread_join(tid, NULL);
    //pthread_mutex_destroy(&lock);

    return 0;
}