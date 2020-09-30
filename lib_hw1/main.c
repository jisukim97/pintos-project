#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.c"
#include "hash.c"

#define MAX_SIZE 10

struct list lists[MAX_SIZE];
int lists_ptr = -1; /* pointer for lists stored */

struct hash hashtables[MAX_SIZE];
int hashs_ptr = -1; /* pointer for hashtables stored */

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

/* Find the exact hashtable from the array of Hashtables. */
struct hash *find_hashtable(char *finding_name)
{
    finding_name = trim(finding_name);
    for (int i = 0; i <= hashs_ptr; i++)
        if (!strcmp(finding_name, hashtables[i].name))
            return &hashtables[i];
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

                command = strtok(NULL, " ");
                strcpy(list_ptr->name, trim(command));
            }
            /* Create Hash table. */
            else if (!strcmp(command, "hashtable"))
            {
                struct hash *hash_ptr = &hashtables[++hashs_ptr];
                hash_init(hash_ptr, hash_function, less_hash, NULL);
                hash_ptr->elem_cnt = 0;
                hash_ptr->bucket_cnt = 4;
                hash_ptr->buckets = malloc(sizeof(*(hash_ptr->buckets)) * hash_ptr->bucket_cnt);
                hash_ptr->hash = hash_function;
                hash_ptr->less = less_hash;
                hash_ptr->aux = NULL;

                for (int i = 0; i < hash_ptr->bucket_cnt; i++)
                {
                    list_init(&hash_ptr->buckets[i]);
                }

                command = strtok(NULL, " ");
                strcpy(hash_ptr->name, trim(command));
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

            /* List: delete. */
            if (strstr(command, "list") != NULL)
            {
                list_init(find_list(command));
            }
            /* Hashtable: delete. */
            else if (strstr(command, "hash") != NULL)
            {
                hash_init(find_hashtable(command), NULL, NULL, NULL);
            }
            /* Bitmap: delete. */
            else if (strstr(command, "bitmap") != NULL)
            {
            }
        }
        /* Enumerate data in the structure. */
        else if (!strcmp(command, "dumpdata"))
        {
            command = strtok(NULL, " ");
            int flag = 0;

            /* List: enumerate. */
            if (strstr(command, "list") != NULL)
            {
                struct list_elem *tmp_elem_ptr = list_begin(find_list(command));

                while (!is_tail(tmp_elem_ptr))
                {
                    flag = 1;
                    struct list_item *tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
                    int tmp_data = tmp_item->data;
                    printf("%d ", tmp_data);
                    tmp_elem_ptr = list_next(tmp_elem_ptr);
                }
            }
            /* Hashtable: enumerate. */
            else if (strstr(command, "hash") != NULL)
            {
                int flag = 0;
                struct hash *hash_ptr = find_hashtable(command);
                struct hash_iterator i;

                hash_first(&i, hash_ptr);
                while (hash_next(&i))
                {
                    flag = 1;
                    struct hash_item *hash_item_ptr = hash_entry(hash_cur(&i), struct hash_item, elem);
                    printf("%d ", hash_item_ptr->data);
                }

                if (flag)
                    printf("\n");
            }
            /* Bitmap: enumerate. */
            else if (strstr(command, "bitmap") != NULL)
            {
            }
            if (flag)
                printf("\n");
        }

        /* Hashtable: insert the given data. */
        else if (!strcmp(command, "hash_insert"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            struct hash_elem *hash_elem_ptr = malloc(sizeof(struct hash_elem));
            struct hash_item *hash_item_ptr = malloc(sizeof(struct hash_item));
            hash_item_ptr = hash_entry(hash_elem_ptr, struct hash_item, elem);
            hash_item_ptr->data = atoi(strtok(NULL, " "));

            hash_insert(hash_ptr, hash_elem_ptr);
        }
        /* Hashtable: apply the operation to Hashtable. */
        else if (!strcmp(command, "hash_apply"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            command = strtok(NULL, "\n");
            if (!strcmp(command, "square"))
                hash_apply(hash_ptr, *square);
            else if (!strcmp(command, "triple"))
                hash_apply(hash_ptr, *triple);
        }
        /* Hashtable: delete the element in hashtable. */
        else if(!strcmp(command, "hash_delete"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);
    
            struct hash_elem *hash_elem_ptr = malloc(sizeof(struct hash_elem));
            struct hash_item *hash_item_ptr = malloc(sizeof(struct hash_item));
            hash_item_ptr = hash_entry(hash_elem_ptr, struct hash_item, elem);
            hash_item_ptr->data = atoi(strtok(NULL, " "));

            hash_delete(hash_ptr, hash_elem_ptr);
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
        /* List: push the data at the front. */
        else if (!strcmp(command, "list_push_front"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *tmp_elem_ptr = malloc(sizeof(struct list_elem));
            struct list_item *tmp_item = malloc(sizeof(struct list_item));
            tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            tmp_item->data = atoi(strtok(NULL, " "));

            list_push_front(tmp_lists_ptr, tmp_elem_ptr);
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
        else if (!strcmp(command, "list_insert_ordered"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *tmp_elem_ptr = malloc(sizeof(struct list_elem));
            struct list_item *tmp_item = malloc(sizeof(struct list_item));
            tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            tmp_item->data = atoi(strtok(NULL, " "));

            list_insert_ordered(tmp_lists_ptr, tmp_elem_ptr, *less_list, NULL);
        }
        /* List: Insert the element at the index given. */
        else if (!strcmp(command, "list_insert"))
        {
            command = strtok(NULL, " ");
            struct list_elem *before_elem_ptr = list_begin(find_list(command));

            int i = 0;
            int index = atoi(strtok(NULL, " "));
            while (i < index)
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
        else if (!strcmp(command, "list_empty"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            if (list_empty(tmp_lists_ptr))
                printf("true\n");
            else
                printf("false\n");
        }
        /* List: Print the size of the list */
        else if (!strcmp(command, "list_size"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            printf("%zu\n", list_size(tmp_lists_ptr));
        }
        /* List: Find the max value in the list */
        else if (!strcmp(command, "list_max"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *max_elem_ptr = list_max(tmp_lists_ptr, *less_list, NULL);
            struct list_item *tmp_item_ptr = list_entry(max_elem_ptr, struct list_item, elem);
            printf("%d\n", tmp_item_ptr->data);
        }
        /* List: Find the min value in the list */
        else if (!strcmp(command, "list_min"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            struct list_elem *min_elem_ptr = list_min(tmp_lists_ptr, *less_list, NULL);
            struct list_item *tmp_item_ptr = list_entry(min_elem_ptr, struct list_item, elem);
            printf("%d\n", tmp_item_ptr->data);
        }
        /* List: Remove an element of the given index. */
        else if (!strcmp(command, "list_remove"))
        {
            command = strtok(NULL, " ");
            struct list_elem *remove_elem_ptr = list_begin(find_list(command));

            int i = 0;
            int index = atoi(strtok(NULL, " "));
            while (i < index)
            {
                remove_elem_ptr = list_next(remove_elem_ptr);
                i++;
            }

            list_remove(remove_elem_ptr);
        }
        /* List: Reverse the original order of the list. */
        else if (!strcmp(command, "list_reverse"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            list_reverse(tmp_lists_ptr);
        }
        /* List: Shuffle the original order of the list. */
        else if (!strcmp(command, "list_shuffle"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            list_shuffle(tmp_lists_ptr);
        }
        /* List: Sort the list. */
        else if (!strcmp(command, "list_sort"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);

            list_sort(tmp_lists_ptr, *less_list, NULL);
        }
        /* List: Move duplicated value to the second list. */
        else if (!strcmp(command, "list_unique"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
            command = strtok(NULL, " ");
            struct list *duplicate_lists_ptr = find_list(command);

            list_unique(tmp_lists_ptr, duplicate_lists_ptr, *less_list, NULL);
        }
        /* List: Swap two elements with the given index. */
        else if (!strcmp(command, "list_swap"))
        {
            command = strtok(NULL, " ");
            struct list_elem *first_elem_ptr = list_begin(find_list(command));
            struct list_elem *second_elem_ptr = first_elem_ptr;

            int first_i = atoi(strtok(NULL, " "));
            int second_i = atoi(strtok(NULL, " "));

            /* Find the first element. */
            int i = 0;
            while (i < first_i)
            {
                first_elem_ptr = list_next(first_elem_ptr);
                i++;
            }

            /* Find the second element. */
            i = 0;
            while (i < second_i)
            {
                second_elem_ptr = list_next(second_elem_ptr);
                i++;
            }

            list_swap(first_elem_ptr, second_elem_ptr);
        }
    }

    /* Free line buffer. */
    free(line);
    line = NULL;
    return 0;
}