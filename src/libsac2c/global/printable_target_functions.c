#include "printable_target_functions.h"
#include "memory.h"
#include "str.h"

// Greatest number of characters in any single value.
static size_t NameMax = 0;
static size_t SBIMax = 0;
static size_t BEMax = 0;

// This is the definition of printable targets, first metnioned in types.h.
struct PRINTABLE_TARGET {
    char *name;
    char *SBI;
    char *env;
    char *BE;
    struct PRINTABLE_TARGET *next;
};

// These are the lists to be printed:
// Creates an SBI.
static printable_target* introductive = NULL;
// Alters an SBI.
static printable_target* additive = NULL;
// Ignores SBI.
static printable_target* neutral = NULL;

/**
 * length of longest element, or max, whichever is biggest
 */

static void
UpdateMaximums (printable_target *list)
{
    if (STRlen (list->name) > NameMax) {
        NameMax = STRlen (list->name);
    }
    if (STRlen (list->SBI) > SBIMax) {
        SBIMax = STRlen (list->SBI);
    }
    if (STRlen (list->BE) > BEMax) {
        BEMax = STRlen (list->BE);
    }
}

/**
 * Returns true if the target already exists, and false otherwise
 */

static bool
Contains (char *name)
{

    printable_target *temp = introductive;
    
    while (temp != NULL) {

        if (STReq (name, temp->name)) {
            return true;
        }

        temp = temp->next;
    }

    temp = neutral;
    
    while (temp != NULL) {

        if (STReq (name, temp->name)) {
            return true;
        }

        temp = temp->next;
    }

    temp = additive;
    
    while (temp != NULL) {

        if (STReq (name, temp->name)) {
            return true;
        }

        temp = temp->next;
    }

    return false; 
}

/**
 * This facilitates sorting, by taking a single target and adding it to an
 * already-sorted list.
 */

static printable_target *
AddSingleTarget (printable_target *list1, printable_target *list2)
{

    printable_target* current = NULL;

    // Prevent perpetual looping
    list1->next = NULL;

    if (strcasecmp (list2->name, list1->name) > 0) {

        // In this scenario, the item belongs at the start of the list.
        list1->next = list2;
        return list1;
    } else {
        // In this scenario, the item belongs deeper in the list.
        current = list2;
        // Go through list2 until current has an alphanumeric value > list1.
        while (current->next != NULL) {
            // This asks: should list1 come before current->next?
            if (strcasecmp (current->next->name, list1->name) > 0) {
                // Here, list1 belongs between current and current->next.
                list1->next = current->next;
                current->next = list1;
                return list2;
            }
            current = current->next;
        }

        // Here, list1 belongs at the end, which is the (null) current->next.
        current->next = list1;
        return list2;

    }
}

/**
 * This function alphabetically sorts an input list of arbitrary length into an
 * already-sorted output list of arbitrary length (either list can be NULL).
 */
static printable_target*
AppendToSortedList (printable_target* input, printable_target* output)
{

    if (input == NULL) {
        return NULL;
    }

    if (output == NULL) {
        // Pop one element from input, and put it in output.
        output = input;
        input = output->next;
        // Prevent perpetual looping.
        output->next = NULL;
    }

    printable_target* temp = NULL;

    while (input != NULL) {
        // Use temp so that the next element in list1 is ready for next time.
        temp = input->next;
        output = AddSingleTarget (input, output);
        input = temp;
    }

    return output;

}

/**
 * Sets all the memory used by the structs free.
 */

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

/**
 * Adds an item to the end of the appropriate linked target list.
 */

void
PTFappend (printable_target *input)
{
    // Do nothing if it's an empty list to start with.
    if (input == NULL) {
        return;
    }

    // Do nothing if recieving a dulpicate (shouldn't be needed)
    if (Contains (input->name)) {
        // Eliminate duplicate from memory.
        input->name = MEMfree (input->name);
        input->SBI = MEMfree (input->SBI);
        input->env = MEMfree (input->env);
        input->BE = MEMfree (input->BE);
        input = MEMfree (input);
        return;
    }

    // Used for printing, later.
    UpdateMaximums (input);
 
    if (STReq (input->SBI, "XXXXX")) {
        // Add to neutral.
        neutral = AppendToSortedList (input, neutral);
    } else if (STRprefix ("XXXXX", input->SBI)) {
        // Add to additive.
        additive = AppendToSortedList (input, additive);
    } else {
        // Add to introductive.
        introductive = AppendToSortedList (input, introductive);
    }

}

/**
 * The build method for printable targets.
 */

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

/**
 * Returns the number of elements in a given list.
 */

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

/**
 * Facilitates PTFprint
 */

static void
PartialPrint (printable_target* current_target)
{
    // NOTE: for printing spaces, the "" string is used, because for some reason
    // using the " " string causes an additional space to form. IDK why.

    printf ("NAME:");
    printf ( "%*s", (int)(NameMax - STRlen ("NAME:")), "");
    printf (" SBI: ");
    printf ( "%*s", (int)(SBIMax - STRlen ("SBI:")), "");
    printf ("BackEnd: ");
    printf ( "%*s", (int)(BEMax - STRlen ("BackEnd:")), "");
    printf ("Environment:\n\n");

    while (current_target != NULL) {       
        printf ("%s",current_target->name);
        printf ( "%*s", (int)(NameMax - STRlen (current_target->name)), "");
        printf (" %s",current_target->SBI);
        printf ( "%*s", (int)(SBIMax - STRlen (current_target->SBI)), "");
        printf (" %s",current_target->BE);
        printf ( "%*s", (int)(BEMax - STRlen (current_target->BE)), "");
        printf (" %s\n",current_target->env);

        current_target = current_target->next;
    }
}

/**
 * Prints all targets.
 */

void
PTFprint (void)
{
    printf ("\n\nTARGETS:\n\n");

    printf ("Introductive targets (these targets introduce their own SBIs):\n\n");
    PartialPrint (introductive);
    printf ("\nThe total number of Introductive targets printed was: %d\n",
            Size (introductive));

    printf ("\n\nAdditive targets (these targets modify SBIs):\n\n");
    PartialPrint (additive);
    printf ("\nThe total number of Additive targets printed was: %d\n",
            Size (additive));

    printf ("\n\nNeutral targets (these have no impact on SBIs):\n\n");
    PartialPrint (neutral);
    printf ("\nThe total number of Neutral targets printed was: %d\n",
            Size (neutral));
    
    return;
}
