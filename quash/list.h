#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static int job_id_count = 0;

struct test_struct
{
    int job_id;
    int job_pid;
    char job_name[256];

    struct test_struct *next;
};

//struct test_struct * ptr;

struct test_struct *head = NULL;
struct test_struct *curr = NULL;

struct test_struct* create_list(int job_pid, char * job_name)
{
    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->job_id = job_id_count++;
    ptr->job_pid = job_pid;
    strcpy(ptr->job_name,job_name);
    ptr->next = NULL;

    head = curr = ptr;
    return ptr;
}

struct test_struct* add_to_list(int job_pid, char * job_name)
{
    if(NULL == head)
    {
        return (create_list(job_pid,job_name));
    }

    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->job_id = job_id_count++;
    ptr->job_pid = job_pid;
    strcpy(ptr->job_name,job_name);
    ptr->next = NULL;

    curr->next = ptr;
    curr = ptr;

    return ptr;
}

struct test_struct* search_in_list(int job_pid, struct test_struct **prev)
{
    struct test_struct *ptr = head;
    struct test_struct *tmp = NULL;
    bool found = false;

    //printf("\n Searching the list for value [%d] \n",job_pid);

    while(ptr != NULL)
    {
        if(ptr->job_pid == job_pid)
        {
            found = true;
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return ptr;
    }
    else
    {
        return NULL;
    }
}

int search_by_job_id(int job_id)
{
    struct test_struct *ptr = head;
    bool found = false;

    //printf("\n Searching the list for value [%d] \n",job_pid);

    while(ptr != NULL)
    {
        if(ptr->job_id == job_id)
        {
            found = true;
            break;
        }
        else
        {
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        return ptr->job_pid;
    }
    else
    {
        return 0;
    }
}

int delete_from_list(int job_pid)
{
    struct test_struct *prev = NULL;
    struct test_struct *del = NULL;


    del = search_in_list(job_pid,&prev);
    if(del == NULL)
    {
        return -1;
    }
    else
    {
        printf("[%d] %d %s Finished!\n",del->job_id,del->job_pid,del->job_name);
        if(prev != NULL)
            prev->next = del->next;

        if(del == head)
        {
            head = del->next;
        }
        else if(del == curr)
        {
            curr = prev;
        }
    }

    free(del);
    del = NULL;

    return 0;
}

void print_list(void)
{
    struct test_struct *ptr = head;
    
    while(ptr != NULL)
    {
        printf("[%d] %d %s\n",ptr->job_id,ptr->job_pid,ptr->job_name);
        ptr = ptr->next;
    }
    return;
}