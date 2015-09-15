/*
 * Macros to build name tuples
 */

/*               SHP   HID   UNQ   REG   SCO   USG   BIT   DIS   CBT */
#define T_EMPTY (___, (___, (___, (___, (___, (___, (___, (___, (___, )))))))))

#define T_SHP(a, b) T_0 (a, b)
#define T_HID(a, b) T_1 (a, b)
#define T_UNQ(a, b) T_2 (a, b)
#define T_REG(a, b) T_3 (a, b)
#define T_SCO(a, b) T_4 (a, b)
#define T_USG(a, b) T_5 (a, b)
#define T_BIT(a, b) T_6 (a, b)
#define T_DIS(a, b) T_7 (a, b)
#define T_CBT(a, b) T_8 (a, b)

#define SET_NT_NAME(a, b) NT_0 (a, b)
#define SET_NT_SHP(a, b) NT_1 (a, b)
#define SET_NT_HID(a, b) NT_2 (a, b)
#define SET_NT_UNQ(a, b) NT_3 (a, b)
#define SET_NT_REG(a, b) NT_4 (a, b)
#define SET_NT_SCO(a, b) NT_5 (a, b)
#define SET_NT_USG(a, b) NT_6 (a, b)
#define SET_NT_BIT(a, b) NT_7 (a, b)
#define SET_NT_DIS(a, b) NT_8 (a, b)
#define SET_NT_CBT(a, b) NT_9 (a, b)

#define SAC_SET_NT_NAME(a, b) NT_0 (a, b)
#define SAC_SET_NT_SHP(a, b) NT_1 (a, b)
#define SAC_SET_NT_HID(a, b) NT_2 (a, b)
#define SAC_SET_NT_UNQ(a, b) NT_3 (a, b)
#define SAC_SET_NT_REG(a, b) NT_4 (a, b)
#define SAC_SET_NT_SCO(a, b) NT_5 (a, b)
#define SAC_SET_NT_USG(a, b) NT_6 (a, b)
#define SAC_SET_NT_BIT(a, b) NT_7 (a, b)
#define SAC_SET_NT_DIS(a, b) NT_8 (a, b)
#define SAC_SET_NT_CBT(a, b) NT_9 (a, b)

#define T_OLD(a) T_SHP (Item0 a, T_HID (Item1 a, T_UNQ (Item2 a, T_EMPTY)))

#define T(a, b)                                                                          \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b, (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, )))))))))

#define T_0(a, b)                                                                        \
    (a, (Item1 b,                                                                        \
         (Item2 b, (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, )))))))))
#define T_1(a, b)                                                                        \
    (Item0 b,                                                                            \
     (a, (Item2 b, (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, )))))))))
#define T_2(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (a, (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, )))))))))
#define T_3(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (Item2 b, (a, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, )))))))))
#define T_4(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (Item2 b, (Item3 b, (a, (Item5 b, (Item6 b, (Item7 b, (Item8 b, )))))))))
#define T_5(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (Item2 b, (Item3 b, (Item4 b, (a, (Item6 b, (Item7 b, (Item8 b, )))))))))
#define T_6(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (Item2 b, (Item3 b, (Item4 b, (Item5 b, (a, (Item7 b, (Item8 b, )))))))))
#define T_7(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (Item2 b, (Item3 b, (Item4 b, (Item5 b, (Item6 b, (a, (Item8 b, )))))))))
#define T_8(a, b)                                                                        \
    (Item0 b,                                                                            \
     (Item1 b, (Item2 b, (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (a, )))))))))

#define NT_0(a, b)                                                                       \
    (a,                                                                                  \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_1(a, b)                                                                       \
    (Item0 b,                                                                            \
     (a,                                                                                 \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_2(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (a,                                                                                \
       (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_3(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (a, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_4(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (a, (Item5 b, (Item6 b, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_5(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (a, (Item6 b, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_6(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (Item5 b, (a, (Item7 b, (Item8 b, (Item9 b, ))))))))))
#define NT_7(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (Item5 b, (Item6 b, (a, (Item8 b, (Item9 b, ))))))))))
#define NT_8(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (a, (Item9 b, ))))))))))
#define NT_9(a, b)                                                                       \
    (Item0 b,                                                                            \
     (Item1 b,                                                                           \
      (Item2 b,                                                                          \
       (Item3 b, (Item4 b, (Item5 b, (Item6 b, (Item7 b, (Item8 b, (a, ))))))))))
