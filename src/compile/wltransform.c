/*
 *
 * $Log$
 * Revision 1.30  1998/08/07 18:03:53  dkr
 * a bug in CheckParams fixed
 *
 * Revision 1.27  1998/08/06 01:16:12  dkr
 * fixed some minor bugs
 *
 * Revision 1.26  1998/07/08 13:02:00  dkr
 * added ; behind DBUG_VOID_RETURN
 *
 * Revision 1.25  1998/06/16 13:59:02  dkr
 * fixed a bug:
 *   call of NormalizeWL now with correct WLSEG_IDX_MAX
 *
 * Revision 1.24  1998/06/09 16:47:24  dkr
 * IDX_MIN, IDX_MAX now segment-specific
 *
 * Revision 1.23  1998/06/03 13:48:08  dkr
 * added some comments for ComputeOneCube and its helper-routines
 *
 * Revision 1.22  1998/05/29 00:06:38  dkr
 * fixed a bug with unrolling
 *
 * Revision 1.21  1998/05/25 13:15:11  dkr
 * ASSERTs about wrong arguments in wlcomp-pragmas are now ABORT-messages
 *
 * Revision 1.20  1998/05/24 21:15:39  dkr
 * fixed a bug in ComputeIndexMinMax
 *
 * Revision 1.19  1998/05/24 00:41:16  dkr
 * fixed some minor bugs
 * code templates are not used anymore
 *
 * Revision 1.18  1998/05/17 02:06:33  dkr
 * added assertion in IntersectOutline()
 *
 * Revision 1.17  1998/05/17 00:11:20  dkr
 * WLGRID_CEXPR_TEMPLATE is now WLGRID_CODE_TEMPLATE
 *
 * Revision 1.16  1998/05/16 19:51:36  dkr
 * fixed minor bugs in ComputeOneCube
 * (0->1) grids are always N_WLgrid-nodes now!
 *
 * Revision 1.15  1998/05/15 23:45:20  dkr
 * removed a unused var and a unused param in NormalizeWL()
 *
 * Revision 1.14  1998/05/15 23:02:31  dkr
 * fixed some minor bugs
 *
 * Revision 1.13  1998/05/15 16:05:06  dkr
 * fixed a bug in ComputeOneCube, ...
 *
 * Revision 1.12  1998/05/15 15:10:56  dkr
 * changed ComputeOneCube (finished :)
 *
 * Revision 1.11  1998/05/14 21:35:45  dkr
 * changed ComputeOneCube (not finished ...)
 *
 * Revision 1.10  1998/05/12 22:42:54  dkr
 * added attributes NWITH2_DIM, NWITH2_IDX_MIN, NWITH2_IDX_MAX
 * added ComputeIndexMinMax()
 *
 * Revision 1.9  1998/05/12 15:00:48  dkr
 * renamed ..._RC_IDS to ..._DEC_RC_IDS
 *
 * Revision 1.8  1998/05/08 15:45:29  dkr
 * fixed a bug in cube-generation:
 *   pathologic grids are eleminated now :)
 *
 * Revision 1.7  1998/05/08 00:46:09  dkr
 * added some attributes to N_Nwith/N_Nwith2
 *
 * Revision 1.6  1998/05/07 10:14:36  dkr
 * inference of NWITH2_IN/INOUT/OUT/LOCAL moved to refcount
 *
 * Revision 1.5  1998/05/06 14:38:16  dkr
 * inference of NWITH_IN/INOUT/OUT/LOCAL moved to refcount
 *
 * Revision 1.4  1998/05/02 17:47:00  dkr
 * added new attributes to N_Nwith2
 *
 * Revision 1.3  1998/04/29 20:09:25  dkr
 * added a comment
 *
 * Revision 1.2  1998/04/29 17:26:36  dkr
 * added a comment
 *
 * Revision 1.1  1998/04/29 17:17:15  dkr
 * Initial revision
 *
 *
 */

/*****************************************************************************


Transformation N_Nwith -> N_Nwith2:
===================================


Beispiel:
---------

  [  0,  0] -> [ 50,150] step [1,1] width [1,1]: e1
  [  0,150] -> [300,400] step [9,1] width [2,1]: e2
  [  2,150] -> [300,400] step [9,1] width [7,1]: e1
  [ 50,  0] -> [300,150] step [1,1] width [1,1]: e2
  [300,  0] -> [400,100] step [1,1] width [1,1]: e3
  [300,100] -> [400,400] step [1,3] width [1,1]: e3
  [300,101] -> [400,400] step [1,3] width [1,2]: e4


1.) Quader-Building (Berechnung der Quadermenge)
-------------------

    -> Quader-Menge

  Im Beispiel:

      0-> 50, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e1
      0->300, step[0] 9
                   0->2: 150->400, step[1] 1
                                        0->1: e2
                   2->9: 150->400, step[1] 1
                                        0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e2
    300->400, step[0] 1
                   0->1:   0->100, step[1] 1
                                        0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, step[1] 3
                                        0->1: e3
                                        1->3: e4


2a.) Wahl der Segmente mit bv0, bv1, ... (blocking-Vektoren),
     -----------------     ubv (unrolling-blocking-Vektor)

     sv sei der globale step-Vektor eines Segmentes S --- d.h. sv ist das kgV
     aller steps der Quader, die sich mit S nicht-leer schneiden.
     Dann m"ussen in jeder Dimension (d) ubv_d, bv0_d, bv1_d, ...
     Vielfache von sv_d sein.
     Abweichend davon, ist f"ur die ersten Komponenten von bv0, bv1, ... und
     ubv auch der Wert 1 erlaubt: es wird dann in diesen Dimensionen kein
     Blocking durchgef"uhrt.

     Falls bv = (1, ..., 1, ?, gt, ..., gt) gilt --- gt bedeute, da"s bv_d
     hier mindestens so gro"s wie die Segmentbreite ist ---, ist dies
     gleichbedeutend mit bv = (1, ..., 1), soweit ubv dies zul"a"st.
     Diese Vereinfachung wird jedoch *nicht* vorgenommen!

     -> Menge von Segmenten (Segment := ein Quader ohne Raster)

  Im Beispiel: Wir w"ahlen als Segment den gesamten shape
               und bv0 = (180,156), ubv = (1,6) --- beachte: sv = (9,3)


2b.) Anpassen der Quader auf die Segmente
     ------------------------------------

     -> jedes Segment zerf"allt in eine Menge von Quadern

  Im Beispiel: Wir erhalten f"ur das gew"ahlte Segment genau die
               Quadermenge aus 1.)


F"ur jedes Segment sind nun folgende Schritte durchzuf"uhren:


3.) Quader-Splitting (Schneiden der Projektionen)
    ----------------

    Zuerst wird das splitting auf allen Quadern in der 0-ten Dimension
    durchgef"uhrt:
    Die Quader werden in der 0-ten Dimension so lange zerteilt, bis die
    Projektionen ihrer Grenzen in der 0-ten Dimension paarweise disjunkt
    oder identisch sind.
    Die Quader lassen anschlie"send zu Gruppen mit jeweils identischen
    0-Projektionen zusammenfassen.
    Auf jeder dieser Gruppen wird dann das splitting in der 1-ten
    Dimension durchgef"uhrt ... usw.

    Das splitting ist Vorbereitung f"ur das merging (siehe 6.Schritt). Es
    erscheint auf den ersten Blick sinnvoll, splitting und merging
    dimensionsweise verschr"ankt durchzuf"uhren: Die nach dem splitting in
    einer Dimension gebildeten Gruppen stellen ja genau die Projektionen dar,
    welche sp"ater beim merging vereinigt werden.
    Allerdings ist es einfacher und "ubersichtlicher, die beiden Phasen strikt
    zu trennen: Das merging kann auf jeden Fall erst nach dem blocking
    durchgef"uhrt werden (Begr"undung: siehe dort), das splitting findet jedoch
    am besten vor dem blocking statt, da sich durch das splitting i. a. die
    Raster verschieben (siehe Beispiel) --- sich also der Inhalt eines Blockes
    noch "andern w"urde!

  Im Beispiel:

      0-> 50, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, step[1] 1
                                        0->1: e2
                   2->9: 150->400, step[1] 1
                                        0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, step[1] 1
                                        0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, step[1] 1
                                        0->1: e1
                   4->6: 150->400, step[1] 1
                                        0->1: e2
                   6->9: 150->400, step[1] 1
                                        0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, step[1] 1
                                        0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, step[1] 3
                                        0->1: e3
                                        1->3: e4


4.) Blocking (ohne sp"ateres Fitting) entsprechend den Werten aus bv
    --------

  Das blocking wird so fr"uh durchgef"uhrt, da sich durch das blocking die
  Ausf"uhrungsreihenfolge innerhalb des Arrays ver"andert und deshalb alle
  "ubrigen Stufen --- insbesondere merging und optimize --- nur mit genauer
  Kenntnis des blockings arbeiten k"onnen.
  Zum Beispiel: Ohne blocking bekommt beim merging der e1-Quader vom
                e2/e3-Quader in der 0-ten Dimension einen step von 9
                aufgezwungen, und anschlie"send k"onnen in der Optimierung
                die (0->2)-Zweige beider Teile zusammengef"ugt werden.
                Mit blocking wird beides nicht durchgef"uhrt, da diese Teile
                dann in verschiedenen Bl"ocken liegen!
                Vergleiche auch mit den Beispielen weiter unten...

  Das blocking wird im Baum a"hnlich wie die Quader dargestellt --- es wird
  eine analog aufgebaute Blockhierarchie vorgeschaltet.

  Im Beispiel (Grenzen x10 f"ur realistische Verh"altnisse):

    An den Quadergrenzen m"ussen immer neue Bl"ocke begonnen werden, also
    brauchen wir f"ur jeden Quader eigene blocking-Schleifen:

    0000->0500, block[0] 180:      // dies ist das Koordinatensystem f"ur ...
        0000->1500, block[1] 156:  // ... die Bl"ocke im ersten Quader.
               op1               // hier mu"s noch definiert werden, was ...
                                 // ... innerhalb eines Blockes passieren soll
    0000->0500, block[0] 180:
        1500->4000, block[1] 156:
               op2

    0500->3000, block[0] 180:
        0000->1500, block[1] 156:
               op3

    0500->3000, block[0] 180:
        1500->4000, block[1] 156:
               op4

    3000->4000, block[0] 180:
        0000->1000, block[1] 156:
               op5

    3000->4000, block[0] 180:
        1000->4000, block[1] 156:
               op6

    An den Bl"attern dieses blocking-Baumes k"onnen wir jetzt die Beschreibungen
    f"ur die Blockinhalte einf"ugen:

    0000->0500, block[0] 180:
        0000->1500, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 1
                                               0->1: e1

    0000->0500, block[0] 180:
        1500->4000, block[1] 156:
               0->180, step[0] 9
                            0->2: 0->156, step[1] 1
                                               0->1: e2
                            2->9: 0->156, step[1] 1
                                               0->1: e1

    0500->3000, block[0] 180:
        0000->1500, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 1
                                               0->1: e2

    0500->3000, block[0] 180:
        1500->4000, block[1] 156:
               0->180, step[0] 9
                            0->4: 0->156, step[1] 1
                                               0->1: e1
                            4->6: 0->156, step[1] 1
                                               0->1: e2
                            6->9: 0->156, step[1] 1
                                               0->1: e1

    3000->4000, block[0] 180:
        0000->1000, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 1
                                               0->1: e3

    3000->4000, block[0] 180:
        1000->4000, block[1] 156:
               0->180, step[0] 1
                            0->1: 0->156, step[1] 3
                                               0->1: e3
                                               1->3: e4

  In dieses Schema lassen sich analog beliebig viele Stufen f"ur hierarchisches
  blocking einziehen.

  Soll in allen oder einigen Dimensionen keine blocking stattfinden, l"a"st
  sich dies --- zumindest im Falle (sv_d > 1) --- wegen der eventuell
  vorhandenen Raster nicht wie oben und einem blocking-step von 1 erreichen.
  Dann w"urde in den entsprechenden Zweigen des Baumes ein komplettes Raster
  n"amlich keinen Platz mehr finden.
  Ein blocking von 1 erreicht man vielmehr dadurch, da"s f"ur die entsprechende
  Dimension kein 'block[d]'-Zweig gebildet wird, sondern sofort der 'step[d]'-
  Zweig im Baum erscheint. Dem kann sich dann die blocking-Hierarchie f"ur die
  nachfolgenden Dimensionen anschlie"sen.
  Also etwa im Beispiel mit bv = (1,156):

    0000->0500, step[0] 1
                     0->1:    0->1500, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
    0000->0500, step[0] 9
                     0->2: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e2
                     2->9: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
    0500->3000, step[0] 1
                     0->1:    0->1500, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e2
    0500->3000, step[0] 9
                     0->4: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
                     4->6: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e2
                     6->9: 1500->4000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e1
    3000->4000, step[0] 1
                     0->1:    0->1000, block[1] 156:
                                             0->156, step[1] 1
                                                          0->1: e3
    3000->4000, step[0] 1
                     0->1: 1000->4000, block[1] 156:
                                             0->156, step[1] 3
                                                          0->1: e3
                                                          1->3: e4

  Nach dieser Systematik l"a"st sich blocking (bv_d > 1) und non-blocking
  (bv_d = 1) bei Bedarf f"ur alle Dimensionen und Level (hierarchisches
  blocking) mischen.
  Auf Grund der gew"ahlten Baumdarstellung lassen sich die nachfolgenden
  Transformationen (Schritt 6 bis 9) in jedem Fall auf "ubersichtliche und
  einfach zu definierende Art und Weise durchf"uhren:
  Es m"ussen nur Indexgrenzen von Knoten und deren Unterb"aume verglichen
  werden; u. U. werden Unterb"aume kopiert, gel"oscht oder verschoben --- aber
  all dies geschieht lokal, unabh"angig von der genauen Position im Baum,
  und funktioniert f"ur jeden Knotentyp (step, block, ...).
  Auch f"ur die Code-Erzeugung im letzten Schritt braucht nur der Typ eines
  Knotens mit dem Wert seiner Attribute bekannt zu sein, nicht seine Position
  im Baum.

  Mit den Originalzahlen (f"ur etwas pathologische Verh"altnisse) ergibt sich
  f"ur das Beispiel nach dem blocking mit bv = (180,156):

    000->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 1
                                              0->1: e1
    000->050, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, step[1] 1
                                              0->1: e2
                           2->9: 0->156, step[1] 1
                                              0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 1
                                              0->1: e2
    050->300, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, step[1] 1
                                              0->1: e1
                           4->6: 0->156, step[1] 1
                                              0->1: e2
                           6->9: 0->156, step[1] 1
                                              0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 1
                                              0->1: e3
    300->400, block[0] 180:
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, step[1] 3
                                              0->1: e3
                                              1->3: e4

  Anm.: statt sofort
          0->50, block[0] 50: ...
        zu schreiben, bleiben erst einmal die Originalwerte f"ur das blocking
        stehen, da diese f"ur die Baum-Optimierung noch gebraucht werden.
        Die angepa"sten blocking-Gr"o"sen werden auch erst f"ur das fitting
        ben"otigt (siehe 9.Schritt)!

  Mit bv = (1,156) --- wegen ubv = (1,6) darf dies nicht in (1,1) konvertiert
  werden --- w"urde sich ergeben:

      0-> 50, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e2
                   2->9: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
                   4->6: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e2
                   6->9: 150->400, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, step[1] 1
                                                      0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, block[1] 156:
                                         0->156, step[1] 3
                                                      0->1: e3
                                                      1->3: e4


5.) Unrolling-Blocking (mit sp"aterem Fitting) entsprechend den Werten aus ubv
    ------------------

    Es wird auf jedem Block f"ur jede Dimension mit (ubv_d > 1) ein weiteres
    Blocking durchgef"uhrt.
    Dieses unterscheidet sich jedoch von einem konventionellen hierarchischen
    Blocking darin, da"s u. U. noch ein Fitting durchgef"uhrt wird --- siehe
    8. Schritt)

  Im Beispiel mit bv = (180,156):

    000->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    000->050, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
    050->300, block[0] 180:
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
    300->400, block[0] 180:
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  Im Beispiel mit bv = (1,156):

      0-> 50, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
      0-> 50, step[0] 9
                   0->2: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 1
                   0->1:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
     50->300, step[0] 9
                   0->4: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9: 150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
    300->400, step[0] 1
                   0->1: 100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


6.) Quader-Merging (Quader mit identischen Unterb"aumen kompatibel machen
    --------------  und zusammenfassen)

    -> Der Baum bildet in jeder Dimension eine Partition der betreffenden
       Indexmengen-Projektion

  Im Beispiel mit bv = (180,156):

    000->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->400, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    300->400, block[0] 180:
        000->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  Im Beispiel mit bv = (1,156):

      0-> 50, step[0] 9
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 9
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


7.) Baum-Optimierung (identische Unterb"aume werden zusammengefa"st)
    ----------------

    Projektionen mit aufeinanderfolgenden Indexranges und identischen
    Operationen (Unterb"aumen) werden zusammengefa"st.

  Im Beispiel mit bv = (1,156):

      0-> 50, step[0] 9
               ...
                   2->9:   0->150, block[1] 156: ... tree_1 ...

                         150->400, block[1] 156: ... tree_1 ...
                        zum Gl"uck stehen da ^ noch gleiche Werte!

    wird zu

      0-> 50, step[0] 9
               ...
                   2->9:   0->400, block[1] 156: ... tree_1 ...
                          jetzt macht dieser ^ Wert wieder Sinn!

  Insgesamt:

      0-> 50, step[0] 9
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->300, step[0] 9
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


8.) Projektions-Fitting (nicht-komplette Perioden am Ende abtrennen)
    -------------------
    (^ nach der Optimierung liegen i. a. keine Quader mehr vor ...)

    Die Grenzen des "au"sersten Knotens jeder Dimension werden auf die Anzahl
    der abzurollenden Elemente --- also max(ubv_d, step) --- angepa"st.

  Im Beispiel mit bv = (180,156):

    000->045, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    045->050, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0->180, step[0] 5
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->5: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 5
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->5: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    050->293, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    293->300, block[0] 180:
        000->150, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->180, step[0] 7
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->7: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1] 156:
              0->180, step[0] 7
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->7: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
    300->400, block[0] 180:
        000-> 96, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        096->100, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        100->400, block[1] 156:
              0->180, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  Im Beispiel mit bv = (1,156):

      0-> 45, step[0] 9
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->9:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     45-> 50, step[0] 5
                   0->2:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   2->5:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
     50->293, step[0] 9
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    293->300, step[0] 7
                   0->4:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                   6->7:   0->150, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0-> 96, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                          96->100, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4


9.) Blockgr"o"sen an Projektionsgrenzen anpassen
    --------------------------------------------

  Im Beispiel mit bv = (180,156):

    000->045, block[0]  45:
        000->150, block[1] 150:
              0-> 45, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0-> 45, step[0] 9
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0-> 45, step[0] 9
                           0->2: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           2->9: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    045->050, block[0]   5:
        000->150, block[1] 150:
              0->  5, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        150->396, block[1] 156:
              0->  5, step[0] 5
                           0->2: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           2->5: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0->  5, step[0] 5
                           0->2: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           2->5: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    050->293, block[0] 180:
        000->150, block[1] 150:
              0->180, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->180, step[0] 9
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->9: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0->180, step[0] 9
                           0->4: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
                           4->6: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           6->9: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    293->300, block[0]   7:
        000->150, block[1] 150:
              0->  7, step[0] 1
                           0->1: 0->150, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
        150->396, block[1] 156:
              0->  7, step[0] 7
                           0->4: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
                           4->6: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e2
                           6->7: 0->156, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e1
        396->400, block[1]   4:
              0->  7, step[0] 7
                           0->4: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
                           4->6: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e2
                           6->7: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e1
    300->400, block[0] 100:
        000-> 96, block[1]  96:
              0->100, step[0] 1
                           0->1: 0-> 96, ublock[1] 6:
                                                0->6, step[1] 1
                                                           0->1: e3
        096->100, block[1]   4:
              0->100, step[0] 1
                           0->1: 0->  4, ublock[1] 4:
                                                0->4, step[1] 1
                                                           0->1: e3
        100->400, block[1] 156:
              0->100, step[0] 1
                           0->1: 0->156, ublock[1] 6:
                                                0->6, step[1] 3
                                                           0->1: e3
                                                           1->3: e4

  Im Beispiel mit bv = (1,156):

      0-> 45, step[0] 9
                   0->2:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   2->9:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
     45-> 50, step[0] 5
                   0->2:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   2->5:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
     50->293, step[0] 9
                   0->4:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   6->9:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
    293->300, step[0] 7
                   0->4:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
                   4->6:   0->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e2
                   6->7:   0->150, block[1] 150:
                                         0->150, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e2
                         150->396, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e1
                         396->400, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e1
    300->400, step[0] 1
                   0->1:   0-> 96, block[1]  96:
                                         0-> 96, ublock[1] 6:
                                                        0->6, step[1] 1
                                                                   0->1: e3
                          96->100, block[1]   4:
                                         0->  4, ublock[1] 4:
                                                        0->4, step[1] 1
                                                                   0->1: e3
                         100->400, block[1] 156:
                                         0->156, ublock[1] 6:
                                                        0->6, step[1] 3
                                                                   0->1: e3
                                                                   1->3: e4



interne Darstellung im Syntaxbaum:
==================================


  Die mei"sten Knoten verf"ugen "uber ein Attribut 'level', welches angibt,
  wieviele Vorfahren im Baum ebenfalls die Dimension 'dim' betreffen.


  Knotentyp:   Attribut  (Typ des Attributes)
  -------------------------------------------

    WLseg:     contents  (WLblock, WLublock, WLstride)
               next      (WLseg)


    WLblock:   level     (int)
               dim       (int)
               bound1    (int)
               bound2    (int)
               step      (int)                // blocking-factor
               nextdim   (WLblock)            // blocking-node of next dim
               contents  (WLublock, WLstride) // op. of interior of block
               next      (WLblock)            // next blocking this dim


    WLublock:  level     (int)
               dim       (int)
               bound1    (int)
               bound2    (int)
               step      (int)
               nextdim   (WLublock)           // only one of this ...
               contents  (WLstride)           // ... nodes is != NULL
               next      (WLublock)


    WLstride:  level     (int)
               dim       (int)
               bound1    (int)
               bound2    (int)
               step      (int)
               unrolling (bool)      // is unrolling wanted? ...
                                     // ... e. g. because of a WLublock-node
               contents  (WLgrid)    // description of the inner grids
               next      (WLstride)  // next stride this dim


    WLgrid:    dim       (int)
               bound1    (int)                         // == offset
               bound2    (int)                         // == offset + width
               unrolling (bool)
               nextdim   (WLblock, WLublock, WLstride) // only one of this ..
               code      (WLcode)                      // .. nodes is != NULL
               next      (WLgrid)


    WLcode:    cblock    (block)
               cexpr     ("expr")


*****************************************************************************/

