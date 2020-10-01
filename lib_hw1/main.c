#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.c"
#include "hash.c"
#include "bitmap.c"

#define MAX_SIZE 10

struct list lists[MAX_SIZE];
int lists_ptr = -1; /* pointer for lists stored */

struct hash hashtables[MAX_SIZE];
int hashs_ptr = -1; /* pointer for hashtables stored */

struct bitmap * bitmaps[MAX_SIZE];
int bitmaps_ptr = -1; /* pointer for bitmaps stored. */

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

/* Find the exact bitmap from the array of Bitmap pointers. */
struct bitmap *find_bitmap(char *finding_name)
{
    char name[10];
    strcpy(name, trim(finding_name));

    int i;
    for (i = 0; i < bitmaps_ptr; i++)
    {
        char *tmp_name = bitmap_get_name(bitmaps[i]);
        if (!strcmp(tmp_name, name))
            break;
    }
    return bitmaps[i];
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
                list_init(list_ptr);

                command = strtok(NULL, " ");
                strcpy(list_ptr->name, trim(command));
            }
            /* Create Hash table. */
            else if (!strcmp(command, "hashtable"))
            {
                struct hash *hash_ptr = &hashtables[++hashs_ptr];
                hash_init(hash_ptr, hash_function, less_hash, NULL);
                
                for (int i = 0; i < hash_ptr->bucket_cnt; i++)
                    list_init(&hash_ptr->buckets[i]);
                
                command = strtok(NULL, " ");
                strcpy(hash_ptr->name, trim(command));
            }
            /* Create Bitmap. */
            else if (!strcmp(command, "bitmap"))
            {
                command = strtok(NULL," ");
                char new_name[10];
                strcpy(new_name, trim(command));

                size_t size = atoi(strtok(NULL, " "));
                bitmaps[++bitmaps_ptr] = bitmap_create(size);
                bitmap_change_name(bitmaps[bitmaps_ptr], new_name);
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
                struct list * list_ptr = find_list(command);
                struct list_elem *tmp_elem_ptr = list_begin(list_ptr);

                while (list_tail(list_ptr)!=tmp_elem_ptr)
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
            else if (strstr(command, "bm") != NULL)
            {
                struct bitmap * bm_ptr = find_bitmap(command);
                bitmap_print(bm_ptr);                
            }
            if (flag)
                printf("\n");
        }
        /*  Bitmap: mark the bit indexed True. */
        else if(!strcmp(command, "bitmap_mark"))
        {
            command = strtok(NULL," ");
            struct bitmap * bm_ptr = find_bitmap(command);

            int bit_idx = atoi(strtok(NULL, " "));
            bitmap_mark(bm_ptr, bit_idx);
        }
        /*  Bitmap: reset the bit with the given index. */
        else if(!strcmp(command, "bitmap_reset"))
        {
            command = strtok(NULL," ");
            struct bitmap * bm_ptr = find_bitmap(command);

            int bit_idx = atoi(strtok(NULL, " "));
            bitmap_reset(bm_ptr, bit_idx);
        }
        /* Bitmap: print whether the bits in given ragne are all true. */
        else if (!strcmp(command, "bitmap_all"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));
            
            printf("%s\n", bitmap_all(bm_ptr, start_idx, cnt) ? "true" : "false");
        }
        /* Bitmap: print whether any bit in given range is true. */
        else if (!strcmp(command, "bitmap_any"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            
            printf("%s\n", bitmap_any(bm_ptr,start_idx,cnt) ? "true" : "false");
        }
        /* Bitmap: print whether the bits in given ragne are all the same as given VALUE. */
        else if (!strcmp(command, "bitmap_contains"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

            printf("%s\n", bitmap_contains(bm_ptr, start_idx, cnt, value) ? "true" : "false");
        }
        /* Bitmap: Check whether the bits in the given range is all 'not true'. */
        else if (!strcmp(command, "bitmap_none"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            printf("%s\n",bitmap_none(bm_ptr, start_idx, cnt)? "true":"false");
        }
        /* Bitmap: print the number of bits with the given VALUE in given range. */
        else if (!strcmp(command, "bitmap_count"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")),"true") ? false : true;

            printf("%zu\n", bitmap_count(bm_ptr, start_idx, cnt, value));
        }
        /* Bitmap: Dumps the contents of B to the console as hexadecimal. */
        else if (!strcmp(command, "bitmap_dump"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            bitmap_dump(bm_ptr);
        }
        /* Bitmap: Toggle the bit in given IDX. */
        else if (!strcmp(command, "bitmap_flip"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t flip_idx = atoi(strtok(NULL, " "));

            bitmap_flip(bm_ptr, flip_idx);
        }
        /* Bitmap: find the CNT consecutive bits group all with VALUE at, after START. */
        else if (!strcmp(command, "bitmap_scan"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")),"true") ? false : true;

            size_t tmp_idx = bitmap_scan(bm_ptr, start_idx, cnt, value);
            printf("%zu\n", tmp_idx);
        }
        /* Bitmap: find the CNT consecutive bits group all with VALUE at, after START. And flip them all to !VALUE */
        else if (!strcmp(command, "bitmap_scan_and_flip"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")),"true") ? false : true;

            size_t tmp_idx = bitmap_scan_and_flip(bm_ptr, start_idx, cnt, value);
            printf("%zu\n", tmp_idx);
        }
        /* Bitmap: set the bit in gieven index to VALUE. */
        else if (!strcmp(command, "bitmap_set"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t idx = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")),"true") ? false : true;

            bitmap_set(bm_ptr, idx, value);
        }
        /* Bitmap: set the all bits to VALUE */
        else if (!strcmp(command, "bitmap_set_all"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            bool value = strcmp(trim(strtok(NULL, " ")),"true") ? false : true;

            bitmap_set_all(bm_ptr, value);
        }
        /* Bitmap: set the CNT bits starting at START in B to VALUE. */
        else if (!strcmp(command, "bitmap_set_multiple"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")),"true") ? false : true;

            bitmap_set_multiple(bm_ptr, start_idx, cnt, value);
        }
        /* Bitmap: print the size of bitmap. */
        else if (!strcmp(command, "bitmap_size"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            printf("%zu\n", bitmap_size(bm_ptr));
        }
        /* Bitmap: print the value of bitmap. */
        else if (!strcmp(command, "bitmap_test"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);
            size_t idx = atoi(strtok(NULL, " "));
           
            printf("%s\n", bitmap_test(bm_ptr, idx)? "true":"false");
        }
        /* Bitmap: expand the size of bit by given size */
        else if (!strcmp(command, "bitmap_expand"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);
            int size = atoi(strtok(NULL, " "));
            bitmap_expand(bm_ptr, size);
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
        /* Hashtable: print whether the hash is empty. */
        else if(!strcmp(command, "hash_empty"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            if(hash_empty(hash_ptr))
                printf("true\n");
            else printf("false\n");
        }
        /* Hashtable: print the size of hash. */
        else if(!strcmp(command, "hash_size"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            printf("%zu\n", hash_size(hash_ptr));
        }
        /* Hashtable: remove all the elements from the hash. */
        else if(!strcmp(command, "hash_clear"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            hash_clear(hash_ptr, *destructor);
        }
        /* Hashtable: find the equal element if there it is. */
        else if(!strcmp(command, "hash_find"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            struct hash_elem *hash_elem_ptr = malloc(sizeof(struct hash_elem));
            struct hash_item *hash_item_ptr = malloc(sizeof(struct hash_item));
            hash_item_ptr = hash_entry(hash_elem_ptr, struct hash_item, elem);
            hash_item_ptr->data = atoi(strtok(NULL, " "));

            if(hash_find(hash_ptr, hash_elem_ptr)!=NULL)
                printf("%d\n", hash_item_ptr->data);
        }
        /* Hashtable: replace the NEW element with OLD one. */
        else if(!strcmp(command, "hash_replace"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            struct hash_elem *hash_elem_ptr = malloc(sizeof(struct hash_elem));
            struct hash_item *hash_item_ptr = malloc(sizeof(struct hash_item));
            hash_item_ptr = hash_entry(hash_elem_ptr, struct hash_item, elem);
            hash_item_ptr->data = atoi(strtok(NULL, " "));

            hash_replace(hash_ptr, hash_elem_ptr);
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