// gehören die typedefs hier rein?
#define ULINT unsigned long int

typedef enum eWritePolicy { unknown } tWritePolicy;

typedef struct sCacheLevel {
    int associativity;
    int cachelinesize;
    tWritePolicy writepolicy;
    ULINT cachesize;
    ULINT setsize;
    ULINT cls_mask;
    int ss_bits;
    ULINT ss_mask;
    ULINT max_cachelineindex;
                            /*tDynArray    data;*/ }
tCacheLevel;

                            extern void SAC_CPF_Initialize (
                              int nr_of_cpu, ULINT cachesize1, int cachelinesize1,
                              int associativity1, tWritePolicy writepolicy1,
                              ULINT cachesize2, int cachelinesize2, int associativity2,
                              tWritePolicy writepolicy2, ULINT cachesize3,
                              int cachelinesize3, int associativity3,
                              tWritePolicy writepolicy3);
                            /* Initialisiert die Cache-Strukturen für die angegebenen
                             * cachelevel aller CPUs
                             *
                             * Die Angabe der cachesize erfolgt in KB (1KByte=1024Byte).
                             * Eine cachesize von 0 KB bedeutet, daß dieser cachelevel
                             * nicht existiert. Die Angabe der cachelinesize erfolgt in
                             * Byte Dabei muss gelten: cachesize mod cachelinesize = 0
                             * fuer die associativity muss gelten:
                             *  +  1 <= associativity <= cachesize/cachelinesize
                             *  +  associativity=1                       -> direct mapped
                             * cache
                             *  +  associativity=cachesize/cachelinesize -> full
                             * associative cache Die Angabe der writepolicy gibt die
                             * Rückschreibestrategie an.
                             */

                            extern void SAC_CPF_Finalize (void);
                            /* Gibt verwendeten Speicher der Cache-Strukturen aller
                             * cachelevel aller CPU's wieder frei
                             */

                            extern void SAC_CPF_RegisterArray (void *baseaddress,
                                                               int size);
                            /* Registriert ein Array, welches durch seine baseaddress
                             * identifiziert wird, für das CPF
                             *
                             * Die Angabe der size erfolgt in Bytes und gibt die Groesse
                             * eines eindimensionalen Arrays an
                             */

                            extern void SAC_CPF_UnregisterArray (void *baseaddress);
                            /* Registrierung des Arrays, welches durch die baseaddress
                             * identifiziert wird, für das CPF wird aufgehoben
                             */

                            extern void SAC_CPF_ReadAccess (void *baseaddress,
                                                            void *elemaddress);
                            /* Lesender Zugriff auf ein Element (gegeben durch die
                             * elemaddress) eines (auch unregistrierten) Arrays (gegeben
                             * durch seine baseaddress)
                             */

                            extern void SAC_CPF_WriteAccess (void *baseaddress,
                                                             void *elemaddress);
                            /* Schreibender Zugriff auf ein Element (gegeben durch die
                             * elemaddress) eines (auch unregistrierten) Arrays (gegeben
                             * durch seine baseaddress)
                             */

                            extern void SAC_CPF_ShowResults (void);
                            /* Gibt das Resultat des CPF auf der stdout aus
                             */
