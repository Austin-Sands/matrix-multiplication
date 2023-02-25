#ifndef MATRIX_HH
#define MATRIX_HH
#include <string>
using namespace std;

double** allocate2d(int rows, int cols);
void free2d(double** x);

double** read2d(string filename, int& nr, int& nc);
void write2d(string filename, double** x, int nr, int nc);

void print2d(string msg, double** x, int nr, int nc);

#endif