#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
 
void print_message_func(void *ptr);
 
int main()
{
    int tmp1,tmp2;
    void *retival;
    pthread_t thread1,thread2;
    char *message1 = "thread1";
    char *message2 = "thread2";
    
    int ret_thread1,ret_thread2;
    ret_thread1 = pthread_create(&thread1,NULL,(void *)&print_message_func,(void *)message1);
    
    if(ret_thread1 == 0)
        printf("create thread 1 true\n");
    else
        printf("create thread 1 false\n");
    
    tmp1 = pthread_join(thread1,&retival);
    printf("thread 1 return value (tmp1) is %d\n",tmp1);
    
    if(tmp1 != 0)
        printf("cannot join with thread 1\n");  
    
    ret_thread2 = pthread_create(&thread2,NULL,(void *)&print_message_func,(void *)message2);
        
    if(ret_thread2 == 0)
        printf("create thread 2 true\n");
    else
        printf("create thread 2 false\n");
    
    tmp2 = pthread_join(thread2,&retival);
    printf("thread 2 return value (tmp2) is %d\n",tmp2);
    
    if(tmp2 != 0)
        printf("cannot join with thread 2\n");    
}
 
void print_message_func(void *ptr)
{
    for(int i=0;i<5;++i)
    {
        printf("%s:%d\n",(char*)ptr,i);
    }   
}


