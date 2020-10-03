#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.h"
#include "hash.h"
#include "bitmap.h"

#define MAX_SIZE 10

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

/* Store the recend idx used. It is used for deleting the element from the array. */
int list_recent_idx;
int hash_recent_idx;
int bm_recent_idx;

/* Store data structures. */
struct list lists[MAX_SIZE];
int list_available[MAX_SIZE] = {0};

struct hash hashtables[MAX_SIZE];
int hash_available[MAX_SIZE] = {0};

struct bitmap *bitmaps[MAX_SIZE];
int bitmap_available[MAX_SIZE] = {0};

/* Return the proper index for a new data structure. */
int find_lists_idx()
{
    for (int i = 0; i < MAX_SIZE; i++)
        if (list_available[i] == 0)
            return i;
}

int find_hashtables_idx()
{
    for (int i = 0; i < MAX_SIZE; i++)
        if (hash_available[i] == 0)
            return i;
}

int find_bitmaps_idx()
{
    for (int i = 0; i < MAX_SIZE; i++)
        if (bitmap_available[i] == 0)
            return i;
}

/* Find the exact list from the array of Lists. */
struct list *find_list(char *finding_name)
{
    finding_name = trim(finding_name);
    for (int i = 0; i < MAX_SIZE; i++)
        if (list_available[i] != 0 && !strcmp(finding_name, lists[i].name))
        {
            list_recent_idx = i;
            return &lists[i];
        }
    return NULL;
}

/* Find the exact hashtable from the array of Hashtables. */
struct hash *find_hashtable(char *finding_name)
{
    finding_name = trim(finding_name);
    for (int i = 0; i < MAX_SIZE; i++)
        if (hash_available[i] != 0 && !strcmp(finding_name, hashtables[i].name))
        {
            hash_recent_idx = i;
            return &hashtables[i];
        }
    return NULL;
}

/* Find the exact bitmap from the array of Bitmap pointers. */
struct bitmap *find_bitmap(char *finding_name)
{
    char name[10];
    strcpy(name, trim(finding_name));

    int i;
    for (i = 0; i < MAX_SIZE; i++)
    {
        char *tmp_name = bitmap_get_name(bitmaps[i]);
        if (bitmap_available[i] != 0 && !strcmp(tmp_name, name))
        {
            bm_recent_idx = i;
            return bitmaps[i];
        }
    }
    return NULL;
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
                int idx = find_lists_idx();
                list_available[idx] = 1;
                struct list *list_ptr = &lists[idx];
                list_init(list_ptr);