#include "tree.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "optimize.h"
#include "wlpragma_funs.h"

#include "DupTree.h"
#include "dbug.h"

/*
 * here we store the lineno of the current with-loop
 *  (for creating error-messages ...)
 */
static int line;

/*
 * these macros are used in 'Parts2Strides' to manage
 *   non-constant generator params
 */

#define TO_FIRST_COMPONENT(node)                                                         \
    if (NODE_TYPE (node) == N_array) {                                                   \
        node = ARRAY_AELEMS (node);                                                      \
    }

#define GET_CURRENT_COMPONENT(node, comp)                                                \
    if (node != NULL) {                                                                  \
        switch (NODE_TYPE (node)) {                                                      \
        case N_id:                                                                       \
            /* here is no break missing!! */                                             \
        case N_num:                                                                      \
            comp = DupNode (node);                                                       \
            break;                                                                       \
        case N_exprs:                                                                    \
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (node)) == N_num),                       \
                         "wrong node type found");                                       \
            comp = MakeNum (NUM_VAL (EXPRS_EXPR (node)));                                \
            node = EXPRS_NEXT (node);                                                    \
            break;                                                                       \
        default:                                                                         \
            DBUG_ASSERT ((0), "wrong node type found");                                  \
        }                                                                                \
    } else {                                                                             \
        comp = MakeNum (1);                                                              \
    }

/*
 * these macros are used in 'CompareWlnode' for compare purpose
 */

#define COMP_BEGIN(a, b, result, inc)                                                    \
    if (a > b) {                                                                         \
        result = inc;                                                                    \
    } else {                                                                             \
        if (a < b) {                                                                     \
            result = -inc;                                                               \
        } else {

#define COMP_END                                                                         \
    }                                                                                    \
    }

/******************************************************************************
 *
 * function:
 *   int CompareWLnode( node *node1, node *node2, int outline)
 *
 * description:
 *   compares the N_WL...-nodes 'node1' and 'node2' IN ALL DIMS.
 *   possibly present next nodes in 'node1' or 'node2' are ignored.
 *
 *   if (outline > 0) ALL GRID DATA IS IGNORED!!!
 *   (this feature is used by 'ComputeCubes', to determine whether two strides
 *    lie in the same cube or not)
 *
 *   this function definies the sort order for InsertWLnodes.
 *
 *   return: -2 => outline('node1') < outline('node2')
 *           -1 => outline('node1') = outline('node2'), 'node1' < 'node2'
 *            0 => 'node1' = 'node2'
 *            1 => outline('node1') = outline('node2'), 'node1' > 'node2'
 *            2 => outline('node1') > outline('node2')
 *
 ******************************************************************************/

