#include "printable_target_functions.h"
#include "memory.h"
#include "str.h"

// greatest number of characters in any single value.
static size_t Name_Max = 0;
static size_t SBI_Max = 0;
static size_t BE_Max = 0;

// this is the definition of printable targets, first metnioned in types.h.
struct PRINTABLE_TARGET {
    char *name;
    char *SBI;
    char *env;
    char *BE;
    struct PRINTABLE_TARGET *next;
};

// these are the lists to be printed:
// creates an SBI.
static printable_target* introductive = NULL;
// alters an SBI.
static printable_target* additive = NULL;
// ignores SBI.
static printable_target* neutral = NULL;

/*****************************************************************************
 *
 * static void UpdateMaximums (printable_target *list)
 *
 * description:
 *
 *   length of longest element, or max, whichever is biggest
 *
 ******************************************************************************/

static void
UpdateMaximums (printable_target *list)
{
    if (strlen (list->name) > Name_Max) {
        Name_Max = strlen (list->name);
    }
    if (strlen (list->SBI) > SBI_Max) {
        SBI_Max = strlen (list->SBI);
    }
    if (strlen (list->BE) > BE_Max) {
        BE_Max = strlen (list->BE);
    }
}

/*****************************************************************************
 *
 * static bool Contains (char *name)
 *
 * description:
 *
 *   returns true if the target already exists, and false otherwise
 *
 ******************************************************************************/

static bool
Contains (char *name)
{

    printable_target *temp = introductive;
    
    while (temp != NULL) {

        if (strcmp (name, temp->name) == 0) {
            return true;
        }

        temp = temp->next;
    }

    temp = neutral;
    
    while (temp != NULL) {

        if (strcmp (name, temp->name) == 0) {
            return true;
        }

        temp = temp->next;
    }

    temp = additive;
    
    while (temp != NULL) {

        if (strcmp (name, temp->name) == 0) {
            return true;
        }

        temp = temp->next;
    }

    return false; 
}

/******************************************************************************
 *
 * function:
 *  AddSingleTarget (printable_target *list1, printable_target *list2)
 *
 * description:
 *  This facilitates sorting, by taking a single target and adding it to an
 *  already-sorted list.
 *
 ******************************************************************************/

static printable_target *
AddSingleTarget (printable_target *list1, printable_target *list2)
{

    printable_target* current = NULL;

    //prevent perpetual looping
    list1->next = NULL;

    if (strcasecmp (list2->name, list1->name) > 0) {

        // in this scenario, the item belongs at the start of the list.
        list1->next = list2;
        return list1;
    } else {
        // in this scenario, the item belongs deeper in the list.
        current = list2;
        // go through list2 until current has an alphanumeric value > list1.
        while (current->next != NULL) {
            // This asks: should list1 come before current->next?
            if (strcasecmp (current->next->name, list1->name) > 0) {
                // here, list1 belongs between current and current->next.
                list1->next = current->next;
                current->next = list1;
                return list2;
            }
            current = current->next;
        }

        // here, list1 belongs at the end, which is the (null) current->next.
        current->next = list1;
        return list2;

    }

    return list2;
}

/******************************************************************************
 *
 * function:
 *  printable_target* AppendToSortedList  (printable_target* input)
 *
 * description:
 *  This function alphabetically sorts an input list of arbitrary length into an
 *  already-sorted output list of arbitrary length (either list can be NULL).
 *
 ******************************************************************************/
static printable_target*
AppendToSortedList (printable_target* input, printable_target* output)
{

    if (input == NULL) {
        return NULL;
    }

    if (output == NULL) {
        // pop one element from input, and put it in output.
        output = input;
        input = output->next;
        // prevent perpetual looping.
        output->next = NULL;
    }

    printable_target* temp = NULL;

    while (input != NULL) {
        // use temp so that the next element in list1 is ready for next time.
        temp = input->next;
        output = AddSingleTarget (input, output);
        input = temp;
    }

    return output;

}

/*****************************************************************************
 *
 * function:
 *    void PTFfreeAll ()
 *
 * description:
 *
 *   sets all the memory free
 *
 ******************************************************************************/

void
PTFfreeAll (void)
{

    printable_target *temp;

    while (introductive != NULL) {

        temp = introductive;
        introductive = introductive->next;

        temp->name = MEMfree (temp->name);
        temp->SBI = MEMfree (temp->SBI);
        temp->env = MEMfree (temp->env);
        temp->BE = MEMfree (temp->BE);
        temp = MEMfree (temp);

    }

    while (additive != NULL) {

        temp = additive;
        additive = additive->next;

        temp->name = MEMfree (temp->name);
        temp->SBI = MEMfree (temp->SBI);
        temp->env = MEMfree (temp->env);
        temp->BE = MEMfree (temp->BE);
        temp = MEMfree (temp);

    }

    while (neutral != NULL) {

        temp = neutral;
        neutral = neutral->next;

        temp->name = MEMfree (temp->name);
        temp->SBI = MEMfree (temp->SBI);
        temp->env = MEMfree (temp->env);
        temp->BE = MEMfree (temp->BE);
        temp = MEMfree (temp);
    }

}

/*****************************************************************************
 *
 * function:
 *    void *PTFappend (printable_target *input)
 *
 * description:
 *
 *   Adds an item to the end of the appropriate linked target list.
 *
 ******************************************************************************/

