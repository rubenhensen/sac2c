/*****************************************************************************
 *
 * function:
 *    int ReverseBits( int x, int bits)
 *
 * description:
 *    reverses the <bits> lowest bits from <x> and returns them as int.
 *
 */

int
ReverseBits (int x, int bits)
{
    int res = 0;
    int mask = 1;
    int shift;

    shift = bits - 1;
    while (shift >= 0) {
        res += (x & mask) << shift;
        shift -= 2;
        mask <<= 1;
    }
    shift = -shift;
    while (shift < bits) {
        res += (x & mask) >> shift;
        shift += 2;
        mask <<= 1;
    }
    return (res);
}

/*****************************************************************************
 *
 * function:
 *    int MaxBit( int x)
 *
 * description:
 *    determines the highest non-zero bit.
 *
 */

int
MaxBit (int x)
{
    int counter = 0;
    while ((x >> counter) != 0)
        counter++;
    return (counter);
}
