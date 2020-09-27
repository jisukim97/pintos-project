#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.c"

#define MAX_SIZE 10

struct list lists[MAX_SIZE];
int lists_ptr = -1; /* pointer for lists stored */


char *rtrim(char *s)
{
    char t[MAX_SIZE];
    char *end;

    strcpy(t, s);
    end = t + strlen(t) - 1;
    while (end != t && isspace(*end))
        end--;
    *(end + 1) = '\0';
    s = t;

    return s;
}

char *ltrim(char *s)
{
    char *begin;
    begin = s;

    while (*begin != '\0')
    {
        if (isspace(*begin))
            begin++;
        else
        {
            s = begin;
            break;
        }
    }
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

/* Find the exact list from the array of Lists. */
struct list *find_list(char *finding_name)
{
    finding_name = trim(finding_name);
    for (int i = 0; i <= lists_ptr; i++)
        if (!strcmp(finding_name, lists[i].name))
            return &lists[i];
}

int main()
{
    char *line = NULL;
    size_t len = 0;
    ssize_t line_size = 0;

    while (1)
    {
        /* get the commands one by one. */
        line_size = getline(&line, &len, stdin);
        if (!strcmp(line, "quit\n"))
            break;

        /* Parse the command. */
        char *command = strtok(line, " ");

        /* Create data structure. */
        if (!strcmp(command, "create"))
        {
            command = strtok(NULL, " ");
            /* Create List. */
            if (!strcmp(command, "list"))
            {
                struct list *list_ptr = &lists[++lists_ptr];
                list_ptr->head.prev = NULL;
                list_ptr->head.next = &list_ptr->tail;
                list_ptr->tail.prev = &list_ptr->head;
                list_ptr->tail.next = NULL;

                command = strtok(NULL, "\n");
                strcpy(list_ptr->name, trim(command));
            }
            /* Create Hash table. */
            else if (!strcmp(command, "hashtable"))
            {
            }
            /* Create Bitmap. */
            else if (!strcmp(command, "bitmap"))
            {
            }
        }
        /* Delete the data structure. */
        else if (!strcmp(command, "delete"))
        {
            command = strtok(NULL, " ");
            list_init(find_list(command));
        }
        /* Enumerate data in the structure. */
        else if (!strcmp(command, "dumpdata"))
        {
            command = strtok(NULL, " ");
            int flag = 0;
            struct list_elem *tmp_elem_ptr = list_begin(find_list(command));

            while (!is_tail(tmp_elem_ptr))
            {
                flag = 1;
                struct list_item *tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
                int tmp_data = tmp_item->data;
                printf("%d ", tmp_data);
                tmp_elem_ptr = list_next(tmp_elem_ptr);
            }

            if (flag)
                printf("\n");
        }
        /* List: print the front data. */
        else if (!strcmp(command, "list_front"))
        {
            command = strtok(NULL, " ");
            struct list_elem *tmp_elem_ptr = list_begin(find_list(command));

            struct list_item *tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            printf("%d\n", tmp_item->data);
        }
        /* List: print the end data. */
        else if (!strcmp(command, "list_back"))
        {
            command = strtok(NULL, " ");
            struct list_elem *tmp_elem_ptr = list_rbegin(find_list(command));

            struct list_item *tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            printf("%d\n", tmp_item->data);
        }
        /* List: push the data at the end. */
        else if (!strcmp(command, "list_push_back"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *tmp_elem_ptr = malloc(sizeof(struct list_elem));
            struct list_item *tmp_item = malloc(sizeof(struct list_item));
            tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            tmp_item->data = atoi(strtok(NULL, " "));

            list_push_back(tmp_lists_ptr, tmp_elem_ptr);
        }
        /* List: Pop the front data. */
        else if (!strcmp(command, "list_pop_front"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *tmp_elem_ptr = list_pop_front(tmp_lists_ptr);
        }
        /* List: Pop the end data. */
        else if (!strcmp(command, "list_pop_back"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *tmp_elem_ptr = list_pop_back(tmp_lists_ptr);
        }
        /* List: Insert the element right before the first bigger one. */
        else if(!strcmp(command, "list_insert_ordered"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
            
            struct list_elem *tmp_elem_ptr = malloc(sizeof(struct list_elem));
            struct list_item *tmp_item = malloc(sizeof(struct list_item));
            tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            tmp_item->data = atoi(strtok(NULL, " "));

            list_insert_ordered(tmp_lists_ptr, tmp_elem_ptr, * less, NULL);
        }
        /* List: Insert the element at the index given. */
        else if(!strcmp(command, "list_insert"))
        {
            command = strtok(NULL, " ");
            struct list_elem *before_elem_ptr = list_begin(find_list(command));
            
            int i=0;
            int index = atoi(strtok(NULL, " "));
            while(i<index)
            {
                before_elem_ptr = list_next(before_elem_ptr);
                i++;
            }

            struct list_elem *tmp_elem_ptr = malloc(sizeof(struct list_elem));
            struct list_item *tmp_item = malloc(sizeof(struct list_item));
            tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            tmp_item->data = atoi(strtok(NULL, " "));

            list_insert(before_elem_ptr, tmp_elem_ptr);
        }
        /* List: Test whether the list is empty */
        else if(!strcmp(command, "list_empty"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
            
            if(list_empty(tmp_lists_ptr)) 
                printf("true\n");
            else printf("false\n");
        }
        /* List: Print the size of the list */
        else if(!strcmp(command, "list_size"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
           
            printf("%zu\n", list_size(tmp_lists_ptr));
        }
        /* List: Find the max value in the list */
        else if(!strcmp(command, "list_max"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
           
            struct list_elem * max_elem_ptr = list_max(tmp_lists_ptr, *less, NULL);
            struct list_item * tmp_item_ptr = list_entry(max_elem_ptr, struct list_item, elem);
            printf("%d\n", tmp_item_ptr->data);
        }
        /* List: Find the min value in the list */
        else if(!strcmp(command, "list_min"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
           
            struct list_elem * min_elem_ptr = list_min(tmp_lists_ptr, *less, NULL);
            struct list_item * tmp_item_ptr = list_entry(min_elem_ptr, struct list_item, elem);
            printf("%d\n", tmp_item_ptr->data);
        }
    }

    /* Free line buffer. */
    free(line);
    line = NULL;
    return 0;
}