int
CompareWLnode (node *node1, node *node2, int outline)
{
    node *grid1, *grid2;
    int result, grid_result;

    DBUG_ENTER ("CompareWLnode");

    if ((node1 != NULL) && (node2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (node1) == NODE_TYPE (node2)),
                     "can not compare objects of different type");

        /* compare the bounds first */
        COMP_BEGIN (WLNODE_BOUND1 (node1), WLNODE_BOUND1 (node2), result, 2)
        COMP_BEGIN (WLNODE_BOUND2 (node1), WLNODE_BOUND2 (node2), result, 2)

        switch (NODE_TYPE (node1)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:

            /* compare next dim */
            result
              = CompareWLnode (WLNODE_NEXTDIM (node1), WLNODE_NEXTDIM (node2), outline);
            break;

        case N_WLstride:

            grid1 = WLSTRIDE_CONTENTS (node1);
            DBUG_ASSERT ((grid1 != NULL), "no grid found");
            grid2 = WLSTRIDE_CONTENTS (node2);
            DBUG_ASSERT ((grid2 != NULL), "no grid found");

            if (outline) {
                /* compare outlines only -> skip grid */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);
            } else {
                /*
                 * compare grid, but leave 'result' untouched
                 *   until later dimensions are checked!
                 */
                COMP_BEGIN (WLGRID_BOUND1 (grid1), WLGRID_BOUND1 (grid2), grid_result, 1)
                COMP_BEGIN (WLGRID_BOUND2 (grid1), WLGRID_BOUND2 (grid2), grid_result, 1)
                grid_result = 0;
                COMP_END
                COMP_END

                /* compare later dimensions */
                result = CompareWLnode (WLGRID_NEXTDIM (grid1), WLGRID_NEXTDIM (grid2),
                                        outline);

                /*
                 * the 'grid_result' value is important
                 *   only if the outlines are equal
                 */
                if (abs (result) != 2) {
                    result = grid_result;
                }
            }

            break;

        case N_WLgrid:

            result
              = CompareWLnode (WLGRID_NEXTDIM (node1), WLGRID_NEXTDIM (node2), outline);
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        COMP_END
        COMP_END

    } else {

        if ((node1 == NULL) && (node2 == NULL)) {
            result = 0;
        } else {
            result = (node2 == NULL) ? 2 : (-2);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *InsertWLnodes( node *nodes, node *insert_nodes)
 *
 * description:
 *   inserts all elements of the chain 'insert_nodes' into the sorted chain
 *     'nodes'.
 *   all elements of 'insert_nodes' that exist already in 'nodes' are freed.
 *   uses function 'CompareWLnode' to sort the elements.
 *
 *   insert_nodes: (unsorted) chain of N_WL...-nodes
 *   nodes:        sorted chain of N_WL...-nodes
 *   return:       sorted chain of N_WL...-nodes containing all the data of
 *                   'nodes' and 'insert_nodes'
 *
 ******************************************************************************/

node *
InsertWLnodes (node *nodes, node *insert_nodes)
{
    node *tmp, *insert_here;
    int compare;

    DBUG_ENTER ("InsertWLnodes");

    /*
     * insert all elements of 'insert_nodes' in 'nodes'
     */
    while (insert_nodes != NULL) {

        /* compare the first element to insert with the first element in 'nodes' */
        compare = CompareWLnode (insert_nodes, nodes, 0);

        if ((nodes == NULL) || (compare < 0)) {
            /* insert current element of 'insert_nodes' at head of 'nodes' */
            tmp = insert_nodes;
            insert_nodes = WLNODE_NEXT (insert_nodes);
            WLNODE_NEXT (tmp) = nodes;
            nodes = tmp;
        } else {

            /* search for insert-position in 'nodes' */
            insert_here = nodes;
            while ((compare > 0) && (WLNODE_NEXT (insert_here) != NULL)) {
                compare = CompareWLnode (insert_nodes, WLNODE_NEXT (insert_here), 0);

                if (compare > 0) {
                    insert_here = WLNODE_NEXT (insert_here);
                }
            }

            if (compare == 0) {
                /* current element of 'insert_nodes' exists already -> free it */
                insert_nodes = FreeNode (insert_nodes);
            } else {
                /* insert current element of 'insert_nodes' after the found position */
                tmp = insert_nodes;
                insert_nodes = WLNODE_NEXT (insert_nodes);
                WLNODE_NEXT (tmp) = WLNODE_NEXT (insert_here);
                WLNODE_NEXT (insert_here) = tmp;
            }
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeStride_1( node *stride)
 *
 * description:
 *   returns the IN THE FIRST DIMENSION normalized N_WLstride-node 'stride'.
 *   a possibly present next node in 'stride' is ignored.
 *
 *   this normalization has two major goals:
 *     * every stride has a unambiguous form
 *        -> two strides represent the same index set
 *           if and only if there attribute values are equal.
 *     * maximize the outline of strides
 *        -> two strides of the same cube do not split each other in several
 *           parts when intersected
 *
 ******************************************************************************/

node *
NormalizeStride_1 (node *stride)
{
    node *grid;
    int bound1, bound2, step, grid_b1, grid_b2, new_bound1, new_bound2;

    DBUG_ENTER ("NormalizeStride_1");

    grid = WLSTRIDE_CONTENTS (stride);
    DBUG_ASSERT ((grid != NULL), "grid not found");
    DBUG_ASSERT ((WLGRID_NEXT (grid) == NULL), "more than one grid found");

    bound1 = WLSTRIDE_BOUND1 (stride);
    bound2 = WLSTRIDE_BOUND2 (stride);
    step = WLSTRIDE_STEP (stride);
    grid_b1 = WLGRID_BOUND1 (grid);
    grid_b2 = WLGRID_BOUND2 (grid);

    /*
     * assure: ([grid_b1; grid_b2] < [0; step]) or (grid_b2 = step = 1);
     * in other terms: (width < step) or (width = step = 1)
     */

    if (grid_b2 > step) {
        grid_b2 = step;
    }
    if ((step > 1) && (grid_b1 == 0) && (grid_b2 == step)) {
        grid_b2 = step = 1;
    }

    /*
     * if (bound2 - bound1 <= step), we set (step = 1) to avoid pathologic cases!!!
     */

    if (bound2 - bound1 <= step) {
        bound2 = bound1 + grid_b2;
        bound1 += grid_b1;
        grid_b1 = 0;
        grid_b2 = 1;
        step = 1;
    }

    /*
     * maximize the outline
     */

    /* calculate minimum for 'bound1' */
    new_bound1 = bound1 - (step - grid_b2);
    new_bound1 = MAX (0, new_bound1);

    /* calculate maximum for 'bound2' */
    new_bound2 = ((bound2 - bound1 - grid_b1) % step >= grid_b2 - grid_b1)
                   ? (bound2 + step - ((bound2 - bound1 - grid_b1) % step))
                   : (bound2);

    WLSTRIDE_BOUND1 (stride) = new_bound1;
    WLSTRIDE_BOUND2 (stride) = new_bound2;
    WLSTRIDE_STEP (stride) = step;
    WLGRID_BOUND1 (grid) = grid_b1 + (bound1 - new_bound1);
    WLGRID_BOUND2 (grid) = grid_b2 + (bound1 - new_bound1);

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node* Parts2Strides( node *parts, int dims)
 *
 * description:
 *   converts a N_Npart-chain ('parts') into a N_WLstride-chain (return).
 *   'dims' is the number of dimensions.
 *
 ******************************************************************************/

node *
Parts2Strides (node *parts, int dims)
{
    node *parts_stride, *stride, *new_stride, *new_grid, *last_grid, *gen, *bound1,
      *bound2, *step, *width, *curr_bound1, *curr_bound2, *curr_step, *curr_width;
    int dim, curr_step_, curr_width_;

    DBUG_ENTER ("Parts2Strides");

    parts_stride = NULL;

    gen = NPART_GEN (parts);
    bound1 = NGEN_BOUND1 (gen);
    bound2 = NGEN_BOUND2 (gen);
    step = NGEN_STEP (gen);
    width = NGEN_WIDTH (gen);

    /*
     * check, if params of generator are constant
     */
    if ((NODE_TYPE (bound1) == N_array) && (NODE_TYPE (bound2) == N_array)
        && ((step == NULL) || (NODE_TYPE (step) == N_array))
        && ((width == NULL) || (NODE_TYPE (width) == N_array))) {

        /*
         * the generator parameters are constant
         *  -> the params of *all* generators are constant (see WLF)
         */

        while (parts != NULL) {
            stride = NULL;

            gen = NPART_GEN (parts);
            DBUG_ASSERT ((NGEN_OP1 (gen) == F_le), "op1 in generator is not <=");
            DBUG_ASSERT ((NGEN_OP2 (gen) == F_lt), "op2 in generator is not <");

            /* get components of current generator */
            bound1 = ARRAY_AELEMS (NGEN_BOUND1 (gen));
            bound2 = ARRAY_AELEMS (NGEN_BOUND2 (gen));
            step = (NGEN_STEP (gen) != NULL) ? ARRAY_AELEMS (NGEN_STEP (gen)) : NULL;
            width = (NGEN_WIDTH (gen) != NULL) ? ARRAY_AELEMS (NGEN_WIDTH (gen)) : NULL;

            for (dim = 0; dim < dims; dim++) {
                DBUG_ASSERT ((bound1 != NULL), "bound1 not complete");
                DBUG_ASSERT ((bound2 != NULL), "bound2 not complete");

                curr_step_ = (step != NULL) ? NUM_VAL (EXPRS_EXPR (step)) : 1;
                curr_width_ = (width != NULL) ? NUM_VAL (EXPRS_EXPR (width)) : 1;

                /* build N_WLstride-node of current dimension */
                new_stride = MakeWLstride (0, dim, NUM_VAL (EXPRS_EXPR (bound1)),
                                           NUM_VAL (EXPRS_EXPR (bound2)), curr_step_, 0,
                                           MakeWLgrid (0, dim, 0, curr_width_, 0, NULL,
                                                       NULL, NULL),
                                           NULL);

                /* the PART-information is needed by 'IntersectOutline' */
                WLSTRIDE_PART (new_stride) = parts;

                new_stride = NormalizeStride_1 (new_stride);

                /* append 'new_stride' to 'stride' */
                if (dim == 0) {
                    stride = new_stride;
                } else {
                    WLGRID_NEXTDIM (last_grid) = new_stride;
                }
                last_grid = WLSTRIDE_CONTENTS (new_stride);

                /* go to next dim */
                bound1 = EXPRS_NEXT (bound1);
                bound2 = EXPRS_NEXT (bound2);
                if (step != NULL) {
                    step = EXPRS_NEXT (step);
                }
                if (width != NULL) {
                    width = EXPRS_NEXT (width);
                }
            }

            WLGRID_CODE (last_grid) = NPART_CODE (parts);
            NCODE_USED (NPART_CODE (parts))++;
            parts_stride = InsertWLnodes (parts_stride, stride);

            parts = NPART_NEXT (parts);
        }
        while (parts != NULL)
            ;

    } else {

        /*
         * not all generator parameters are constant
         */

        DBUG_ASSERT ((NPART_NEXT (parts) == NULL), "more than one part found");

        TO_FIRST_COMPONENT (bound1)
        TO_FIRST_COMPONENT (bound2)
        if (step != NULL) {
            TO_FIRST_COMPONENT (step)
        }
        if (width != NULL) {
            TO_FIRST_COMPONENT (width)
        }

        for (dim = 0; dim < dims; dim++) {
            /*
             * components of current dim
             */
            GET_CURRENT_COMPONENT (bound1, curr_bound1)
            GET_CURRENT_COMPONENT (bound2, curr_bound2)
            GET_CURRENT_COMPONENT (step, curr_step)
            GET_CURRENT_COMPONENT (width, curr_width)

            if ((NODE_TYPE (curr_width) == N_num) && (NUM_VAL (curr_width) == 1)) {
                /*
                 * If we have found a (0 -> 1) grid, we can build a N_WLgrid- instead of
                 *  a N_WLgridVar-node.
                 *
                 * CAUTION: We must *not* build a N_WLgrid-node for constant grids in
                 * general!! An example:
                 *
                 *            0 -> b step 4
                 *                   0 -> 3: op0
                 *                   3 -> 4: op1
                 *
                 *          For N_WLgrid-nodes we create code, that executes
                 * unconditionally the whole grid. If the grid is not a (0 -> 1) grid,
                 * this can be dangerous: Let b=10, then we must cut off the grids in the
                 * last loop-pass, because the step (4) is not a divisor of the
                 * stride-width (b=10). For constant strides this is done statically in
                 * 'wltransform: fit'. But for grids != (0 -> 1), belonging to a
                 * WLstriVar-node, we must do this at runtime. Therefore these grids
                 * *must* be N_WLgridVar-nodes.
                 */
                new_grid = MakeWLgrid (0, dim, 0, 1, 0, NULL, NULL, NULL);
                curr_width = FreeNode (curr_width);
            } else {
                new_grid = MakeWLgridVar (dim, MakeNum (0), curr_width, NULL, NULL, NULL);
            }

            /* build N_WLstriVar-node of current dimension */
            new_stride
              = MakeWLstriVar (dim, curr_bound1, curr_bound2, curr_step, new_grid, NULL);

            /* append 'new_stride' to 'parts_stride' */
            if (dim == 0) {
                parts_stride = new_stride;
            } else {
                WLGRIDVAR_NEXTDIM (last_grid) = new_stride;
            }
            last_grid = WLSTRIVAR_CONTENTS (new_stride);
        }

        if (NODE_TYPE (last_grid) == N_WLgrid) {
            WLGRID_CODE (last_grid) = NPART_CODE (parts);
        } else {
            WLGRIDVAR_CODE (last_grid) = NPART_CODE (parts);
        }
        NCODE_USED (NPART_CODE (parts))++;
    }

    DBUG_RETURN (parts_stride);
}

/******************************************************************************
 *
 * function:
 *   int IndexHeadStride( node *stride)
 *
 * description:
 *   returns the index position of the first element of 'stride'
 *
 ******************************************************************************/

int
IndexHeadStride (node *stride)
{
    int result;

    DBUG_ENTER ("IndexHeadStride");

    result = WLSTRIDE_BOUND1 (stride) + WLGRID_BOUND1 (WLSTRIDE_CONTENTS (stride));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int IndexRearStride( node *stride)
 *
 * description:
 *   returns the index position '+1' of the last element of 'stride'
 *
 ******************************************************************************/

int
IndexRearStride (node *stride)
{
    node *grid = WLSTRIDE_CONTENTS (stride);
    int bound2 = WLSTRIDE_BOUND2 (stride);
    int grid_b1 = WLGRID_BOUND1 (grid);
    int result;

    DBUG_ENTER ("IndexRearStride");

    /* search last grid (there will we find the last element!) */
    while (WLGRID_NEXT (grid) != NULL) {
        grid = WLGRID_NEXT (grid);
    }

    result = bound2
             - MAX (0, ((bound2 - WLSTRIDE_BOUND1 (stride) - grid_b1 - 1)
                        % WLSTRIDE_STEP (stride))
                         + 1 - (WLGRID_BOUND2 (grid) - grid_b1));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int GridOffset( int new_bound1,
 *                   int bound1, int step, int grid_b2)
 *
 * description:
 *   computes a offset for a grid relating to 'new_bound1':
 *     what happens to the bounds of a grid if 'new_bound1' is the new
 *     upper bound for the accessory stride?
 *     the new bounds of the grid are:
 *         grid_b1 - offset, grid_b2 - offset
 *
 *   CAUTION: if (offset > grid_b1) the grid must be devided in two parts:
 *              "(grid_b1 - offset + step) -> step" and
 *              "0 -> (grid_b2 - offset)" !!
 *
 ******************************************************************************/

int
GridOffset (int new_bound1, int bound1, int step, int grid_b2)
{
    int offset;

    DBUG_ENTER ("GridOffset");

    offset = (new_bound1 - bound1) % step;

    if (offset >= grid_b2) {
        offset -= step;
    }

    DBUG_RETURN (offset);
}

/******************************************************************************
 *
 * function:
 *   int IntersectOutline( node *stride1, node *stride2,
 *                         node **i_stride1, node **i_stride2)
 *
 * description:
 *   returns in 'i_stride1' and 'i_stride2' the part of 'stride1', 'stride2'
 *     respectively that lies in a common cube.
 *   possibly present next nodes in 'stride1' or 'stride2' are ignored.
 *   the return value is 1 if and only if the intersection is non-empty.
 *
 *   if we are interested in the return value only, we can call this
 *     function with ('stride1' == NULL), ('stride2' == NULL).
 *
 ******************************************************************************/

int
IntersectOutline (node *stride1, node *stride2, node **i_stride1, node **i_stride2)
{
    node *grid1, *grid2, *new_i_stride1, *new_i_stride2;
    int bound11, bound21, grid1_b1, grid1_b2, bound12, bound22, grid2_b1, grid2_b2, head1,
      rear1, head2, rear2, i_bound1, i_bound2, i_offset1, i_offset2;
    int flag = 0;
    int result = 1;

    DBUG_ENTER ("IntersectOutline");

    if (i_stride1 != NULL) {
        new_i_stride1 = *i_stride1 = DupNode (stride1);
    }
    if (i_stride2 != NULL) {
        new_i_stride2 = *i_stride2 = DupNode (stride2);
    }

    while (stride1 != NULL) {
        DBUG_ASSERT ((stride2 != NULL), "dim not found");

        DBUG_ASSERT ((WLSTRIDE_PART (stride1) != NULL), "no part found");
        DBUG_ASSERT ((WLSTRIDE_PART (stride2) != NULL), "no part found");

        grid1 = WLSTRIDE_CONTENTS (stride1);
        DBUG_ASSERT ((grid1 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid1) == NULL), "more than one grid found");
        grid2 = WLSTRIDE_CONTENTS (stride2);
        DBUG_ASSERT ((grid2 != NULL), "grid not found");
        DBUG_ASSERT ((WLGRID_NEXT (grid2) == NULL), "more than one grid found");

        bound11 = WLSTRIDE_BOUND1 (stride1);
        bound21 = WLSTRIDE_BOUND2 (stride1);
        grid1_b1 = WLGRID_BOUND1 (grid1);
        grid1_b2 = WLGRID_BOUND2 (grid1);

        bound12 = WLSTRIDE_BOUND1 (stride2);
        bound22 = WLSTRIDE_BOUND2 (stride2);
        grid2_b1 = WLGRID_BOUND1 (grid2);
        grid2_b2 = WLGRID_BOUND2 (grid2);

        head1 = IndexHeadStride (stride1);
        rear1 = IndexRearStride (stride1);
        head2 = IndexHeadStride (stride2);
        rear2 = IndexRearStride (stride2);

        i_bound1 = MAX (bound11, bound12);
        i_bound2 = MIN (bound21, bound22);

        i_offset1 = GridOffset (i_bound1, bound11, WLSTRIDE_STEP (stride1), grid1_b2);
        i_offset2 = GridOffset (i_bound1, bound12, WLSTRIDE_STEP (stride2), grid2_b2);

        if (/* are the outlines of 'stride1' and 'stride2' not disjunkt? */
            (head1 < rear2) && (head2 < rear1) &&

            /* are the grids compatible? */
            (i_offset1 <= grid1_b1) && (i_offset2 <= grid2_b1)
            /*
             * Note: (i_offset_1 < grid1_b1) means, that the grid1 must be split
             *        in two parts to fit the new upper bound in the current dim.
             *       Then the *projections* of grid1, grid2 can not be disjunct,
             *        therefore grid1, grid2 must have disjunct outlines!!!
             */

        ) {

            DBUG_ASSERT ((
                           /* is intersection of 'stride1' with the outline of 'stride2'
                              not empty? */
                           (i_bound1 + grid1_b1 - i_offset1 < i_bound2) &&
                           /* is intersection of 'stride2' with the outline of 'stride1'
                              not empty? */
                           (i_bound1 + grid2_b1 - i_offset2 < i_bound2)
                           /*
                            * example:
                            *
                            *   stride1: 0->5 step 2          stride2: 2->3 step 1
                            *                   1->2: ...                     0->1: ...
                            *
                            * Here we must notice, that the intersection of 'stride1' with
                            * the outline of 'stride2' is empty:
                            *
                            *            2->3 step 2
                            *                   1->2: ...     !!!!!!!!!!!!
                            *
                            * The following parts of the 'cube generation' can not handle
                            * this case! Therefore we will stop here!!
                            *
                            * remark: If this assertion fails, there is a bug in the
                            * 'first step' of 'ComputeCubes()' !!!
                            */
                           ),
                         ("must resign: "
                          "intersection of outline(stride1) and outline(stride2) is "
                          "non-empty, "
                          "while intersection of outline(stride1) and stride2, as well "
                          "as "
                          "intersection of stride1 and outline(stride2) is empty :-("));

            if ((WLSTRIDE_PART (stride1) == WLSTRIDE_PART (stride2)) &&
                /* are 'stride1' and 'stride2' descended from the same Npart? */
                (!flag)) {
                /* we should deal with this exception only once !! */

                /*
                 * example:
                 *
                 *  0->6  step 3, 0->1: op1
                 *  0->16 step 3, 1->3: op2
                 *  4->20 step 3, 2->3: op3
                 * ------------------------- after first round with IntersectOutline:
                 *  0->7  step 3, 1->3: op2  <- intersection of 'op2' and outline('op1')
                 *  3->16 step 3, 1->3: op2  <- intersection of 'op2' and outline('op3')
                 *
                 *  these two strides are **not** disjunkt!!!
                 *  but they are part of the same Npart!!
                 */

                flag = 1; /* skip this exception handling in later dimensions! */

                /*
                 * modify the bounds of the first stride,
                 * so that the new outlines are disjunct
                 */
                if (WLSTRIDE_BOUND2 (stride1) < WLSTRIDE_BOUND2 (stride2)) {
                    if (i_stride1 != NULL) {
                        WLSTRIDE_BOUND2 (new_i_stride1) = i_bound1;
                        new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                    }
                } else {
                    if (i_stride2 != NULL) {
                        WLSTRIDE_BOUND2 (new_i_stride2) = i_bound1;
                        new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                    }
                }

            } else {

                if (i_stride1 != NULL) {
                    /* intersect 'stride1' with the outline of 'stride2' */
                    WLSTRIDE_BOUND1 (new_i_stride1) = i_bound1;
                    WLSTRIDE_BOUND2 (new_i_stride1) = i_bound2;
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride1))
                      = grid1_b1 - i_offset1;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride1))
                      = grid1_b2 - i_offset1;
                    new_i_stride1 = NormalizeStride_1 (new_i_stride1);
                }

                if (i_stride2 != NULL) {
                    /* intersect 'stride2' with the outline of 'stride1' */
                    WLSTRIDE_BOUND1 (new_i_stride2) = i_bound1;
                    WLSTRIDE_BOUND2 (new_i_stride2) = i_bound2;
                    WLGRID_BOUND1 (WLSTRIDE_CONTENTS (new_i_stride2))
                      = grid2_b1 - i_offset2;
                    WLGRID_BOUND2 (WLSTRIDE_CONTENTS (new_i_stride2))
                      = grid2_b2 - i_offset2;
                    new_i_stride2 = NormalizeStride_1 (new_i_stride2);
                }
            }

        } else {
            /*
             * intersection is empty
             *  -> free the useless data in 'i_stride1', 'i_stride2'
             */
            if (i_stride1 != NULL) {
                if (*i_stride1 != NULL) {
                    *i_stride1 = FreeTree (*i_stride1);
                }
            }
            if (i_stride2 != NULL) {
                if (*i_stride2 != NULL) {
                    *i_stride2 = FreeTree (*i_stride2);
                }
            }
            result = 0;

            /* we can give up here */
            break;
        }

        /* next dim */
        stride1 = WLGRID_NEXTDIM (grid1);
        stride2 = WLGRID_NEXTDIM (grid2);
        if (i_stride1 != NULL) {
            new_i_stride1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride1));
        }
        if (i_stride2 != NULL) {
            new_i_stride2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (new_i_stride2));
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SetSegs( node *pragma, node *cubes, int dims)
 *
 * description:
 *   returns chain of segments (based on the calculated cubes 'cubes')
 *
 ******************************************************************************/

node *
SetSegs (node *pragma, node *cubes, int dims)
{
    node *aps;
    node *segs;
    char *fun_names;

    DBUG_ENTER ("SetSegs");

    /*
     * create default configuration
     */
#if 1 /* -> sac2c flag!?! */
    segs = All (NULL, NULL, cubes, dims, line);
#else
    segs = Cubes (NULL, NULL, cubes, dims, line);
#endif

    /*
     * create pragma-dependent configuration
     */
    if (pragma != NULL) {
        aps = PRAGMA_WLCOMP_APS (pragma);
        while (aps != NULL) {

#define WLP(fun, str)                                                                    \
    if (strcmp (AP_NAME (EXPRS_EXPR (aps)), str) == 0) {                                 \
        segs = fun (segs, AP_ARGS (EXPRS_EXPR (aps)), cubes, dims, line);                \
    } else
#include "wlpragma_funs.mac"
#undef WLP
            {
                fun_names =
#define WLP(fun, str) " "##str
#include "wlpragma_funs.mac"
#undef WLP
                  ;

                ABORT (line, ("Illegal function name in wlcomp-pragma found."
                              " Currently supported functions are:"
                              "%s",
                              fun_names));
            }

            aps = EXPRS_NEXT (aps);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   void CheckParams( node *seg)
 *
 * description:
 *   checks whether the parameter of the segment 'seg' are legal.
 *
 ******************************************************************************/

void
CheckParams (node *seg)
{
    int last, first_block, d, j;

    DBUG_ENTER (" CheckParams");

    /* test, whether (bv0 >= bv1 >= bv2 >= ... >= 1), (ubv >= 1) */
    for (d = 0; d < WLSEG_DIMS (seg); d++) {
        j = WLSEG_BLOCKS (seg) - 1;
        if ((WLSEG_BV (seg, j))[d] < 1) {
            ABORT (line, ("Blocking step is smaller than 1."
                          " Please check parameters of functions in wlcomp-pragma"));
        }
        last = (WLSEG_BV (seg, j))[d];
        for (; j >= 0; j--) {
            if ((WLSEG_BV (seg, j))[d] < last) {
                ABORT (line, ("Inner Blocking step is smaller than outer one."
                              " Please check parameters of functions in wlcomp-pragma"));
            }
            last = (WLSEG_BV (seg, j))[d];
        }

        if ((WLSEG_UBV (seg))[d] < 1) {
            ABORT (line, ("Unrolling-blocking step is smaller than 1."
                          " Please check parameters of functions in wlcomp-pragma"));
        }
    }

    /*
     * check bv:
     *
     * checks for all bv (bv0, bv1, bv2, ...):
     *  exists k: (forall (d < k): bv_d = 1) and (forall (d >= k): bv_d >= max(sv_j,
     * ubv_j))
     */
    first_block = 0;
    for (j = 0; j < WLSEG_BLOCKS (seg); j++) {
        /* goto first dim with (bv_d > 1) */
        d = 0;
        while ((d < WLSEG_DIMS (seg)) && ((WLSEG_BV (seg, j))[d] == 1)) {
            d++;
        }

        if (d < WLSEG_DIMS (seg)) {
            first_block = d;
        }
        for (; d < WLSEG_DIMS (seg); d++) {
            if ((WLSEG_BV (seg, j))[d]
                < MAX ((WLSEG_SV (seg))[d], (WLSEG_UBV (seg)[d]))) {
                ABORT (line, ("Blocking step is smaller than stride step, "
                              "unrolling-blocking step respectively."
                              " Please check parameters of functions in wlcomp-pragma"));
            }
        }
    }

    /*
     * check ubv:
     *
     * checks for ubv:
     *  - exists k: (forall (d < k): ubv_d = 1) and (forall (d >= k): sv_d | ubv_d)
     *  - ubv <= bv, for most inner bv with (bv != 1)
     *    (we must prevent e.g. bv = (1,40), ubv = (2,2), sv = (2,2),
     *     but this is allowed: bv = (1,1),  ubv = (2,2), sv = (2,2))
     */

    /* goto first dim with (bv_d > 1) */
    d = 0;
    while ((d < WLSEG_DIMS (seg)) && ((WLSEG_UBV (seg))[d] == 1)) {
        d++;
    }

    if (d < first_block) {
        ABORT (line, ("Unrolling-blocking step is greater than most inner blocking step."
                      " Please check parameters of functions in wlcomp-pragma"));
    }

    for (; d < WLSEG_DIMS (seg); d++) {
        if ((WLSEG_UBV (seg))[d] % (WLSEG_SV (seg))[d] != 0) {
            ABORT (line, ("Stride step is not a divisor of unrolling-blocking step."
                          " Please check parameters of functions in wlcomp-pragma"));
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *NewBoundsStride( node *stride, int dim,
 *                          int new_bound1, int new_bound2)
 *
 * description:
 *   returns modified 'stride':
 *     all strides in dimension "current dimension"+'dim' are new bounds
 *     given ('bound1', 'bound2').
 *
 ******************************************************************************/

node *
NewBoundsStride (node *stride, int dim, int new_bound1, int new_bound2)
{
    node *grids, *new_grids, *tmp, *tmp2;
    int bound1, step, grid_b1, grid_b2, offset;

    DBUG_ENTER ("NewBoundsStride");

    grids = WLSTRIDE_CONTENTS (stride);

    if (dim == 0) {
        /*
         * arrived at the correct dimension
         *  -> set new bounds
         *  -> correct the grids if necessary
         */

        bound1 = WLSTRIDE_BOUND1 (stride);
        if (bound1 != new_bound1) {
            /*
             * correct the grids
             */

            step = WLSTRIDE_STEP (stride);
            new_grids = NULL;
            do {
                grid_b1 = WLGRID_BOUND1 (grids);
                grid_b2 = WLGRID_BOUND2 (grids);

                offset = GridOffset (new_bound1, bound1, step, grid_b2);

                /* extract current grid from chain -> single grid in 'tmp' */
                tmp = grids;
                grids = WLGRID_NEXT (grids);
                WLGRID_NEXT (tmp) = NULL;

                if (offset <= grid_b1) {
                    /*
                     * grid is still in one pice :)
                     */

                    WLGRID_BOUND1 (tmp) = grid_b1 - offset;
                    WLGRID_BOUND2 (tmp) = grid_b2 - offset;

                    /* insert changed grid into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, tmp);
                } else {
                    /*
                     * the grid is split into two parts :(
                     */

                    /* first part: recycle old grid */
                    WLGRID_BOUND1 (tmp) = grid_b1 - offset + step;
                    WLGRID_BOUND2 (tmp) = step;
                    /* second part: duplicate old grid first */
                    tmp2 = DupNode (tmp);
                    WLGRID_BOUND1 (tmp2) = 0;
                    WLGRID_BOUND2 (tmp2) = grid_b2 - offset;
                    /* concate the two grids */
                    WLGRID_NEXT (tmp2) = tmp;

                    /* insert them into 'new_grids' */
                    new_grids = InsertWLnodes (new_grids, tmp2);
                }
            } while (grids != NULL);

            WLSTRIDE_CONTENTS (stride) = new_grids;
            WLSTRIDE_BOUND1 (stride) = new_bound1;
        }

        WLSTRIDE_BOUND2 (stride) = new_bound2;

    } else {
        /*
         * involve all grids of current dimension
         */

        do {
            WLGRID_NEXTDIM (grids)
              = NewBoundsStride (WLGRID_NEXTDIM (grids), dim - 1, new_bound1, new_bound2);
            grids = WLGRID_NEXT (grids);
        } while (grids != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   void SplitStride( node *stride1, node *stride2
 *                     node **s_stride1, node **s_stride2)
 *
 * description:
 *   returns in 's_stride1', 's_stride2' the splitted stride 'stride1',
 *     'stride2' respectively.
 *   returns NULL if there is nothing to split.
 *
 ******************************************************************************/

void
SplitStride (node *stride1, node *stride2, node **s_stride1, node **s_stride2)
{
    node *tmp1, *tmp2;
    int i_bound1, i_bound2, dim;

    DBUG_ENTER ("SplitStride");

    tmp1 = stride1;
    tmp2 = stride2;

    *s_stride1 = *s_stride2 = NULL;

    /*
     * in which dimension is splitting needed?
     *
     * search for the first dim,
     * in which the bounds of 'stride1' and 'stride2' are not equal
     */
    dim = 0;
    while ((tmp1 != NULL) && (tmp2 != NULL)
           && (WLSTRIDE_BOUND1 (tmp1) == WLSTRIDE_BOUND1 (tmp2))
           && (WLSTRIDE_BOUND2 (tmp1) == WLSTRIDE_BOUND2 (tmp2))) {
        /*
         * we can take the first grid only,
         * because the stride-bounds are equal in all grids!!
         */
        tmp1 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (tmp1));
        tmp2 = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (tmp2));
        dim++;
    }

    if ((tmp1 != NULL) && (tmp2 != NULL)) { /* is there anything to split? */
        /* compute bounds of intersection */
        i_bound1 = MAX (WLSTRIDE_BOUND1 (tmp1), WLSTRIDE_BOUND1 (tmp2));
        i_bound2 = MIN (WLSTRIDE_BOUND2 (tmp1), WLSTRIDE_BOUND2 (tmp2));

        if (i_bound1 < i_bound2) { /* is intersection non-empty? */
            *s_stride1 = DupNode (stride1);
            *s_stride2 = DupNode (stride2);

            /*
             * propagate the new bounds in dimension 'dim'
             * over the whole tree of 'stride1', 'stride2' respectively
             */
            *s_stride1 = NewBoundsStride (*s_stride1, dim, i_bound1, i_bound2);
            *s_stride2 = NewBoundsStride (*s_stride2, dim, i_bound1, i_bound2);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *SplitWL( node *strides)
 *
 * description:
 *   returns the splitted stride-tree 'strides'.
 *
 ******************************************************************************/

node *
SplitWL (node *strides)
{
    node *stride1, *stride2, *split_stride1, *split_stride2, *new_strides, *tmp;
    int fixpoint;

    DBUG_ENTER ("SplitWL");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

    /*
     * the outline of each stride is intersected with all the other ones.
     * this is done until no new intersections are generated (fixpoint).
     */
    do {
        fixpoint = 1;       /* initialize 'fixpoint' */
        new_strides = NULL; /* here we collect the new stride-set */

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == 0), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /*
         * split in pairs
         */
        stride1 = strides;
        while (stride1 != NULL) {

            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {

                SplitStride (stride1, stride2, &split_stride1, &split_stride2);
                if (split_stride1 != NULL) {
                    DBUG_ASSERT ((split_stride2 != NULL), "wrong splitting");
                    fixpoint = 0; /* no, not a fixpoint yet :( */
                    WLSTRIDE_MODIFIED (stride1) = WLSTRIDE_MODIFIED (stride2) = 1;
                    new_strides = InsertWLnodes (new_strides, split_stride1);
                    new_strides = InsertWLnodes (new_strides, split_stride2);
                } else {
                    DBUG_ASSERT ((split_stride2 == NULL), "wrong splitting");
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /*
             * was 'stride1' not modified?
             *  -> it is a part of the result
             */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /*
                 * 'stride1' was modified, hence no part of the result.
                 *  -> is no longer needed
                 */
                stride1 = FreeNode (stride1);
                /* 'stride1' points to his successor now! */
            }
        }

        strides = new_strides;
    } while (!fixpoint); /* fixpoint found? */

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *BlockStride( node *stride, long *bv, int unroll)
 *
 * description:
 *   returns 'stride' with corrected bounds, blocking levels and
 *     unrolling-flag.
 *   this function is needed after a blocking.
 *
 ******************************************************************************/

node *
BlockStride (node *stride, long *bv, int unroll)
{
    node *curr_stride, *curr_grid, *grids;

    DBUG_ENTER ("BlockStride");

    if (stride != NULL) {

        DBUG_ASSERT ((NODE_TYPE (stride)), "no N_WLstride node found");

        curr_stride = stride;
        do {

            /* correct blocking level and unrolling-flag */
            WLSTRIDE_LEVEL (curr_stride)++;
            WLSTRIDE_UNROLLING (curr_stride) = unroll;
            grids = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_LEVEL (grids)++;
                WLGRID_UNROLLING (grids) = unroll;
                grids = WLGRID_NEXT (grids);
            } while (grids != NULL);

            /* fit bounds of stride to blocking step */
            WLSTRIDE_BOUND1 (curr_stride) = 0;
            WLSTRIDE_BOUND2 (curr_stride) = bv[WLSTRIDE_DIM (curr_stride)];

            /*
             * involve all grids of current dimension
             */
            curr_grid = WLSTRIDE_CONTENTS (curr_stride);
            do {
                WLGRID_NEXTDIM (curr_grid)
                  = BlockStride (WLGRID_NEXTDIM (curr_grid), bv, unroll);
                curr_grid = WLGRID_NEXT (curr_grid);
            } while (curr_grid != NULL);

            curr_stride = WLSTRIDE_NEXT (curr_stride);
        } while (curr_stride != NULL);
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node *BlockWL( node *stride, int dims, long *bv, int unroll)
 *
 * description:
 *   returns with blocking-vector 'bv' blocked 'stride'.
 *   'dims' is the number of dimensions in 'stride'.
 *
 *   when called multiple times in a row, this function even realizes
 *     hierarchical blocking!! (top-down: coarse blocking first!)
 *
 *   ('unroll' > 0) means unrolling-blocking --- allowed only once after all
 *     convential blocking!
 *
 ******************************************************************************/

node *
BlockWL (node *stride, int dims, long *bv, int unroll)
{
    node *curr_block, *curr_dim, *curr_stride, *curr_grid, *contents, *lastdim,
      *last_block, *block;
    int level, d;

    DBUG_ENTER ("BlockWL");

    if (stride != NULL) {

        switch (NODE_TYPE (stride)) {

        case N_WLblock:
            /*
             * block found -> hierarchical blocking
             */

            curr_block = stride;
            while (curr_block != NULL) {

                /* go to contents of block -> skip all found block nodes */
                curr_dim = curr_block;
                DBUG_ASSERT ((NODE_TYPE (curr_dim) == N_WLblock), "no block found");
                while (WLBLOCK_NEXTDIM (curr_dim) != NULL) {
                    curr_dim = WLBLOCK_NEXTDIM (curr_dim);
                    DBUG_ASSERT ((NODE_TYPE (curr_dim) == N_WLblock), "no block found");
                }

                /* block contents of found block */
                WLBLOCK_CONTENTS (curr_dim)
                  = BlockWL (WLBLOCK_CONTENTS (curr_dim), dims, bv, unroll);

                curr_block = WLBLOCK_NEXT (curr_block);
            }
            break;

        case N_WLublock:
            /*
             * ublock found ?!?! -> error
             */

            /*
             * unrolling-blocking is allowed only once
             * after all conventional blocking!!
             */
            DBUG_ASSERT ((0), "data of unrolling-blocking found while blocking");
            break;

        case N_WLstride:
            /*
             * unblocked stride found
             */

            level = WLSTRIDE_LEVEL (stride);

            last_block = NULL;
            curr_stride = stride;
            while (curr_stride != NULL) {

                DBUG_ASSERT ((bv[WLSTRIDE_DIM (curr_stride)] >= 1),
                             "wrong bv-value found");
                if (bv[WLSTRIDE_DIM (curr_stride)] == 1) {
                    /*
                     * no blocking -> go to next dim
                     */
                    curr_grid = WLSTRIDE_CONTENTS (curr_stride);
                    do {
                        WLGRID_NEXTDIM (curr_grid)
                          = BlockWL (WLGRID_NEXTDIM (curr_grid), dims, bv, unroll);

                        curr_grid = WLGRID_NEXT (curr_grid);
                    } while (curr_grid != NULL);

                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                } else {
                    /*
                     * blocking -> create a N_WLblock (N_WLublock respectively) node
                     *   for each following dim
                     */
                    contents = curr_stride;
                    lastdim = NULL;
                    for (d = WLSTRIDE_DIM (curr_stride); d < dims; d++) {
                        DBUG_ASSERT ((NODE_TYPE (contents) == N_WLstride),
                                     "no stride found");
                        DBUG_ASSERT (((d == WLSTRIDE_DIM (curr_stride))
                                      || (WLSTRIDE_NEXT (contents) == NULL)),
                                     "more than one stride found");

                        block
                          = MakeWLblock (level, WLSTRIDE_DIM (contents),
                                         WLSTRIDE_BOUND1 (contents),
                                         WLSTRIDE_BOUND2 (contents),
                                         bv[WLSTRIDE_DIM (contents)], NULL, NULL, NULL);

                        /*
                         * unrolling-blocking wanted?
                         */
                        if (unroll > 0) {
                            NODE_TYPE (block) = N_WLublock;
                        }

                        if (lastdim != NULL) {
                            /*
                             * not first blocking dim
                             *  -> append at block node of last dim
                             */
                            WLBLOCK_NEXTDIM (lastdim) = block;
                        } else {
                            /*
                             * current dim is first blocking dim
                             */
                            if (last_block != NULL) {
                                /* append to last block */
                                WLBLOCK_NEXT (last_block) = block;
                            } else {
                                /* this is the first block */
                                stride = block;
                            }
                            last_block = block;
                        }
                        lastdim = block;

                        DBUG_ASSERT ((WLSTRIDE_CONTENTS (contents) != NULL),
                                     "no grid found");
                        contents = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (contents));
                    }

                    /*
                     * now the block nodes are complete
                     *  -> append contents of block
                     */
                    DBUG_ASSERT ((lastdim != NULL), "block node of last dim not found");
                    WLBLOCK_CONTENTS (lastdim) = curr_stride;
                    curr_stride = WLSTRIDE_NEXT (curr_stride);
                    /* successor is in next block -> no 'next' anymore! */
                    WLSTRIDE_NEXT (WLBLOCK_CONTENTS (lastdim)) = NULL;
                    /* correct the bounds and blocking level in contents of block */
                    WLBLOCK_CONTENTS (lastdim)
                      = BlockStride (WLBLOCK_CONTENTS (lastdim), bv, unroll);
                }
            }
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }
    }

    DBUG_RETURN (stride);
}

/******************************************************************************
 *
 * function:
 *   node *NewStepGrids( node *grids, int step, int new_step, int offset)
 *
 * description:
 *   returns a modified 'grids' chain:
 *     - the bounds of the grids are modified (relating to 'offset')
 *     - the step of the grids is now 'step'
 *        -> possibly the grids must be duplicated
 *
 ******************************************************************************/

node *
NewStepGrids (node *grids, int step, int new_step, int offset)
{
    node *last_old, *last, *new_grid, *tmp;
    int i, div;

    DBUG_ENTER ("NewStepGrids");

    DBUG_ASSERT ((new_step % step == 0), "wrong new step");

    if (step == 1) {
        DBUG_ASSERT ((WLGRID_BOUND1 (grids) == 0), "grid has wrong lower bound");
        DBUG_ASSERT ((WLGRID_NEXT (grids) == NULL), "grid has wrong bounds");
        WLGRID_BOUND2 (grids) = new_step;
    } else {
        div = new_step / step;

        /*
         * adjust bounds (relating to 'offset')
         *
         * search for last grid -> save it in 'last_old'
         */
        WLGRID_BOUND1 (grids) -= offset;
        WLGRID_BOUND2 (grids) -= offset;
        last_old = grids;
        while (WLGRID_NEXT (last_old) != NULL) {
            last_old = WLGRID_NEXT (last_old);
            WLGRID_BOUND1 (last_old) -= offset;
            WLGRID_BOUND2 (last_old) -= offset;
        }

        if (div > 1) {
            /*
             * duplicate all grids ('div' -1) times
             */
            last = last_old;
            for (i = 1; i < div; i++) {
                tmp = grids;
                do {
                    /* duplicate current grid */
                    new_grid = DupNode (tmp);
                    WLGRID_BOUND1 (new_grid) = WLGRID_BOUND1 (new_grid) + i * step;
                    WLGRID_BOUND2 (new_grid) = WLGRID_BOUND2 (new_grid) + i * step;

                    last = WLGRID_NEXT (last) = new_grid;
                } while (tmp != last_old);
            }
        }
    }

    DBUG_RETURN (grids);
}

/******************************************************************************
 *
 * function:
 *   node *IntersectGrid( node *grid1, node *grid2, int step,
 *                        node **i_grid1, node **i_grid2)
 *
 * description:
 *   returns in 'i_grid1', 'i_grid2' the intersection of 'grid1' and 'grid2'.
 *   both grids must have the same step ('step').
 *
 *   returns NULL if the intersection is equal to the original grid!!
 *
 ******************************************************************************/

void
IntersectGrid (node *grid1, node *grid2, int step, node **i_grid1, node **i_grid2)
{
    int bound11, bound21, bound12, bound22, i_bound1, i_bound2;

    DBUG_ENTER ("IntersectGrid");

    *i_grid1 = *i_grid2 = NULL;

    bound11 = WLGRID_BOUND1 (grid1);
    bound21 = WLGRID_BOUND2 (grid1);

    bound12 = WLGRID_BOUND1 (grid2);
    bound22 = WLGRID_BOUND2 (grid2);

    /* compute bounds of intersection */
    i_bound1 = MAX (bound11, bound12);
    i_bound2 = MIN (bound21, bound22);

    if (i_bound1 < i_bound2) { /* is intersection non-empty? */

        if ((i_bound1 != bound11) || (i_bound2 != bound21)) {
            *i_grid1 = DupNode (grid1);
            WLGRID_BOUND1 ((*i_grid1)) = i_bound1;
            WLGRID_BOUND2 ((*i_grid1)) = i_bound2;
        }

        if ((i_bound1 != bound12) || (i_bound2 != bound22)) {
            *i_grid2 = DupNode (grid2);
            WLGRID_BOUND1 ((*i_grid2)) = i_bound1;
            WLGRID_BOUND2 ((*i_grid2)) = i_bound2;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *MergeWL( node *nodes)
 *
 * description:
 *   returns the merged chain 'nodes'.
 *   if necessary (e.g. if called from 'ComputeCubes') the bounds of the
 *     chain-elements are adjusted.
 *
 ******************************************************************************/

node *
MergeWL (node *nodes)
{
    node *node1, *grids, *new_grids, *grid1, *grid2, *i_grid1, *i_grid2, *tmp;
    int bound1, bound2, step, rear1, count, fixpoint, i;

    DBUG_ENTER ("MergeWL");

    node1 = nodes;
    while (node1 != NULL) {

        /*
         * get all nodes with same bounds as 'node1'
         *
         * (because of the sort order these nodes are
         * located directly after 'node1' in the chain)
         */

        switch (NODE_TYPE (node1)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:
            /* here is no break missing! */
        case N_WLgrid:

            while ((WLNODE_NEXT (node1) != NULL)
                   && (WLNODE_BOUND1 (node1) == WLNODE_BOUND1 (WLNODE_NEXT (node1)))) {

                DBUG_ASSERT ((WLNODE_BOUND2 (node1)
                              == WLNODE_BOUND2 (WLNODE_NEXT (node1))),
                             "wrong bounds found");
                DBUG_ASSERT ((WLNODE_NEXTDIM (node1) != NULL), "dim not found");
                DBUG_ASSERT ((WLNODE_NEXTDIM (WLNODE_NEXT (node1)) != NULL),
                             "dim not found");

                /*
                 * merge 'node1' with his successor
                 */
                WLNODE_NEXTDIM (node1)
                  = InsertWLnodes (WLNODE_NEXTDIM (node1),
                                   WLNODE_NEXTDIM (WLNODE_NEXT (node1)));

                /* the remaining block node is useless now */
                WLNODE_NEXTDIM (WLNODE_NEXT (node1)) = NULL;
                WLNODE_NEXT (node1) = FreeNode (WLNODE_NEXT (node1));
                /* 'WLNODE_NEXT( node1)' points to his successor now */

                /* merge next dimension */
                WLNODE_NEXTDIM (node1) = MergeWL (WLNODE_NEXTDIM (node1));
            }
            break;

        case N_WLstride:

            /*
             * compute new bounds and step
             *             ^^^^^^
             * CAUTION: when called by 'ComputeCubes' the bounds are not equal!!
             */
            rear1 = IndexRearStride (node1);
            bound1 = WLSTRIDE_BOUND1 (node1);
            bound2 = WLSTRIDE_BOUND2 (node1);
            step = WLSTRIDE_STEP (node1);
            count = 0;
            tmp = WLSTRIDE_NEXT (node1);
            while ((tmp != NULL) && (IndexHeadStride (tmp) < rear1)) {
                /* compute new bounds */
                bound1 = MAX (bound1, WLSTRIDE_BOUND1 (tmp));
                bound2 = MIN (bound2, WLSTRIDE_BOUND2 (tmp));
                /* compute new step */
                step = lcm (step, WLSTRIDE_STEP (tmp));
                /* count the number of found dimensions for next traversal */
                count++;
                tmp = WLSTRIDE_NEXT (tmp);
            }

            /*
             * fit all grids to new step and collect them in 'grids'
             */
            grids = NewStepGrids (WLSTRIDE_CONTENTS (node1), WLSTRIDE_STEP (node1), step,
                                  bound1 - WLSTRIDE_BOUND1 (node1));
            for (i = 0; i < count; i++) {
                grids
                  = InsertWLnodes (grids,
                                   NewStepGrids (WLSTRIDE_CONTENTS (
                                                   WLSTRIDE_NEXT (node1)),
                                                 WLSTRIDE_STEP (WLSTRIDE_NEXT (node1)),
                                                 step,
                                                 bound1
                                                   - WLSTRIDE_BOUND1 (
                                                       WLSTRIDE_NEXT (node1))));

                /* the remaining block node is useless now */
                WLSTRIDE_CONTENTS (WLSTRIDE_NEXT (node1)) = NULL;
                WLSTRIDE_NEXT (node1) = FreeNode (WLSTRIDE_NEXT (node1));
                /* 'WLSTRIDE_NEXT( node1)' points to his successor now */
            }

            /*
             * intersect all grids with each other
             *   until fixpoint is reached.
             */
            do {
                fixpoint = 1;
                new_grids = NULL;

                /* check WLGRID_MODIFIED */
                grid1 = grids;
                while (grid1 != NULL) {
                    DBUG_ASSERT ((WLGRID_MODIFIED (grid1) == 0), "grid was modified");
                    grid1 = WLGRID_NEXT (grid1);
                }

                grid1 = grids;
                while (grid1 != NULL) {

                    grid2 = WLGRID_NEXT (grid1);
                    while (grid2 != NULL) {
                        IntersectGrid (grid1, grid2, step, &i_grid1, &i_grid2);
                        if (i_grid1 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid1);
                            WLGRID_MODIFIED (grid1) = 1;
                            fixpoint = 0;
                        }
                        if (i_grid2 != NULL) {
                            new_grids = InsertWLnodes (new_grids, i_grid2);
                            WLGRID_MODIFIED (grid2) = 1;
                            fixpoint = 0;
                        }

                        grid2 = WLGRID_NEXT (grid2);
                    }

                    /* was 'grid1' not modified? */
                    if (WLGRID_MODIFIED (grid1) == 0) {
                        /* insert 'grid1' in 'new_grids' */
                        tmp = grid1;
                        grid1 = WLGRID_NEXT (grid1);
                        WLGRID_NEXT (tmp) = NULL;
                        new_grids = InsertWLnodes (new_grids, tmp);
                    } else {
                        /* 'grid1' is no longer needed */
                        grid1 = FreeNode (grid1);
                        /* 'grid1' points to his successor now! */
                    }
                }

                grids = new_grids;
            } while (!fixpoint);

            /*
             * merge the grids
             */
            WLSTRIDE_BOUND1 (node1) = bound1;
            WLSTRIDE_BOUND2 (node1) = bound2;
            WLSTRIDE_STEP (node1) = step;
            WLSTRIDE_CONTENTS (node1) = MergeWL (grids);
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        node1 = WLNODE_NEXT (node1);
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int IsEqualWLnodes( node *tree1, node *tree2)
 *
 * description:
 *   returns 1 if the N_WL...-trees 'tree1' and 'tree2' are equal.
 *   returns 0 otherwise.
 *
 *   remark: we can not use 'CompareWlnodes' here, because this function only
 *           compares the first level of dimensions.
 *           but here we must compare the *whole* trees --- all block levels,
 *           and even the code ...
 *
 ******************************************************************************/

int
IsEqualWLnodes (node *tree1, node *tree2)
{
    node *tmp1, *tmp2;
    int equal = 1;

    DBUG_ENTER ("IsEqualWLnodes");

    if ((tree1 != NULL) && (tree2 != NULL)) {

        DBUG_ASSERT ((NODE_TYPE (tree1) == NODE_TYPE (tree2)),
                     "can not compare objects of different type");

        /*
         * compare the whole chains
         */
        tmp1 = tree1;
        tmp2 = tree2;
        do {
            DBUG_ASSERT ((tmp2 != NULL), "trees differ in length");

            /*
             * compare type-independent data
             */
            if ((WLNODE_BOUND1 (tmp1) == WLNODE_BOUND1 (tmp2))
                && (WLNODE_BOUND2 (tmp1) == WLNODE_BOUND2 (tmp2))
                && (WLNODE_STEP (tmp1) == WLNODE_STEP (tmp2))) {

                /*
                 * compare type-specific data
                 */
                switch (NODE_TYPE (tmp1)) {

                case N_WLblock:
                    /* here is no break missing! */
                case N_WLublock:

                    /*
                     * CAUTION: to prevent nice ;-) bugs in this code fragment
                     *          WLBLOCK_CONTENTS and WLUBLOCK_CONTENTS must be
                     *          equivalent (as currently realized in tree_basic.h)
                     */
                    if (WLNODE_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CONTENTS is NULL)
                         */
                        DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp1) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        DBUG_ASSERT ((WLBLOCK_CONTENTS (tmp2) == NULL),
                                     "data in NEXTDIM *and* CONTENTS found");
                        equal
                          = IsEqualWLnodes (WLNODE_NEXTDIM (tmp1), WLNODE_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CONTENTS (NEXTDIM is NULL)
                         */
                        equal = IsEqualWLnodes (WLBLOCK_CONTENTS (tmp1),
                                                WLBLOCK_CONTENTS (tmp2));
                    }
                    break;

                case N_WLstride:

                    equal = IsEqualWLnodes (WLSTRIDE_CONTENTS (tmp1),
                                            WLSTRIDE_CONTENTS (tmp2));
                    break;

                case N_WLgrid:

                    if (WLGRID_NEXTDIM (tmp1) != NULL) {
                        /*
                         * compare NEXTDIM (CODE is NULL)
                         */
                        DBUG_ASSERT ((WLGRID_CODE (tmp1) == NULL),
                                     "data in NEXTDIM *and* CODE found");
                        DBUG_ASSERT ((WLGRID_CODE (tmp2) == NULL),
                                     "data in NEXTDIM *and* CODE found");
                        equal
                          = IsEqualWLnodes (WLGRID_NEXTDIM (tmp1), WLGRID_NEXTDIM (tmp2));
                    } else {
                        /*
                         * compare CODE (NEXTDIM is NULL)
                         */
                        equal = (WLGRID_CODE (tmp1) == WLGRID_CODE (tmp2));
                    }
                    break;

                default:

                    DBUG_ASSERT ((0), "wrong node type");
                }

            } else {
                equal = 0;
            }

            tmp1 = WLNODE_NEXT (tmp1);
            tmp2 = WLNODE_NEXT (tmp2);
        } while (equal && (tmp1 != NULL));

    } else {
        equal = ((tree1 == NULL) && (tree2 == NULL));
    }

    DBUG_RETURN (equal);
}

/******************************************************************************
 *
 * function:
 *   node *OptimizeWL( node *nodes)
 *
 * description:
 *   returns the optimized N_WL...-tree 'nodes'.
 *
 ******************************************************************************/

node *
OptimizeWL (node *nodes)
{
    node *next, *grids, *comp1, *comp2;
    int offset;

    DBUG_ENTER ("OptimizeWL");

    if (nodes != NULL) {

        /*
         * optimize the next node
         */
        next = WLNODE_NEXT (nodes) = OptimizeWL (WLNODE_NEXT (nodes));

        /*
         * optimize the type-specific sons
         *
         * save in 'comp1', 'comp2' the son of 'nodes', 'next' respectively.
         */
        switch (NODE_TYPE (nodes)) {

        case N_WLblock:
            /* here is no break missing! */
        case N_WLublock:

            if (WLBLOCK_NEXTDIM (nodes) != NULL) {
                /*
                 * compare NEXTDIM (CONTENTS is NULL)
                 */
                DBUG_ASSERT ((WLBLOCK_CONTENTS (nodes) == NULL),
                             "data in NEXTDIM *and* CONTENTS found");
                comp1 = WLBLOCK_NEXTDIM (nodes) = OptimizeWL (WLBLOCK_NEXTDIM (nodes));
                if (next != NULL) {
                    comp2 = WLBLOCK_NEXTDIM (next);
                }
            } else {
                /*
                 * compare CONTENTS (NEXTDIM is NULL)
                 */
                comp1 = WLBLOCK_CONTENTS (nodes) = OptimizeWL (WLBLOCK_CONTENTS (nodes));
                if (next != NULL) {
                    comp2 = WLBLOCK_CONTENTS (next);
                }
            }
            break;

        case N_WLstride:

            comp1 = WLSTRIDE_CONTENTS (nodes) = OptimizeWL (WLSTRIDE_CONTENTS (nodes));
            if (next != NULL) {
                comp2 = WLSTRIDE_CONTENTS (next);
            }

            /*
             * if the grids contained in the stride have an offset
             * (the first grid does not begin at index 0), remove this offset.
             */
            grids = comp1;
            DBUG_ASSERT ((grids != NULL), "no grid found");
            offset = WLGRID_BOUND1 (grids);
            WLSTRIDE_BOUND1 (nodes) += offset;
            if (offset > 0) {
                do {
                    WLGRID_BOUND1 (grids) -= offset;
                    WLGRID_BOUND2 (grids) -= offset;
                    grids = WLGRID_NEXT (grids);
                } while (grids != NULL);
            }

            /*
             * if the first (and only) grid fills the whole step range
             *   set upper bound of this grid and step to 1
             */
            DBUG_ASSERT ((comp1 != NULL), "no grid found");
            if ((WLGRID_BOUND1 (comp1) == 0)
                && (WLGRID_BOUND2 (comp1) == WLSTRIDE_STEP (nodes))) {
                WLGRID_BOUND2 (comp1) = WLSTRIDE_STEP (nodes) = 1;
            }
            break;

        case N_WLgrid:

            if (WLGRID_NEXTDIM (nodes) != NULL) {
                /*
                 * compare NEXTDIM (CODE is NULL)
                 */
                DBUG_ASSERT ((WLGRID_CODE (nodes) == NULL),
                             "data in NEXTDIM *and* CODE found");
                comp1 = WLGRID_NEXTDIM (nodes) = OptimizeWL (WLGRID_NEXTDIM (nodes));
                if (next != NULL) {
                    comp2 = WLGRID_NEXTDIM (next);
                }
            } else {
                /*
                 * compare CODE (NEXTDIM is NULL)
                 */
                comp1 = WLGRID_CODE (nodes);
                if (next != NULL) {
                    comp2 = WLGRID_CODE (next);
                }
            }
            break;

        default:

            DBUG_ASSERT ((0), "wrong node type");
        }

        /*
         * if 'comp1' and 'comp2' are equal subtrees
         *   we can concate 'nodes' and 'next'
         */
        if (next != NULL) {
            if ((WLNODE_STEP (nodes) == WLNODE_STEP (next))
                && (WLNODE_BOUND2 (nodes) == WLNODE_BOUND1 (next))) {
                if (((comp1 != NULL) && (NODE_TYPE (comp1) != N_Ncode))
                      ? IsEqualWLnodes (comp1, comp2)
                      : (comp1 == comp2)) {
                    /* concate 'nodes' and 'next' */
                    WLNODE_BOUND2 (nodes) = WLNODE_BOUND2 (next);
                    /* free useless data in 'next' */
                    WLNODE_NEXT (nodes) = FreeNode (WLNODE_NEXT (nodes));
                }
            }
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   int GetMaxUnroll( node *nodes, int unroll, int dim)
 *
 * description:
 *   returns the maximally number elements that must be unrolled
 *     in dimension 'dim' of N_WL...-tree 'nodes'.
 *   'unroll' is the initial value for the computation (normally 1).
 *
 *   we must search for the first N_WLublock- or N_WLstride-node in each
 *     leaf of the 'nodes'-tree and get the step of this node.
 *
 ******************************************************************************/

int
GetMaxUnroll (node *nodes, int unroll, int dim)
{
    DBUG_ENTER ("GetMaxUnroll");

    if (nodes != NULL) {

        unroll = GetMaxUnroll (WLNODE_NEXT (nodes), unroll, dim);

        if ((WLNODE_DIM (nodes) == dim)
            && ((NODE_TYPE (nodes) == N_WLublock) || (NODE_TYPE (nodes) == N_WLstride))) {

            /*
             * we have found a node with unrolling information
             */
            unroll = MAX (unroll, WLNODE_STEP (nodes));

        } else {

            /*
             * search in whole tree for nodes with unrolling information
             */
            switch (NODE_TYPE (nodes)) {

            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:

                unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), unroll, dim);
                unroll = GetMaxUnroll (WLBLOCK_CONTENTS (nodes), unroll, dim);
                break;

            case N_WLstride:

                unroll = GetMaxUnroll (WLSTRIDE_CONTENTS (nodes), unroll, dim);
                break;

            case N_WLgrid:

                unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (nodes), unroll, dim);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }
        }
    }

    DBUG_RETURN (unroll);
}

/******************************************************************************
 *
 * function:
 *   node *FitWL( node *nodes, int curr_dim, int dims)
 *
 * description:
 *   returns the fitted N_WL...-tree 'nodes'.
 *   the tree is fitted in the dimension from 'curr_dim' till ('dims'-1).
 *
 ******************************************************************************/

node *
FitWL (node *nodes, int curr_dim, int dims)
{
    node *new_node, *grids, *tmp;
    int unroll, remain, width;

    DBUG_ENTER ("FitWL");

    if (curr_dim < dims) {

        /*
         * traverse the whole chain
         */
        tmp = nodes;
        while (tmp != NULL) {

            switch (NODE_TYPE (tmp)) {

            case N_WLblock:

                if (curr_dim < dims - 1) {
                    /*
                     * fit in next dimension;
                     *   compute unrolling information
                     */
                    WLBLOCK_NEXTDIM (tmp)
                      = FitWL (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                    unroll = GetMaxUnroll (WLBLOCK_NEXTDIM (tmp), 1, curr_dim);
                } else {
                    unroll = GetMaxUnroll (WLBLOCK_CONTENTS (tmp), 1, curr_dim);
                }
                break;

            case N_WLublock:

                if (curr_dim < dims - 1) {
                    /*
                     * fit in next dimension;
                     *   get unrolling information
                     */
                    WLBLOCK_NEXTDIM (tmp)
                      = FitWL (WLBLOCK_NEXTDIM (tmp), curr_dim + 1, dims);
                }
                unroll = WLUBLOCK_STEP (tmp);
                break;

            case N_WLstride:

                grids = WLSTRIDE_CONTENTS (tmp);
                if (curr_dim < dims - 1) {
                    /*
                     * fit for all grids in next dimension;
                     *   get unrolling information
                     */
                    while (grids != NULL) {
                        WLGRID_NEXTDIM (grids)
                          = FitWL (WLGRID_NEXTDIM (grids), curr_dim + 1, dims);
                        grids = WLGRID_NEXT (grids);
                    }
                }
                unroll = WLSTRIDE_STEP (tmp);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }

            /*
             * fit current dimension:
             *   split a uncompleted periode at the end of index range
             */
            width = WLNODE_BOUND2 (tmp) - WLNODE_BOUND1 (tmp);
            remain = width % unroll;
            if ((remain > 0) && (width > remain)) {
                /*
                 *  uncompleted periode found -> split
                 */
                new_node = DupNode (tmp);
                WLNODE_BOUND2 (new_node) = WLNODE_BOUND2 (tmp);
                WLNODE_BOUND2 (tmp) = WLNODE_BOUND1 (new_node)
                  = WLNODE_BOUND2 (tmp) - remain;
                WLNODE_NEXT (new_node) = WLNODE_NEXT (tmp);
                WLNODE_NEXT (tmp) = new_node;
                tmp = new_node;
            }

            tmp = WLNODE_NEXT (tmp);
        }
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWLnodes( node *nodes, int *width)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'width' is an array with one component for each dimension;
 *     here we save the width of the index ranges
 *
 ******************************************************************************/

node *
NormalizeWLnodes (node *nodes, int *width)
{
    node *tmp;
    int curr_width;

    DBUG_ENTER ("NormalizeWLnodes");

    if (nodes != NULL) {

        /*
         * backup width of current dim
         */
        curr_width = width[WLNODE_DIM (nodes)];

        tmp = nodes;
        do {

            /*
             * adjust upper bound
             */
            DBUG_ASSERT ((WLNODE_BOUND1 (tmp) < curr_width), "wrong bounds found");
            WLNODE_BOUND2 (tmp) = MIN (WLNODE_BOUND2 (tmp), curr_width);

            /*
             * remove nodes whose index ranges lies outside the current block
             */
            while ((WLNODE_NEXT (tmp) != NULL)
                   && (WLNODE_BOUND1 (WLNODE_NEXT (tmp)) >= curr_width)) {
                WLNODE_NEXT (tmp) = FreeNode (WLNODE_NEXT (tmp));
            }

            /* take next node */
            tmp = WLNODE_NEXT (tmp);
        } while (tmp != NULL);

        tmp = nodes;
        do {

            /*
             * save width of current index range; adjust step
             */
            width[WLNODE_DIM (tmp)] = WLNODE_BOUND2 (tmp) - WLNODE_BOUND1 (tmp);
            WLNODE_STEP (tmp) = MIN (WLNODE_STEP (tmp), width[WLNODE_DIM (tmp)]);

            /*
             * normalize the type-specific sons
             */
            switch (NODE_TYPE (tmp)) {

            case N_WLblock:
                /* here is no break missing! */
            case N_WLublock:

                WLBLOCK_NEXTDIM (tmp) = NormalizeWLnodes (WLBLOCK_NEXTDIM (tmp), width);
                WLBLOCK_CONTENTS (tmp) = NormalizeWLnodes (WLBLOCK_CONTENTS (tmp), width);
                break;

            case N_WLstride:

                WLSTRIDE_CONTENTS (tmp)
                  = NormalizeWLnodes (WLSTRIDE_CONTENTS (tmp), width);
                break;

            case N_WLgrid:

                WLGRID_NEXTDIM (tmp) = NormalizeWLnodes (WLGRID_NEXTDIM (tmp), width);
                break;

            default:

                DBUG_ASSERT ((0), "wrong node type");
            }

            /* take next node */
            tmp = WLNODE_NEXT (tmp);
        } while (tmp != NULL);

        /*
         * restore width of current dim
         */
        width[WLNODE_DIM (nodes)] = curr_width;
    }

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *NormalizeWL( node *nodes, int *idx_max)
 *
 * description:
 *   returns the normalized N_WL...-tree 'nodes'.
 *   'idx_max' is the supremum of the index-vector.
 *
 ******************************************************************************/

node *
NormalizeWL (node *nodes, int *idx_max)
{
    DBUG_ENTER ("NormalizeWL");

    nodes = NormalizeWLnodes (nodes, idx_max);

    DBUG_RETURN (nodes);
}

/******************************************************************************
 *
 * function:
 *   node *InferParams( node *seg)
 *
 * description:
 *   infers WLSEG_MAXHOMDIM for the given segment 'seg' and WL..._INNERSTEP
 *   for all contained WLblock-, WLublock-, WLstride-nodes with
 *   (WL..._LEVEL == 0).
 *
 ******************************************************************************/

node *
InferParams (node *seg)
{
    DBUG_ENTER ("InferParams");

    DBUG_RETURN (seg);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateCompleteGrid( node *stride_var)
 *
 * description:
 *   Supplements missings parts of the grid in 'stride_var'.
 *
 *   Example (with shape [300,300]):
 *
 *         60 -> 200 step[0] 50
 *                     40 -> 50:  60 -> 200 step[1] 50
 *                                            40 -> 50: op0
 *
 *   =>>
 *
 *        100 -> 200 step[0] 50
 *                      0 -> 10: 100 -> 200 step[1] 50
 *                                             0 -> 10: op0
 *                                            10 -> 50: noop
 *                     10 -> 50: noop
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
GenerateCompleteGrid (node *stride_var)
{
    node *grid_var;

    DBUG_ENTER ("GenerateCompleteGrid");

    if (stride_var != NULL) {

        grid_var = WLSTRIANY_CONTENTS (stride_var);

        if (NODE_TYPE (stride_var) == N_WLstride) {
            DBUG_ASSERT ((NODE_TYPE (grid_var) == N_WLgrid), "wrong node type found");

            /*
             * is the grid incomplete?
             */
            if (WLGRID_BOUND2 (grid_var) - WLGRID_BOUND1 (grid_var)
                < WLSTRIDE_STEP (stride_var)) {
                WLSTRIDE_BOUND1 (stride_var) += WLGRID_BOUND1 (grid_var);
                WLGRID_BOUND2 (grid_var) -= WLGRID_BOUND1 (grid_var);
                WLGRID_BOUND1 (grid_var) = 0;

                WLGRID_NEXT (grid_var)
                  = MakeWLgrid (0, WLGRID_DIM (grid_var), WLGRID_BOUND2 (grid_var),
                                WLSTRIDE_STEP (stride_var), 0, NULL, NULL, NULL);
            }

            /*
             * next dim
             */
            WLGRID_NEXTDIM (grid_var) = GenerateCompleteGrid (WLGRID_NEXTDIM (grid_var));

        } else { /* NODE_TYPE( stride_var) == N_WLstriVar */

            /*
             * CAUTION: the grid can be a N_WLgrid node!!!
             */

            if (NODE_TYPE (grid_var) == N_WLgrid) {
                DBUG_ASSERT ((WLGRID_BOUND1 (grid_var) == 0), "bound1 not zero");

                /*
                 * is the grid incomplete?
                 */
                if ((NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                    || (WLGRID_BOUND2 (grid_var)
                        < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {

                    WLGRID_NEXT (grid_var)
                      = MakeWLgridVar (WLGRID_DIM (grid_var),
                                       MakeNum (WLGRID_BOUND2 (grid_var)),
                                       DupNode (WLSTRIVAR_STEP (stride_var)), NULL, NULL,
                                       NULL);
                }

                /*
                 * next dim
                 */
                WLGRID_NEXTDIM (grid_var)
                  = GenerateCompleteGrid (WLGRID_NEXTDIM (grid_var));

            } else { /* NODE_TYPE( grid_var) == N_WLgridVar */
                DBUG_ASSERT (((NODE_TYPE (WLGRIDVAR_BOUND1 (grid_var)) == N_num)
                              && (NUM_VAL (WLGRIDVAR_BOUND1 (grid_var)) == 0)),
                             "bound1 not zero");

                /*
                 * is the grid incomplete?
                 */
                if ((NODE_TYPE (WLGRIDVAR_BOUND2 (grid_var)) != N_num)
                    || (NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                    || (NUM_VAL (WLGRIDVAR_BOUND2 (grid_var))
                        < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {

                    WLGRIDVAR_NEXT (grid_var)
                      = MakeWLgridVar (WLGRIDVAR_DIM (grid_var),
                                       DupNode (WLGRIDVAR_BOUND2 (grid_var)),
                                       DupNode (WLSTRIVAR_STEP (stride_var)), NULL, NULL,
                                       NULL);
                }

                /*
                 * next dim
                 */
                WLGRIDVAR_NEXTDIM (grid_var)
                  = GenerateCompleteGrid (WLGRIDVAR_NEXTDIM (grid_var));
            }
        }
    }

    DBUG_RETURN (stride_var);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateShapeStrides( int dim, int dims, shpseg* shape)
 *
 * description:
 *   Returns strides/grids of the size found in 'shape'.
 *
 *   This function is called by 'GenerateCompleteDomain',
 *    'GenerateCompleteDomainVar'.
 *
 ******************************************************************************/

node *
GenerateShapeStrides (int dim, int dims, shpseg *shape)
{
    node *new_grid, *strides = NULL;

    DBUG_ENTER ("GenerateShapeStrides");

    if (dim < dims) {
        new_grid = MakeWLgrid (0, dim, 0, 1, 0,
                               GenerateShapeStrides (dim + 1, dims, shape), NULL, NULL);
        strides
          = MakeWLstride (0, dim, 0, SHPSEG_SHAPE (shape, dim), 1, 0, new_grid, NULL);
    }

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateCompleteDomain( node *strides,
 *                                 int dims, shpseg *shape)
 *
 * description:
 *   Supplements strides/grids for the complement of 'stride'.
 *
 *   For constant strides we must *not* optimize and merge strides, because
 *   'BlockWL()' can not handle them!! We must create simple cubes instead.
 *   Example (with shape [10,10]):
 *
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *
 *     is *not* converted into (the following is not a cube!!)
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *                          5 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *                  1 -> 2: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *
 *     but into
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> 5  step[1] 1
 *                                     0 -> 1: op
 *                  1 -> 2: 0 -> 5  step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 1
 *                  0 -> 1: 5 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
GenerateCompleteDomain (node *strides, int dims, shpseg *shape)
{
    node *new_strides, *comp_strides, *stride, *grid, *comp_stride, *comp_grid,
      *last_comp_grid, *new_stride, *new_grid, *dup_strides, *last_dup_grid, *next_dim;

    DBUG_ENTER ("GenerateCompleteDomain");

    DBUG_ASSERT ((shape != NULL), "no shape found");

    /*
     * we duplicate 'strides'
     *  -> later on we use this to generate complement strides
     */
    DBUG_ASSERT ((WLSTRIDE_NEXT (strides) == NULL), "more than one stride found");
    comp_strides = DupNode (strides);
    /*
     * in the duplicated chain we set all steps to '1'
     */
    comp_stride = comp_strides;
    while (comp_stride != NULL) {
        WLSTRIDE_STEP (comp_stride) = 1;
        comp_grid = WLSTRIDE_CONTENTS (comp_stride);
        WLSTRIDE_BOUND1 (comp_stride) += WLGRID_BOUND1 (comp_grid);
        WLGRID_BOUND1 (comp_grid) = 0;
        WLGRID_BOUND2 (comp_grid) = 1;
        comp_stride = WLGRID_NEXTDIM (comp_grid);
    }
    /*
     * this chain is base for complements.
     *  -> we must remove the code.
     */
    WLGRID_CODE (comp_grid) = NULL;

    new_strides = NULL;
    stride = strides;
    comp_stride = comp_strides;
    last_comp_grid = NULL;
    while (stride != NULL) {
        DBUG_ASSERT ((NODE_TYPE (stride) == N_WLstride), "no constant stride found");

        grid = WLSTRIDE_CONTENTS (stride);
        comp_grid = WLSTRIDE_CONTENTS (comp_stride);
        DBUG_ASSERT ((NODE_TYPE (grid) == N_WLgrid), "no constant grid found");

        /*
         * normalize the bounds
         */
        WLSTRIDE_BOUND1 (stride) += WLGRID_BOUND1 (grid);
        WLGRID_BOUND2 (grid) -= WLGRID_BOUND1 (grid);
        WLGRID_BOUND1 (grid) = 0;

        /*
         * insert lower part of complement
         */
        if (WLSTRIDE_BOUND2 (stride) < SHPSEG_SHAPE (shape, WLSTRIDE_DIM (stride))) {
            if (last_comp_grid != NULL) {
                /*
                 * duplicate 'comp_strides' from root til 'last_comp_grid'.
                 */
                next_dim = WLGRID_NEXTDIM (last_comp_grid);
                WLGRID_NEXTDIM (last_comp_grid) = NULL;
                dup_strides = DupNode (comp_strides);
                WLGRID_NEXTDIM (last_comp_grid) = next_dim;
                /*
                 * go to duplicated 'last_comp_grid'
                 */
                last_dup_grid = WLSTRIDE_CONTENTS (dup_strides);
                while (WLGRID_NEXTDIM (last_dup_grid) != NULL) {
                    last_dup_grid = WLSTRIDE_CONTENTS (WLGRID_NEXTDIM (last_dup_grid));
                }
            }

            /*
             * generate new stride/grid
             */
            new_grid
              = MakeWLgrid (0, WLGRID_DIM (grid), 0, 1, 0,
                            GenerateShapeStrides (WLGRID_DIM (grid) + 1, dims, shape),
                            NULL, NULL);
            new_stride = MakeWLstride (0, WLSTRIDE_DIM (stride), WLSTRIDE_BOUND2 (stride),
                                       SHPSEG_SHAPE (shape, WLSTRIDE_DIM (stride)), 1, 0,
                                       new_grid, NULL);

            /*
             * append new stride/grid to duplicated 'comp_strides'
             */
            if (last_comp_grid != NULL) {
                WLGRID_NEXTDIM (last_dup_grid) = new_stride;
            } else {
                dup_strides = new_stride;
            }

            /*
             * insert 'dup_strides' into 'new_strides'
             */
            new_strides = InsertWLnodes (new_strides, dup_strides);
        }

        /*
         * insert upper part of complement
         */
        if (WLSTRIDE_BOUND1 (stride) > 0) {
            if (last_comp_grid != NULL) {
                /*
                 * duplicate 'comp_strides' from root til 'last_comp_grid'.
                 */
                next_dim = WLGRID_NEXTDIM (last_comp_grid);
                WLGRID_NEXTDIM (last_comp_grid) = NULL;
                dup_strides = DupNode (comp_strides);
                WLGRID_NEXTDIM (last_comp_grid) = next_dim;
                /*
                 * go to duplicated 'last_comp_grid'
                 */
                last_dup_grid = WLSTRIDE_CONTENTS (dup_strides);
                while (WLGRID_NEXTDIM (last_dup_grid) != NULL) {
                    last_dup_grid = WLSTRIDE_CONTENTS (WLGRID_NEXTDIM (last_dup_grid));
                }
            }

            /*
             * generate new stride/grid
             */
            new_grid
              = MakeWLgrid (0, WLGRID_DIM (grid), 0, 1, 0,
                            GenerateShapeStrides (WLGRID_DIM (grid) + 1, dims, shape),
                            NULL, NULL);
            new_stride = MakeWLstride (0, WLSTRIDE_DIM (stride), 0,
                                       WLSTRIDE_BOUND1 (stride), 1, 0, new_grid, NULL);

            /*
             * append new stride/grid to duplicated 'comp_strides'
             */
            if (last_comp_grid != NULL) {
                WLGRID_NEXTDIM (last_dup_grid) = new_stride;
            } else {
                dup_strides = new_stride;
            }

            /*
             * insert 'dup_strides' into 'new_strides'
             */
            new_strides = InsertWLnodes (new_strides, dup_strides);
        }

        /*
         * is the grid incomplete?
         */
        if (WLGRID_BOUND2 (grid) - WLGRID_BOUND1 (grid) < WLSTRIDE_STEP (stride)) {
            WLGRID_NEXT (grid)
              = MakeWLgrid (0, WLGRID_DIM (grid), WLGRID_BOUND2 (grid),
                            WLSTRIDE_STEP (stride), 0,
                            DupNode (WLGRID_NEXTDIM (comp_grid)), NULL, NULL);
        }

        /*
         * next dim
         */
        stride = WLGRID_NEXTDIM (grid);
        comp_stride = WLGRID_NEXTDIM (comp_grid);
        last_comp_grid = comp_grid;
    }

    /*
     * insert completed stride/grid into 'new_strides'
     */
    new_strides = InsertWLnodes (new_strides, strides);

    /*
     * the copy of 'strides' is useless now
     */
    comp_strides = FreeTree (comp_strides);

    DBUG_RETURN (new_strides);
}

/******************************************************************************
 *
 * function:
 *   node *GenerateCompleteDomainVar( node *stride_var, int dims, shpseg *shape)
 *
 * description:
 *   Supplements strides/grids for the complement of 'stride_var'.
 *
 *   For variable strides we do not call 'SplitWL()', 'MergeWL()', 'OptWL()', ...
 *   therefore we must create optimized and merged strides/grids.
 *   (This means, we actually do not create a cube!!!)
 *   Example (with shape [10,10]):
 *
 *       5 -> 10 step[0] 2
 *                  0 -> 1: a -> b  step[1] 1
 *                                     0 -> 1: op
 *
 *     is converted into
 *
 *       0 ->  5 step[0] 1
 *                  0 -> 1: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *       5 -> 10 step[0] 2
 *                  0 -> 1: 0 -> a  step[1] 1
 *                                     0 -> 1: init/copy/noop
 *                          a -> b  step[1] 1
 *                                     0 -> 1: op
 *                          b -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *                  1 -> 2: 0 -> 10 step[1] 1
 *                                     0 -> 1: init/copy/noop
 *
 *   This function is called by 'ComputeOneCube'.
 *
 ******************************************************************************/

node *
GenerateCompleteDomainVar (node *stride_var, int dims, shpseg *shape)
{
    node *grid_var, *new_grid;

    DBUG_ENTER ("GenerateCompleteDomainVar");

    DBUG_ASSERT ((shape != NULL), "no shape found");

    if (stride_var != NULL) {
        DBUG_ASSERT ((NODE_TYPE (stride_var) == N_WLstriVar), "no variable stride found");

        grid_var = WLSTRIANY_CONTENTS (stride_var);
        /*
         * CAUTION: the grid can be a N_WLgrid node!!!
         */

        if (NODE_TYPE (grid_var) == N_WLgrid) {

            /*
             * is the grid incomplete?
             */
            if ((NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                || (WLGRID_BOUND2 (grid_var) < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {
                WLGRID_NEXT (grid_var)
                  = MakeWLgridVar (WLGRID_DIM (grid_var),
                                   MakeNum (WLGRID_BOUND2 (grid_var)),
                                   DupNode (WLSTRIVAR_STEP (stride_var)),
                                   GenerateShapeStrides (WLGRID_DIM (grid_var) + 1, dims,
                                                         shape),
                                   NULL, NULL);
            }

            /*
             * next dim
             */
            WLGRID_NEXTDIM (grid_var)
              = GenerateCompleteDomainVar (WLGRID_NEXTDIM (grid_var), dims, shape);

            /*
             * append lower part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND2 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND2 (stride_var))
                    < SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var)))) {
                new_grid = MakeWLgrid (0, WLGRID_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRID_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                WLSTRIVAR_NEXT (stride_var)
                  = MakeWLstriVar (WLSTRIVAR_DIM (stride_var),
                                   DupNode (WLSTRIVAR_BOUND2 (stride_var)),
                                   MakeNum (
                                     SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var))),
                                   MakeNum (1), new_grid, NULL);
            }

            /*
             * insert upper part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND1 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND1 (stride_var)) > 0)) {
                new_grid = MakeWLgrid (0, WLGRID_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRID_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                stride_var = MakeWLstriVar (WLSTRIVAR_DIM (stride_var), MakeNum (0),
                                            DupNode (WLSTRIVAR_BOUND1 (stride_var)),
                                            MakeNum (1), new_grid, stride_var);
            }

        } else { /* NODE_TYPE( grid_var) == N_WLgridVar */

            /*
             * is the grid incomplete?
             */
            if ((NODE_TYPE (WLGRIDVAR_BOUND2 (grid_var)) != N_num)
                || (NODE_TYPE (WLSTRIVAR_STEP (stride_var)) != N_num)
                || (NUM_VAL (WLGRIDVAR_BOUND2 (grid_var))
                    < NUM_VAL (WLSTRIVAR_STEP (stride_var)))) {
                WLGRIDVAR_NEXT (grid_var)
                  = MakeWLgridVar (WLGRIDVAR_DIM (grid_var),
                                   DupNode (WLGRIDVAR_BOUND2 (grid_var)),
                                   DupNode (WLSTRIVAR_STEP (stride_var)),
                                   GenerateShapeStrides (WLGRIDVAR_DIM (grid_var) + 1,
                                                         dims, shape),
                                   NULL, NULL);
            }

            /*
             * next dim
             */
            WLGRIDVAR_NEXTDIM (grid_var)
              = GenerateCompleteDomainVar (WLGRIDVAR_NEXTDIM (grid_var), dims, shape);

            /*
             * append lower part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND2 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND2 (stride_var))
                    < SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var)))) {
                new_grid = MakeWLgrid (0, WLGRIDVAR_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRIDVAR_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                WLSTRIVAR_NEXT (stride_var)
                  = MakeWLstriVar (WLSTRIVAR_DIM (stride_var),
                                   DupNode (WLSTRIVAR_BOUND2 (stride_var)),
                                   MakeNum (
                                     SHPSEG_SHAPE (shape, WLSTRIVAR_DIM (stride_var))),
                                   MakeNum (1), new_grid, NULL);
            }

            /*
             * insert upper part of complement
             */
            if ((NODE_TYPE (WLSTRIVAR_BOUND1 (stride_var)) != N_num)
                || (NUM_VAL (WLSTRIVAR_BOUND1 (stride_var)) > 0)) {
                new_grid = MakeWLgrid (0, WLGRIDVAR_DIM (grid_var), 0, 1, 0,
                                       GenerateShapeStrides (WLGRIDVAR_DIM (grid_var) + 1,
                                                             dims, shape),
                                       NULL, NULL);
                stride_var = MakeWLstriVar (WLSTRIVAR_DIM (stride_var), MakeNum (0),
                                            DupNode (WLSTRIVAR_BOUND1 (stride_var)),
                                            MakeNum (1), new_grid, stride_var);
            }
        }
    }

    DBUG_RETURN (stride_var);
}

/******************************************************************************
 *
 * function:
 *   node* ComputeOneCube( node *stride_var, WithOpType wltype,
 *                         int dims, shpseg *shape)
 *
 * description:
 *   If the with-loop contains one part/generator only, we must supplement
 *   new generators for the complement.
 *
 * remark:
 *   The new generators contain no pointer to a code-block. We inspect the
 *   type of the with-loop (WO_genarray, WO_modarray, WO_fold...) to decide
 *   whether we must ...
 *     ... initialize the array-part with 0 (WO_genarray -> 'init'),
 *     ... copy the source-array (WO_modarray -> 'copy'),
 *     ... do nothing (WO_fold -> 'noop').
 *
 ******************************************************************************/

node *
ComputeOneCube (node *stride_var, WithOpType wltype, int dims, shpseg *shape)
{
    DBUG_ENTER ("ComputeOneCube");

    if ((wltype == WO_genarray) || (wltype == WO_modarray)) {
        if (NODE_TYPE (stride_var) == N_WLstriVar) {
            stride_var = GenerateCompleteDomainVar (stride_var, dims, shape);
        } else { /* N_WLstride */
            stride_var = GenerateCompleteDomain (stride_var, dims, shape);
        }
    } else { /* WO_fold... */
        stride_var = GenerateCompleteGrid (stride_var);
    }

    DBUG_RETURN (stride_var);
}

/******************************************************************************
 *
 * function:
 *   node *ComputeCubes( node *strides)
 *
 * description:
 *   returns the set of cubes as a N_WLstride-chain
 *
 ******************************************************************************/

node *
ComputeCubes (node *strides)
{
    node *new_strides, *stride1, *stride2, *i_stride1, *i_stride2, *remain, *last_remain,
      *last_stride1, *tmp;
    int fixpoint;

    DBUG_ENTER ("ComputeCubes");

    DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "wrong node type found");

#if 0
  /*
   * first step:
   *
   * if a stride contains ...
   */
#endif

    /*
     * second step:
     *
     * create disjunct outlines
     *  -> every stride lies in one and only one cube
     */
    do {
        fixpoint = 1;
        new_strides = NULL;

        /* check WLSTRIDE_MODIFIED */
        stride1 = strides;
        while (stride1 != NULL) {
            DBUG_ASSERT ((WLSTRIDE_MODIFIED (stride1) == 0), "stride was modified");
            stride1 = WLSTRIDE_NEXT (stride1);
        }

        /* intersect the elements of 'strides' in pairs */
        stride1 = strides;
        while (stride1 != NULL) {

            stride2 = WLSTRIDE_NEXT (stride1);
            while (stride2 != NULL) {

                /* intersect outlines of 'stride1' and 'stride2' */
                IntersectOutline (stride1, stride2, &i_stride1, &i_stride2);

                if (i_stride1 != NULL) {
                    if (CompareWLnode (stride1, i_stride1, 1) != 0) {
                        fixpoint = 0;
                        WLSTRIDE_MODIFIED (stride1) = 1;
                        new_strides = InsertWLnodes (new_strides, i_stride1);
                    } else {
                        /*
                         * 'stride1' and 'i_stride1' are equal
                         *  -> free 'i_stride1'
                         */
                        i_stride1 = FreeTree (i_stride1);
                    }
                }

                if (i_stride2 != NULL) {
                    if (CompareWLnode (stride2, i_stride2, 1) != 0) {
                        fixpoint = 0;
                        WLSTRIDE_MODIFIED (stride2) = 1;
                        new_strides = InsertWLnodes (new_strides, i_stride2);
                    } else {
                        /*
                         * 'stride2' and 'i_stride2' are equal
                         *  -> free 'i_stride2'
                         */
                        i_stride2 = FreeTree (i_stride2);
                    }
                }

                stride2 = WLSTRIDE_NEXT (stride2);
            }

            /* was 'stride1' not modified? */
            if (WLSTRIDE_MODIFIED (stride1) == 0) {
                /* insert 'stride1' in 'new_strides' */
                tmp = stride1;
                stride1 = WLSTRIDE_NEXT (stride1);
                WLSTRIDE_NEXT (tmp) = NULL;
                new_strides = InsertWLnodes (new_strides, tmp);
            } else {
                /* 'stride1' is no longer needed */
                stride1 = FreeNode (stride1);
                /* 'stride1' points to his successor now! */
            }
        }

        strides = new_strides;
    } while (!fixpoint);

    /*
     * third step:
     *
     * merge the strides of each cube
     */
    stride1 = strides;
    while (stride1 != NULL) {

        /*
         * collect all strides, that lie in the same cube as 'stride1'.
         * 'remain' collects the remaining strides.
         */
        stride2 = WLSTRIDE_NEXT (stride1);
        last_stride1 = NULL;
        remain = last_remain = NULL;
        while (stride2 != NULL) {

            /* lie 'stride1' and 'stride2' in the same cube? */
            if (IntersectOutline (stride1, stride2, NULL, NULL)) {
                /*
                 * 'stride1' and 'stride2' lie in the same cube
                 *  -> append 'stride2' to the 'stride1'-chain
                 */
                if (last_stride1 == NULL) {
                    WLSTRIDE_NEXT (stride1) = stride2;
                } else {
                    WLSTRIDE_NEXT (last_stride1) = stride2;
                }
                last_stride1 = stride2;
            } else {
                /*
                 * 'stride2' lies not in the same cube as 'stride1'
                 *  -> append 'stride2' to to 'remain'-chain
                 */
                if (remain == NULL) {
                    remain = stride2;
                } else {
                    WLSTRIDE_NEXT (last_remain) = stride2;
                }
                last_remain = stride2;
            }

            stride2 = WLSTRIDE_NEXT (stride2);
        }

        /*
         * merge the 'stride1'-chain
         */
        if (last_stride1 != NULL) {
            WLSTRIDE_NEXT (last_stride1) = NULL;
            stride1 = MergeWL (stride1);
        }

        if (strides == NULL) {
            strides = stride1;
        }

        WLSTRIDE_NEXT (stride1) = remain;
        if (last_remain != NULL) {
            WLSTRIDE_NEXT (last_remain) = NULL;
        }
        stride1 = remain;
    }
    strides = new_strides;

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRANwith( node *arg_node, node *arg_info)
 *
 * description:
 *   transforms with-loop (N_Nwith-node) into new representation (N_Nwith2).
 *
 * remark:
 *   'INFO_WL_SHPSEG( arg_info)' points to the shape-segs of 'LET_IDS'.
 *
 ******************************************************************************/

node *
WLTRANwith (node *arg_node, node *arg_info)
{
    node *new_node, *strides, *cubes, *segs, *seg;
    int dims, b;
    enum {
        WL_PH_conv,
        WL_PH_cubes,
        WL_PH_segs,
        WL_PH_split,
        WL_PH_block,
        WL_PH_ublock,
        WL_PH_merge,
        WL_PH_opt,
        WL_PH_fit,
        WL_PH_norm
    } WL_break_after;

    DBUG_ENTER ("WLTRANwith");

    /*
     * store the lineno of the current with-loop
     *  (for generation of error-messages)
     */
    line = NODE_LINE (arg_node);

    /* analyse 'break_specifier' */
    WL_break_after = WL_PH_norm;
    if (break_after == PH_wltrans) {
        if (strcmp (break_specifier, "conv") == 0) {
            WL_break_after = WL_PH_conv;
        } else {
            if (strcmp (break_specifier, "cubes") == 0) {
                WL_break_after = WL_PH_cubes;
            } else {
                if (strcmp (break_specifier, "segs") == 0) {
                    WL_break_after = WL_PH_segs;
                } else {
                    if (strcmp (break_specifier, "split") == 0) {
                        WL_break_after = WL_PH_split;
                    } else {
                        if (strcmp (break_specifier, "block") == 0) {
                            WL_break_after = WL_PH_block;
                        } else {
                            if (strcmp (break_specifier, "ublock") == 0) {
                                WL_break_after = WL_PH_ublock;
                            } else {
                                if (strcmp (break_specifier, "merge") == 0) {
                                    WL_break_after = WL_PH_merge;
                                } else {
                                    if (strcmp (break_specifier, "opt") == 0) {
                                        WL_break_after = WL_PH_opt;
                                    } else {
                                        if (strcmp (break_specifier, "fit") == 0) {
                                            WL_break_after = WL_PH_fit;
                                        } else {
                                            if (strcmp (break_specifier, "norm") == 0) {
                                                WL_break_after = WL_PH_norm;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    /*
     * get number of dims of with-loop index range
     */
    dims = IDS_SHAPE (NWITHID_VEC (NPART_WITHID (NWITH_PART (arg_node))), 0);

    new_node = MakeNWith2 (NPART_WITHID (NWITH_PART (arg_node)), NULL,
                           NWITH_CODE (arg_node), NWITH_WITHOP (arg_node), dims);

    NWITH2_DEC_RC_IDS (new_node) = NWITH_DEC_RC_IDS (arg_node);
    NWITH2_IN (new_node) = NWITH_IN (arg_node);
    NWITH2_INOUT (new_node) = NWITH_INOUT (arg_node);
    NWITH2_OUT (new_node) = NWITH_OUT (arg_node);
    NWITH2_LOCAL (new_node) = NWITH_LOCAL (arg_node);

    /*
     * withid, code, withop and IN/INOUT/OUT/LOCAL are overtaken to the Nwith2-tree
     *  without a change.
     * Because of that, these parts are cut off from the old nwith-tree,
     *  before freeing it.
     */

    NPART_WITHID (NWITH_PART (arg_node)) = NULL;
    NWITH_CODE (arg_node) = NULL;
    NWITH_WITHOP (arg_node) = NULL;

    NWITH_DEC_RC_IDS (arg_node) = NULL;
    NWITH_IN (arg_node) = NULL;
    NWITH_INOUT (arg_node) = NULL;
    NWITH_OUT (arg_node) = NULL;
    NWITH_LOCAL (arg_node) = NULL;

    /*
     * convert parts of with-loop into new format
     */
    DBUG_EXECUTE ("WLprec", NOTE (("step 0: converting parts to strides\n")));
    strides = Parts2Strides (NWITH_PART (arg_node), dims);

    if (WL_break_after >= WL_PH_cubes) {
        /*
         * build the cubes
         */
        DBUG_EXECUTE ("WLprec", NOTE (("step 1: cube-building\n")));
        if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
            /*
             * we have one part only.
             *  -> the index-range of the generator is possibly a *proper* subset of
             *     the index-vector-space.
             *  -> the generator params are possibly vars.
             */
            cubes = ComputeOneCube (strides, NWITH2_TYPE (new_node), dims,
                                    INFO_WL_SHPSEG (arg_info));
        } else {
            /*
             * we have multiple parts.
             *  -> the index-ranges of the generators partitionize the index-vector-space.
             *  -> the generator params are constant.
             *
             * remark: for the time being these assertions are not a restriction, because
             *         in a SAC-source we can specifiy one part only.
             *         Therefore multiple parts are generated exclusiv by WLF, and these
             *         multiple parts meet the above conditions.
             */
            cubes = ComputeCubes (strides);
        }

        if (NODE_TYPE (cubes) == N_WLstride) {

            /*
             * all parameters are constant.
             *  -> full-featured wltransformation
             */

            if (WL_break_after >= WL_PH_segs) {
                DBUG_EXECUTE ("WLprec", NOTE (("step 2: choice of segments\n")));
                segs = SetSegs (NWITH_PRAGMA (arg_node), cubes, dims);
                /* free temporary data */
                if (NWITH_PRAGMA (arg_node) != NULL) {
                    NWITH_PRAGMA (arg_node) = FreeTree (NWITH_PRAGMA (arg_node));
                }
                if (cubes != NULL) {
                    cubes = FreeTree (cubes);
                }

                seg = segs;
                while (seg != NULL) {
                    /* check params of segment */
                    CheckParams (seg);

                    /* splitting */
                    if (WL_break_after >= WL_PH_split) {
                        DBUG_EXECUTE ("WLprec", NOTE (("step 3: splitting\n")));
                        WLSEG_CONTENTS (seg) = SplitWL (WLSEG_CONTENTS (seg));
                    }

                    /* hierarchical blocking */
                    if (WL_break_after >= WL_PH_block) {
                        DBUG_EXECUTE ("WLprec",
                                      NOTE (("step 4: hierarchical blocking\n")));
                        for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                            DBUG_EXECUTE (
                              "WLprec",
                              NOTE (("step 4.%d: hierarchical blocking (level %d)\n",
                                     b + 1, b)));
                            WLSEG_CONTENTS (seg) = BlockWL (WLSEG_CONTENTS (seg), dims,
                                                            WLSEG_BV (seg, b), 0);
                        }
                    }

                    /* unrolling-blocking */
                    if (WL_break_after >= WL_PH_ublock) {
                        DBUG_EXECUTE ("WLprec", NOTE (("step 5: unrolling-blocking\n")));
                        WLSEG_CONTENTS (seg)
                          = BlockWL (WLSEG_CONTENTS (seg), dims, WLSEG_UBV (seg), 1);
                    }

                    /* merging */
                    if (WL_break_after >= WL_PH_merge) {
                        DBUG_EXECUTE ("WLprec", NOTE (("step 6: merging\n")));
                        WLSEG_CONTENTS (seg) = MergeWL (WLSEG_CONTENTS (seg));
                    }

                    /* optimization */
                    if (WL_break_after >= WL_PH_opt) {
                        DBUG_EXECUTE ("WLprec", NOTE (("step 7: optimization\n")));
                        WLSEG_CONTENTS (seg) = OptimizeWL (WLSEG_CONTENTS (seg));
                    }

                    /* fitting */
                    if (WL_break_after >= WL_PH_fit) {
                        DBUG_EXECUTE ("WLprec", NOTE (("step 8: fitting\n")));
                        WLSEG_CONTENTS (seg) = FitWL (WLSEG_CONTENTS (seg), 0, dims);
                    }

                    /* normalization */
                    if (WL_break_after >= WL_PH_norm) {
                        DBUG_EXECUTE ("WLprec", NOTE (("step 9: normalization\n")));
                        WLSEG_CONTENTS (seg)
                          = NormalizeWL (WLSEG_CONTENTS (seg), WLSEG_IDX_MAX (seg));
                    }

                    /* infer WLSEG_MAXHOMDIM and WL..._INNERSTEP */
                    seg = InferParams (seg);

                    seg = WLSEG_NEXT (seg);
                }
            } else {
                /*
                 * we want to stop after cube-building.
                 *  -> build one segment containing all cubes.
                 */
                segs = All (NULL, NULL, cubes, dims, line);
            }
        } else {
            /*
             * not all params are constant.
             *  -> build one segment containing all cubes.
             */
            segs = All (NULL, NULL, cubes, dims, line);
        }
    } else {
        /*
         * we want to stop after converting.
         *  -> build one segment containing the strides.
         */
        segs = All (NULL, NULL, strides, dims, line);
    }

    NWITH2_SEGS (new_node) = segs;

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRANcode( node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of Ncode-nodes.
 *
 * remarks:
 *   - CODE_NO is set in the whole Ncode-chain
 *
 ******************************************************************************/

node *
WLTRANcode (node *arg_node, node *arg_info)
{
    node *code;
    int no = 0;

    DBUG_ENTER ("WLTRANcode");

    code = arg_node;
    while (code != NULL) {
        NCODE_NO (code) = no;

        if (NCODE_CBLOCK (code) != NULL) {
            NCODE_CBLOCK (code) = Trav (NCODE_CBLOCK (code), arg_info);
        }
        if (NCODE_CEXPR (code) != NULL) {
            NCODE_CEXPR (code) = Trav (NCODE_CEXPR (code), arg_info);
        }

        no++;
        code = NCODE_NEXT (code);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRAFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses sons.
 *
 ******************************************************************************/

node *
WLTRAFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLTRAFundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLTRAFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   'INFO_WL_SHPSEG( arg_info)' points to the shape-segs of 'LET_IDS'
 *   (needed for 'WLTRANwith').
 *
 ******************************************************************************/

node *
WLTRALet (node *arg_node, node *arg_info)
{
    shpseg *tmp;

    DBUG_ENTER ("WLTRALet");

    tmp = INFO_WL_SHPSEG (arg_info);
    INFO_WL_SHPSEG (arg_info) = VARDEC_SHPSEG (LET_VARDEC (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_WL_SHPSEG (arg_info) = tmp;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WlTransform( node *syntax_tree)
 *
 * description:
 *   In this compiler phase all N_Nwith nodes are transformed in N_Nwith2
 *   nodes.
 *
 ******************************************************************************/

node *
WlTransform (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("WlTransform");

    info = MakeInfo ();

    act_tab = wltrans_tab;
    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}
