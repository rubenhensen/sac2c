#include <string.h>
#include "memory.h"
#include "ctinfo.h"
#include "hash_table.h"

static void *_HTfold (ht_t *, void *, void *(*)(void *, void *, void *));

/*
 * This hashing algorithm is created on the basis of the "Murmur 2.0" hashing
 * algorithm by Austin Appleby (source sites.google.com/site/murmurhash)
 */
static unsigned int
hashfunc (void *key)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    unsigned int k, i;
    unsigned int hash = 0;
    const unsigned char *data = (const unsigned char *)&key;

    for (i = sizeof (void *); i >= 4; i = i - 4) {
        k = *(unsigned int *)data;
        k += m;
        k ^= k >> r;
        k *= m;
        hash *= m;
        hash ^= k;
        data += 4;
    }

    switch (i) {
    case 3:
        hash ^= data[2] << 16;
    case 2:
        hash ^= data[1] << 8;
    case 1:
        hash ^= data[0];
        hash *= m;
    }

    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;

    return hash;
}

void *
WrapResizeInsert (void *new, void *key, void *value)
{
    new = HTinsert (new, key, value);
    return new;
}

ht_t *
HTresize (ht_t *old)
{

    ht_t *new = HTcreate (old->size * 2);

    _HTfold (old, new, WrapResizeInsert);

    HTdestroy (old);

    return new;
}

ht_t *
HTcopy (ht_t *ht)
{

    ht_t *copy = HTcreate (ht->size);

    _HTfold (ht, copy, WrapResizeInsert);

    return copy;
}

ht_t *
HTcreate (int size)
{
    ht_t *ht = MEMmalloc (sizeof (ht_t));

    size = size == 0 ? HT_DEFAULT_SIZE : size;
    ht->table = MEMmalloc (sizeof (ht_entry_t *) * size);
    memset (ht->table, 0, sizeof (ht_entry_t *) * size);

    ht->current = 0;
    ht->fill = 0;
    ht->size = size;
    ht->initsize = size;
    return ht;
}

ht_t *
HTinsert (ht_t *ht, void *key, void *value)
{
    if (ht->fill > HT_MAX_FILL * ht->size) {
        ht = HTresize (ht);
    }

    /*
     * Our hash table buckets consist of linked lists, if the key is already
     * present we prepend the new value in this list
     */
    unsigned int hash = hashfunc (key) % ht->size;
    ht_entry_t *entry = MEMmalloc (sizeof (ht_entry_t));
    entry->key = key;
    entry->value = value;

    while (ht->table[hash] != NULL) {
        if (ht->table[hash]->key == key) {
            break;
        }
        hash = (hash + 1) % ht->size;
    }

    entry->next = ht->table[hash];
    ht->table[hash] = entry;
    ht->fill++;
    return ht;
}

void *
HTlookup (ht_t *ht, void *key)
{
    unsigned int hash = hashfunc (key) % ht->size;
    void *value = NULL;

    while (ht->table[hash] != NULL) {
        if (ht->table[hash]->key == key) {
            value = ht->table[hash]->value;
            ht->current = ht->table[hash];
            break;
        }
        hash = (hash + 1) % ht->size;
    }
    return value;
}

void *
HTlookupNext (ht_t *ht)
{
    void *value = NULL;

    ht->current = ht->current->next;
    value = ht->current->value;
    return value;
}

static void *
_HTfold (ht_t *ht, void *init, void *(*func) (void *, void *, void *))
{
    ht_entry_t *iterator;

    for (int i = 0; i < ht->size; i++) {
        iterator = ht->table[i];
        while (iterator) {
            init = func (init, iterator->key, iterator->value);
            iterator = iterator->next;
        }
    }

    return init;
}

void *
HTfold (ht_t *ht, void *init, void *(*func) (void *, void *, void *))
{
    /*
     * The fold function first makes a copy of the hash-table
     * This is necessary to allow deletion of an entry during a fold
     * Because if something is deleted from the hash-table we have no way of
     * knowing and this could lead to a segmentation fault while traversing
     */
    ht_t *copy = HTcopy (ht);
    init = _HTfold (copy, init, func);
    HTdestroy (copy);
    return init;
}

void
HTdestroy (ht_t *ht)
{
    ht_entry_t *iterator;
    ht_entry_t *next;
    for (int i = 0; i < ht->size; i++) {
        iterator = ht->table[i];
        while (iterator) {
            next = iterator->next;
            MEMfree (iterator);
            iterator = next;
        }
    }

    MEMfree (ht->table);
    MEMfree (ht);
}

void
HTdelete (ht_t *ht, void *key)
{
    unsigned int hash = hashfunc (key) % ht->size;
    unsigned int begin = hash;
    ht_entry_t *iterator;
    ht_entry_t *next;

    while (ht->table[hash] != NULL) {
        if (ht->table[hash]->key == key) {
            iterator = ht->table[hash];
            while (iterator) {
                next = iterator->next;
                MEMfree (iterator);
                iterator = next;
                ht->fill--;
            }
            ht->table[hash] = NULL;
        }

        hash = (hash + 1) % ht->size;

        if (hash == begin) {
            break;
        }
    }
    return;
}
