/*
File:	Project2.cpp
Author:	Austin Sands
Date:	02/20/2023
Desc:	This program will use std::thread to multiply two matrices and calculate the sum of all elements,
	mean of elements, and std. deviation of elements in resultant matrix
 */

#include "matrix.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <cmath>
using namespace std;

static const int MATRIX_SETTING = 0; //change to 0 for 1024 x 1024 matrices, 1 for 2048 x 2048 matrices
static const int MAX_THREADS = 8; //change amount of threads to be created

//this function handles the thread execution and handling. Takes in three matrices, the two being multiplied and a destination matrix, number of rows followed by columns, and the arrays that hold thread stats
void threadMultiplication(double** matrixA, double** matrixB, double** resultMatrix, int nRows, int nCols, double* threadSumArray, double* threadAvgArray, double* threadDevArray);
//this function handles the matrix multiplication operations as well as computation of thread sum, avg, and std dev. Takes in three matrices the two being multiplied and a destination matrix,
//number of rows followed by columns, and the arrays that hold thread stats
void multiplyMat(double** matrixA, double** matrixB, double** resultMatrix, int threadIndex, int nRows, int nCols, double* threadSumArray, double* threadAvgArray, double* threadDevArray);
//this function simply prints out stats at the end of the program execution. Takes number of rows followed by columns, microseconds variable, arrays that hold thread stats, and the variables that hold accumulative stats
void printStats(int nRows, int nCols, chrono::microseconds totalTime, double* threadSumArray, double* threadAvgArray, double* threadDevArray, double totalSum, double totalAvg, double totalStdDev);

int main()
{
	int nr, nc;
	double totalSum = 0, totalAvg = 0, devSum = 0, totalStdDev = 0;
	double threadSum[MAX_THREADS], threadAvg[MAX_THREADS], threadDev[MAX_THREADS]; //arrays to hold the thread stats outside of their scope
	double **xa, **xb, **xc;

	//can easily switch between 1024 matrices and 2048 by changing parameter. 0 = 1024 matrices and 1 = 2048 matrices 
	if (MATRIX_SETTING == 0) {
		xa = read2d("a.mat", nr, nc); // read a 2d matrix, dimensions are returned as nr and nc
		xb = read2d("b.mat", nr, nc);
	}
	else
	{
		xa = read2d("c.mat", nr, nc); // read a 2d matrix, dimensions are returned as nr and nc
		xb = read2d("d.mat", nr, nc);
	}
	xc = allocate2d(nr, nc); 	// allocate dynamic memory for nr*nc matrix

	//initialize all values to zero
	for (int i = 0; i < nr; i++)
		for (int j = 0; j < nc; j++)
			xc[i][j] = 0;

	auto startTime = chrono::high_resolution_clock::now(); //save start time for calculating process time

	//handle threads for matrix multiplication
	threadMultiplication(xa, xb, xc, nr, nc, threadSum, threadAvg, threadDev);

	//calculate stats based on thread computations
	for(int i = 0; i < MAX_THREADS; i++)
	{
		totalSum += threadSum[i];
		totalAvg += threadAvg[i];
	}
	totalAvg /= MAX_THREADS;

	//after calculating total avg we can calculate combined std dev
	for(int i = 0; i < MAX_THREADS; i++)
	{
		devSum +=  pow(threadDev[i], 2) + pow(threadAvg[i] - totalAvg, 2);
	}
	totalStdDev = sqrt(devSum / MAX_THREADS);

	auto endTime = chrono::high_resolution_clock::now(); //save end time for calculating process time
	auto totalTime = chrono::duration_cast<chrono::microseconds>(endTime - startTime); //get total process time in microseconds

	cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

	print2d("\nxa", xa, nr, nc); 	// print the matrix to screen
	print2d("\nxb", xb, nr, nc);
	print2d("\nxc", xc, nr, nc);

	cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
	cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

	//print process stats
	printStats(nr, nc, totalTime, threadSum, threadAvg, threadDev, totalSum, totalAvg, totalStdDev);

	free2d(xa); 			// deallocate the dynamic memory
	free2d(xb);
	free2d(xc);
	return 0; 
}

