/*
 * Macros to build name tuples
 */

#define T_EMPTY (___, (___, (___, (___, (___, (___, ))))))

#define T_SHP(a, b) T_0 (a, b)
#define T_HID(a, b) T_1 (a, b)
#define T_UNQ(a, b) T_2 (a, b)
#define T_SCL(a, b) T_3 (a, b)

#define T_OLD(a) T_SHP (Item0 a, T_HID (Item1 a, T_UNQ (Item2 a, T_EMPTY)))

#define T(a, b) (Item0 b, (Item1 b, (Item2 b, (Item3 b, (Item4 b, (Item5 b, ))))))

#define T_0(a, b) (a, (Item1 b, (Item2 b, (Item3 b, (Item4 b, (Item5 b, ))))))
#define T_1(a, b) (Item0 b, (a, (Item2 b, (Item3 b, (Item4 b, (Item5 b, ))))))
#define T_2(a, b) (Item0 b, (Item1 b, (a, (Item3 b, (Item4 b, (Item5 b, ))))))
#define T_3(a, b) (Item0 b, (Item1 b, (Item2 b, (a, (Item4 b, (Item5 b, ))))))
#define T_4(a, b) (Item0 b, (Item1 b, (Item2 b, (Item3 b, (a, (Item5 b, ))))))
