#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_
#define HT_DEFAULT_SIZE 100
#define HT_MAX_FILL 0.8

typedef struct ht_entry_t {
    void *key;
    void *value;
    struct ht_entry_t *next;
} ht_entry_t;

typedef struct ht_t {
    ht_entry_t **table;
    ht_entry_t *current;
    int fill;
    int size;
    int initsize;
} ht_t;

/*
 * Create a Hash table with given size
 * when HT_MAX_FILL of the HT is filled, the size is doubled, this is an expensive
 * operation so try to avoid it by picking the right size!
 */
ht_t *HTcreate (int);

/*
 * Inserts a key-value (k,v) pair in the hash-table,
 * Multiple values (a,b) (a, c) are allowed!
 */
ht_t *HTinsert (ht_t *, void *, void *);
ht_t *HTcopy (ht_t *);

void *HTlookup (ht_t *, void *);
void *HTlookupNext (ht_t *);
void *HTfold (ht_t *, void *init, void *(*)(void *init, void *key, void *value));

/*
 * HTdelete deletes every entry of a given key
 */
void HTdelete (ht_t *, void *key);

void HTdestroy (ht_t *);

#endif /*_HASH_TABLE_H_*/