void threadMultiplication(double** matrixA, double** matrixB, double** resultMatrix, int nRows, int nCols, double* threadSumArray, double* threadAvgArray, double* threadDevArray)
{
	thread workThreads[MAX_THREADS]; //create array of threads

	cout << "Starting multiplication threads...\n";
	for (int i = 0; i < MAX_THREADS; i++)
	{
		workThreads[i] = thread(multiplyMat, matrixA, matrixB, resultMatrix, i, nRows, nCols, threadSumArray, threadAvgArray, threadDevArray); //start threads to multiply matrices
	}

	for (int i = 0; i < MAX_THREADS; i++)
	{
		workThreads[i].join(); //wait for threads to finish and join
	}
	cout << "Multiplication threads stopped...\n";
}

void multiplyMat(double** matrixA, double** matrixB, double** resultMatrix, int threadIndex, int nRows, int nCols, double* threadSumArray, double* threadAvgArray, double* threadDevArray)
{
	int firstRow = threadIndex * (nRows / MAX_THREADS); //calculate starting row for thread based on number of thread (index in thread array)
	int lastRow = min((threadIndex + 1) * (nRows / MAX_THREADS), nRows); //calculate last row to compute based on number of thread
	double threadElements = (lastRow - firstRow) * nCols; //get total number of elements in thread
	double threadSum = 0; //variable to track sum of elements in resultant matrix in thread
	double devSum = 0; //variable to track sum of deviations for std. dev calculation

	for (int currentRow = firstRow; currentRow < lastRow; currentRow++) //loop through rows in matrix A
		for (int currentCol = 0; currentCol < nCols; currentCol++) //loop through columns in matrix B
		{
			for (int currentElement = 0; currentElement < nCols; currentElement++) //loop through elements in Matrix A row and Matrix B column 
			{
				resultMatrix[currentRow][currentCol] += matrixA[currentRow][currentElement] * matrixB[currentElement][currentCol];
			}

			threadSum += resultMatrix[currentRow][currentCol]; //while calculating resultant matrix calculate sum of resultant elements in thread
		}

	threadSumArray[threadIndex] =  threadSum; //add thread sum to total elements sum

	double threadAvg = threadSum / threadElements; //calculate thread mean
	threadAvgArray[threadIndex] = threadAvg; //save thread mean to array of means

	//calculate thread sample standard deviation
	for (int currentRow = firstRow; currentRow < lastRow; currentRow++)
		for (int currentElement = 0; currentElement < nCols; currentElement++)
			devSum += pow((resultMatrix[currentRow][currentElement] - threadAvg), 2);

	threadDevArray[threadIndex] = sqrt(devSum / (threadElements - 1));
}

void printStats(int nRows, int nCols, chrono::microseconds totalTime, double* threadSumArray, double* threadAvgArray, double* threadDevArray, double totalSum, double totalAvg, double totalStdDev)
{
	cout << "\nThread stats: \n";
	cout << "=======================================================================\n";
	cout << "Number of threads: " << MAX_THREADS << "\tMatrix Size: " << nRows << " X " << nCols << endl;
	for (int i = 0; i < MAX_THREADS; i++)
	{
		cout << "Thread " << i << " sum: " << threadSumArray[i] << "\tAvg: " << threadAvgArray[i] << "\tStd. Dev: " << threadDevArray[i] << endl;
	}
	cout << "=======================================================================\n";
	cout << "Total Sum: " << totalSum << "\tTotal Avg: " << totalAvg << "\tTotal Std. Dev: " << totalStdDev << endl;
	cout << "Total process time: " << totalTime.count() << " microseconds\n";
}
