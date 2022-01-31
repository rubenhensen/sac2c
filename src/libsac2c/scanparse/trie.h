#ifndef __TRIE_H__
#define __TRIE_H__

#include <sys/types.h>

#ifdef TRIE_MAIN
  typedef int bool;
#endif

#if !defined (true) || !defined (false)
# define true (!0)
# define false 0
#endif


/* Deafult number of the number of children in trie node.
   It is a good idea to pick the value being power of two
   as when the number of children has to be increase the
   number will be ultiplied by 2, and further operarions
   could be optimized to shifts rather than multiplications.  */
#define TRIE_CHILDREN	      4


/* Each child in the trie has to now if he could possibly
   end the word.  The field LAST keeps this information, and
   the fact that it is of ssize_t allows you to store there
   either a number or a pointer.  In any case you have to
   differentiate valid bnumbers from invalid, which is done
   by comparing value of LAST with TRIE_NOT_LAST.  */
#define TRIE_NOT_LAST	      -1


/* Structure to search keywords.  The search complexity is proportional to
   LW * log P, where LW is the length of the word, and P is an average number
   of words sharing the same prefix.  As an upper aproximation P is a length
   of the alphabet.

   The benefit of the structure is that with the same complexity one can
   get all the possible endings of a certain prefix.  */
struct trie;
struct child
{
  int symb;
  ssize_t last;
  struct trie *  next;
};

struct trie
{
  unsigned int children_size;
  unsigned int children_count;
  struct child *  children;
};

__BEGIN_DECLS
struct trie *  trie_new (void);
void trie_add_word (struct trie *, const char *, size_t, ssize_t);
void trie_print (struct trie *);
void trie_free (struct trie *);
ssize_t trie_search (struct trie *, const char *, size_t);
struct trie * trie_check_prefix (struct trie *, const char *, size_t, ssize_t *);
__END_DECLS

#endif  /* __TRIE_H__  */

