#include "bigMSolver.h"
#include "utility.h"
#ifdef PARALLEL
#include "omp.h"
#endif

BigMSolver::BigMSolver(int n, int m, Matrix c, Matrix a, Matrix b, Matrix d,
                                           Matrix e) :
        Solver(n, m, c, a, b, d, e) {}

BigMSolver::~BigMSolver() = default;

void BigMSolver::exchange(int inIndex, int outIndex) {
#ifdef DEBUG
    cout << "in: x" << inIndex << " out: x" << baseIndex[outIndex] << endl;
#endif
    baseIndex[outIndex] = inIndex;
    Row pivot = a[outIndex];
    Row pivotB = b[outIndex];
    // normalize to one
    double toOneRatio = 1 / a[outIndex][inIndex];
    pivot.multi(toOneRatio);
    pivotB.multi(toOneRatio);
    // normalize other rows to zero
#ifdef PARALLEL
#pragma omp parallel for default(none) shared(pivot) shared(pivotB) shared(inIndex) shared(outIndex)
#endif
    for (int i = 0; i < m; i++) {
        if (i != outIndex) {
            Row another = a[i];
            Row anotherB = b[i];
            double ratio = -another[inIndex];
            another.addRow(pivot, ratio);
            anotherB.addRow(pivotB, ratio);
        }
#ifdef DEBUG
#ifdef PARALLEL
        printf("parallel: i = %d, run on Thread %d!\n", i, omp_get_thread_num());
#endif
#endif
    }
    // transform checked number to zero
    Row cRow = c[0];
    double ratio = -cRow[inIndex];
    cRow.addRow(pivot, ratio);
    value += pivotB[0] * ratio;
}

void BigMSolver::solve(int &k, double &y, Matrix &x) {
    nonManualVariableCount = n;
    // add artificial variables
    Matrix zero = Matrix(m, 1);
    Matrix one = Matrix(1, 1, 1);
    Matrix matrixM = Matrix(1, 1, M);
    for (int i = 0; i < m; i++) {
        a.appendColumn(zero);
        a[i][n] = 1;
        c.appendColumn(matrixM);
        e.appendColumn(one);
        baseIndex.push_back(n);
        n++;
    }
    // to max format
    Row checkedArray = c[0];
    for (int i = 0; i < n; i++) {
        checkedArray[i] = -checkedArray[i];
    }
    // normalize checked number of mannual variables
    for (int i = 0; i < m; i++) {
        auto rowi = a[i];
        checkedArray.addRow(rowi, M);
        double delta = M * b[i][0];
        value += delta;
    }
#ifdef DEBUG
    printSimplexTable();
#endif

    while (true) {
        // get max
        int inIndex = -1;
        double maxCheckedNum = 0;
        for (int i = 0; i < n; i++) {
            double num = checkedArray[i];
            if (GRE(num, 0)) {
                // judge unbound
                bool flag = true;
                for (int j = 0; j < m; j++) {
                    double aji = a[j][i];
                    if (GRE(aji, 0)) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    // All numbers in column i are <= 0: unbound!
                    k = 0;
                    return;
                }
                // get max index
                if (GRE(num, maxCheckedNum)) {
                    inIndex = i;
                    maxCheckedNum = num;
                }
            }
        }
        if (inIndex == -1) {
            // all of checked numbers are <= 0
            // judge manual variables
            for (int i = 0; i < m; i++) {
                if (baseIndex[i] >= nonManualVariableCount && b[i][0] != 0) {
                    // no feasible solution!
                    k = -1;
                    return;
                }
            }
            // output answer
            k = 1;
            for (int i = 0; i < m; i++) {
                int base = baseIndex[i];
                x[0][base] = b[i][0];
            }
            y = value;
            return;
        }
        int outIndex = -1;
        int minOutIndex = INT_MAX;
        double minRatio = 0;
        for (int i = 0; i < m; i++) {
            double aik = a[i][inIndex];
            if (GEQ(0, aik)) continue;
            double ratio = b[i][0] / aik;
            if (outIndex == -1 || GRE(minRatio, ratio) || (equal(minRatio, ratio) && baseIndex[i] < minOutIndex)) {
                minRatio = ratio;
                outIndex = i;
                minOutIndex = baseIndex[i];
            }
        }
        // outIndex == -1 is impossible, due to this case is unbound!
        exchange(inIndex, outIndex);

#ifdef DEBUG
        printSimplexTable();
#endif
    }
}

#ifdef DEBUG

void BigMSolver::printSimplexTable() {
    printf("============  Simplex Table  ===========\n    ");
    for (int i = 0; i < n; i++) {
        printf(DOUBLE_FORMAT, c[0][i]);
    }
    printf(DOUBLE_FORMAT, value);
    printf("\n");

    for (int i = 0; i < m; i++) {
        printf("x%-3d", baseIndex[i]);
        for (int j = 0; j < n; j++) {
            printf(DOUBLE_FORMAT, a[i][j]);
        }
        printf(DOUBLE_FORMAT, b[i][0]);
        printf("\n");
    }
}

#endif