                command = strtok(NULL, " ");
                strcpy(list_ptr->name, trim(command));
            }
            /* Create Hash table. */
            else if (!strcmp(command, "hashtable"))
            {
                int idx = find_hashtables_idx();
                hash_available[idx] = 1;
                struct hash *hash_ptr = &hashtables[idx];
                hash_init(hash_ptr, hash_function, less_hash, NULL);

                for (int i = 0; i < hash_ptr->bucket_cnt; i++)
                    list_init(&hash_ptr->buckets[i]);

                command = strtok(NULL, " ");
                strcpy(hash_ptr->name, trim(command));
            }
            /* Create Bitmap. */
            else if (!strcmp(command, "bitmap"))
            {
                int idx = find_bitmaps_idx();
                bitmap_available[idx] = 1;
                command = strtok(NULL, " ");
                char new_name[10];
                strcpy(new_name, trim(command));

                size_t size = atoi(strtok(NULL, " "));
                bitmaps[idx] = bitmap_create(size);
                bitmap_set_name(bitmaps[idx], new_name);
            }
        }
        /* Delete the data structure. */
        else if (!strcmp(command, "delete"))
        {
            command = strtok(NULL, " ");

            struct list *list_ptr = find_list(command);
            struct hash *hash_ptr = find_hashtable(command);
            struct bitmap *bm_ptr = find_bitmap(command);

            if (list_ptr!= NULL)
            {
                struct list *list_ptr = find_list(command);
                struct list_item *dummy;
                struct list_elem *begin = list_begin(list_ptr);

                while (begin!=list_tail(list_ptr))
                {
                    dummy = list_entry(begin, struct list_item, elem);
                    begin = list_next(begin);
                    free(dummy);
                }

                list_init(list_ptr);
                list_available[list_recent_idx] = 0;
            }
            /* Hashtable: delete. */
            else if (hash_ptr != NULL)
            {
                struct hash *hash_ptr = find_hashtable(command);
                hash_destroy(hash_ptr, *destructor);
                hash_init(&hashtables[hash_recent_idx], NULL, NULL, NULL);
                hash_available[hash_recent_idx] = 0;
            }
            /* Bitmap: delete. */
            else if (bm_ptr!= NULL)
            {
                struct bitmap *bm_ptr = find_bitmap(command);
                bitmap_destroy(bm_ptr);
                bitmaps[bm_recent_idx] = NULL;
                bitmap_available[bm_recent_idx] = 0;
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
                struct list *list_ptr = find_list(command);
                struct list_elem *tmp_elem_ptr = list_begin(list_ptr);

                while (list_tail(list_ptr) != tmp_elem_ptr)
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
                struct bitmap *bm_ptr = find_bitmap(command);
                bitmap_print(bm_ptr);
            }
            if (flag)
                printf("\n");
        }
        /*  Bitmap: mark the bit indexed True. */
        else if (!strcmp(command, "bitmap_mark"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            int bit_idx = atoi(strtok(NULL, " "));
            bitmap_mark(bm_ptr, bit_idx);
        }
        /*  Bitmap: reset the bit with the given index. */
        else if (!strcmp(command, "bitmap_reset"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

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

            printf("%s\n", bitmap_any(bm_ptr, start_idx, cnt) ? "true" : "false");
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

            printf("%s\n", bitmap_none(bm_ptr, start_idx, cnt) ? "true" : "false");
        }
        /* Bitmap: print the number of bits with the given VALUE in given range. */
        else if (!strcmp(command, "bitmap_count"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

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

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

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

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

            size_t tmp_idx = bitmap_scan_and_flip(bm_ptr, start_idx, cnt, value);
            printf("%zu\n", tmp_idx);
        }
        /* Bitmap: set the bit in gieven index to VALUE. */
        else if (!strcmp(command, "bitmap_set"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t idx = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

            bitmap_set(bm_ptr, idx, value);
        }
        /* Bitmap: set the all bits to VALUE */
        else if (!strcmp(command, "bitmap_set_all"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

            bitmap_set_all(bm_ptr, value);
        }
        /* Bitmap: set the CNT bits starting at START in B to VALUE. */
        else if (!strcmp(command, "bitmap_set_multiple"))
        {
            command = strtok(NULL, " ");
            struct bitmap *bm_ptr = find_bitmap(command);

            size_t start_idx = atoi(strtok(NULL, " "));
            size_t cnt = atoi(strtok(NULL, " "));

            bool value = strcmp(trim(strtok(NULL, " ")), "true") ? false : true;

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

            printf("%s\n", bitmap_test(bm_ptr, idx) ? "true" : "false");
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
        else if (!strcmp(command, "hash_delete"))
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
        else if (!strcmp(command, "hash_empty"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            if (hash_empty(hash_ptr))
                printf("true\n");
            else
                printf("false\n");
        }
        /* Hashtable: print the size of hash. */
        else if (!strcmp(command, "hash_size"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            printf("%zu\n", hash_size(hash_ptr));
        }
        /* Hashtable: remove all the elements from the hash. */
        else if (!strcmp(command, "hash_clear"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            hash_clear(hash_ptr, *destructor);
        }
        /* Hashtable: find the equal element if there it is. */
        else if (!strcmp(command, "hash_find"))
        {
            command = strtok(NULL, " ");
            struct hash *hash_ptr = find_hashtable(command);

            struct hash_elem *hash_elem_ptr = malloc(sizeof(struct hash_elem));
            struct hash_item *hash_item_ptr = malloc(sizeof(struct hash_item));
            hash_item_ptr = hash_entry(hash_elem_ptr, struct hash_item, elem);
            hash_item_ptr->data = atoi(strtok(NULL, " "));

            if (hash_find(hash_ptr, hash_elem_ptr) != NULL)
                printf("%d\n", hash_item_ptr->data);
        }
        /* Hashtable: replace the NEW element with OLD one. */
        else if (!strcmp(command, "hash_replace"))
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
            struct list *list_ptr = find_list(command);

            int index = atoi(strtok(NULL, " "));

            struct list_elem *tmp_elem_ptr = malloc(sizeof(struct list_elem));
            struct list_item *tmp_item = malloc(sizeof(struct list_item));
            tmp_item = list_entry(tmp_elem_ptr, struct list_item, elem);
            tmp_item->data = atoi(strtok(NULL, " "));

            list_insert(list_find_elem_by_index(list_ptr, index), tmp_elem_ptr);
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
        /* List:  */
        else if (!strcmp(command, "list_splice"))
        {
            command = strtok(NULL, " ");
            struct list *list1 = find_list(command);
            int before = atoi(strtok(NULL, " "));

            command = strtok(NULL, " ");
            struct list *list2 = find_list(command);
            int first = atoi(strtok(NULL, " "));
            int last = atoi(strtok(NULL, " "));

            struct list_elem * before_elem = list_find_elem_by_index(list1, before);
            struct list_elem * first_elem = list_find_elem_by_index(list2, first);
            struct list_elem * last_elem = list_find_elem_by_index(list2, last);

            list_splice( before_elem, first_elem, last_elem);
        }
        /* List: Move duplicated value to the second list. */
        else if (!strcmp(command, "list_unique"))
        {
            command = strtok(NULL, " ");
            struct list *tmp_lists_ptr = find_list(command);
            command = strtok(NULL, " ");
            struct list *duplicate_lists_ptr;

            if (command != NULL)
                duplicate_lists_ptr = find_list(command);

            else {
                duplicate_lists_ptr = malloc(sizeof(struct list));
                list_init(duplicate_lists_ptr);
            }
            list_unique(tmp_lists_ptr, duplicate_lists_ptr, *less_list, NULL);
        }
        /* List: Swap two elements with the given index. */
        else if (!strcmp(command, "list_swap"))
        {
            command = strtok(NULL, " ");
            struct list *list_ptr = find_list(command);

            int first_i = atoi(strtok(NULL, " "));
            int second_i = atoi(strtok(NULL, " "));

            /* Find the first element. */
            struct list_elem *first_elem_ptr = list_find_elem_by_index(list_ptr, first_i);
            struct list_elem *second_elem_ptr = list_find_elem_by_index(list_ptr, second_i);

            list_swap(first_elem_ptr, second_elem_ptr);
        }
    }

    /* Free line buffer. */
    free(line);
    line = NULL;
    return 0;
}