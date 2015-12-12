float **Matrix (int r, int c);
void DelMatrix (float **a, int r, int c);
void Transpose (float **a, int r, int c, float **trans);
void MxMMultiply (float **a, int r1, int c1, float **b, int r2, int c2, float **mult);
void MxVMultiply (float **a, int r, int c, float *b, float *mult);
void Inverse (float **a, int n, float **inv);
float Determinant (float **a, int n);
void CoFactor (float **a, int n, float **b);
void PolyRegression (float **X, int r, int c, float *y, float *reg);
