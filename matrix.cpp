#include <iostream>
#include <fstream>
#include "matrix.h"
using namespace std;

double** allocate2d(int rows, int cols)
{
    double** data;
    if (rows == 0 || cols == 0)
        data = nullptr;
    else {
        data = new double* [rows];
        double* temp = new double[rows * cols];
        for (size_t i = 0; i < rows; i++)
            data[i] = temp + i * cols;
    }
    return data;
}

void free2d(double** x)
{
    if (x != nullptr) {
        delete[] x[0];
        delete[] x;
    }
}

double** read2d(string filename, int& nr, int& nc)
{
    ifstream fin;
    fin.open(filename, ios::binary);
    fin.read((char*)&nr, sizeof(int));
    fin.read((char*)&nc, sizeof(int));
    double** x = allocate2d(nr, nc);
    fin.read((char*)x[0], sizeof(double) * nr * nc);
    fin.close();
    return x;
}

void write2d(string filename, double** x, int nr, int nc)
{
    ofstream fout;
    fout.open(filename, ios::binary);
    fout.write((char*)&nr, sizeof(int));
    fout.write((char*)&nc, sizeof(int));
    fout.write((char*)x[0], sizeof(double) * nr * nc);
    fout.close();
}

void print2d(string msg, double** x, int nr, int nc)
{
    if (msg.length() > 0)
        cout << msg;
    cout << "(" << nr << "*" << nc << ")" << endl;

    for (int i = 0; i < nr; i++) {
        if (nr > 8 && i == 6) {
            cout << "...\n";
            i = nr - 2;
            continue;
        }

        if (nc < 9) {
            for (int j = 0; j < nc; j++)
                cout << x[i][j] << ((j + 1 == nc || (j + 1) % 8 == 0) ? "\n" : " ");
        }
        else {
            for (int j = 0; j < 6; j++)
                cout << x[i][j] << " ";
            cout << "... " << x[i][nc - 1] << "\n";
        }
    }
}