void
PTFappend (printable_target *input)
{
    // do nothing if it's an empty list to start with.
    if (input == NULL) {
        return;
    }

    // do nothing if recieving a dulpicate (shouldn't be needed)
    if (Contains (input->name)) {
        // eliminate duplicate from memory.
        input->name = MEMfree (input->name);
        input->SBI = MEMfree (input->SBI);
        input->env = MEMfree (input->env);
        input->BE = MEMfree (input->BE);
        input = MEMfree (input);
        return;
    }

    // used for printing, later.
    UpdateMaximums (input);
 
    if (strcmp (input->SBI, "XXXXX") == 0) {
        // add to neutral.
        neutral = AppendToSortedList (input, neutral);
    } else if (STRprefix ("XXXXX", input->SBI)) {
        // add to additive.
        additive = AppendToSortedList (input, additive);
    } else {
        // add to introductive.
        introductive = AppendToSortedList (input, introductive);
    }

}

/*****************************************************************************
 *
 *printable_target *PTFmake (char *name, char *SBI,
 *   char *BE, char* env, printable_target *next)
 *
 *description:
 *
 *   The build method for printable targets.
 *
 ******************************************************************************/

printable_target *
PTFmake (char *name, char *SBI, char *BE, char* env, printable_target *next)
{

    printable_target *new_list;
    new_list = (printable_target *)MEMmalloc (sizeof (printable_target));

    // STRcpy is needed to prevent the data from accidental overwriting.
    new_list->name = STRcpy (name);
    new_list->SBI = STRcpy (SBI);
    new_list->env = STRcpy (env);
    new_list->BE = STRcpy (BE);
    new_list->next = next;

    return new_list;
}

/*****************************************************************************
 *
 *int Size (printable_target *list)
 *
 *description:
 *
 *   returns number of elements
 *
 ******************************************************************************/

static int
Size (printable_target *list)
{
    printable_target *temp = list;
    int count = 0;

    while (temp != NULL) {
        temp = temp->next;
        count = count + 1;
    }

    return count;
}

/*****************************************************************************
 *
 *void PTFprint (void)
 *
 *description:
 *
 *   Prints all targets.
 *
 ******************************************************************************/

void
PTFprint (void)
{
    printf ("\n\nTARGETS:\n\n");

    printable_target* current_target = NULL;

    // Finally, print it all out.

    // NOTE: for printing spaces, the "" string is used, because for some reason
    // using the " " string causes an additional space to form. IDK why.

    printf ("Introductive targets (these targets introduce their own SBIs):\n\n");
    current_target = introductive;

    printf ("NAME:");
    printf ( "%*s", (int)(Name_Max - strlen ("NAME:")), "");
    printf (" SBI: ");
    printf ( "%*s", (int)(SBI_Max - strlen ("SBI:")), "");
    printf ("BackEnd: ");
    printf ( "%*s", (int)(BE_Max - strlen ("BackEnd:")), "");
    printf ("Environment:\n\n");

    while (current_target != NULL) {       
        printf ("%s",current_target->name);
        printf ( "%*s", (int)(Name_Max - strlen (current_target->name)), "");
        printf (" %s",current_target->SBI);
        printf ( "%*s", (int)(SBI_Max - strlen (current_target->SBI)), "");
        printf (" %s",current_target->BE);
        printf ( "%*s", (int)(BE_Max - strlen (current_target->BE)), "");
        printf (" %s\n",current_target->env);

        current_target = current_target->next;
    }
    printf ("\nThe total number of Introductive targets printed was: %d\n",
        Size (introductive));

    printf ("\n\nAdditive targets (these targets modify SBIs):\n\n");
    current_target = additive;

    printf ("NAME:");
    printf ( "%*s", (int)(Name_Max - strlen ("NAME:")), "");
    printf (" SBI: ");
    printf ( "%*s", (int)(SBI_Max - strlen ("SBI:")), "");
    printf ("BackEnd: ");
    printf ( "%*s", (int)(BE_Max - strlen ("BackEnd:")), "");
    printf ("Environment:\n\n");

    while (current_target != NULL) {       
        printf ("%s",current_target->name);
        printf ( "%*s", (int)(Name_Max - strlen (current_target->name)), "");
        printf (" %s",current_target->SBI);
        printf ( "%*s", (int)(SBI_Max - strlen (current_target->SBI)), "");
        printf (" %s",current_target->BE);
        printf ( "%*s", (int)(BE_Max - strlen (current_target->BE)), "");
        printf (" %s\n",current_target->env);

        current_target = current_target->next;
    }
    printf ("\nThe total number of Additive targets printed was: %d\n",
        Size (additive));

    printf ("\n\nNeutral targets (these have no impact on SBIs):\n\n");
    current_target = neutral;

    printf ("NAME:");
    printf ( "%*s", (int)(Name_Max - strlen ("NAME:")), "");
    printf (" SBI: ");
    printf ( "%*s", (int)(SBI_Max - strlen ("SBI:")), "");
    printf ("BackEnd: ");
    printf ( "%*s", (int)(BE_Max - strlen ("BackEnd:")), "");
    printf ("Environment:\n\n");

    while (current_target != NULL) {       
        printf ("%s",current_target->name);
        printf ( "%*s", (int)(Name_Max - strlen (current_target->name)), "");
        printf (" %s",current_target->SBI);
        printf ( "%*s", (int)(SBI_Max - strlen (current_target->SBI)), "");
        printf (" %s",current_target->BE);
        printf ( "%*s", (int)(BE_Max - strlen (current_target->BE)), "");
        printf (" %s\n",current_target->env);

        current_target = current_target->next;
    }
    printf ("\nThe total number of Neutral targets printed was: %d\n",
        Size (neutral));
    
    return;
}
