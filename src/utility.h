#ifndef UTILITY_H
#define UTILITY_H

#define ITEM_FORMAT "%8.2fx%-3d"
#define ITEM_NO_COEFFICIENT_FORMAT "        x%-3d"
#define DOUBLE_FORMAT "%8.2f"

#define GRE(x, y) (x > y && !equal(x, y))
#define GEQ(x, y) (x > y || equal(x, y))

// comment this in release!
//#define DEBUG
//#define PARALLEL
#define SIMPLEX_BIG_M
//#define SIMPLEX_DOUBLE_STAGE
//#define DUAL

const double EPS = 1e-5;

bool equal(double a, double b);
void printSign(double flag);
void printDouble(double x);
void printItem(double coefficient, int index);
void printConstraint(int index, double flag);

#endif