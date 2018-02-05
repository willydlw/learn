#include <stdio.h>
#include <stdlib.h>				// rand 

#include "matrix.h"
#include "utility.h"

#define MAX_RANDOM_VALUE 50
#define MIN_RANDOM_VALUE 1



int main()
{
	// create two square matrices with the same dimensions
	int matrixA[3][3], matrixB[3][3], matrixC[3][3];


    // fill the matrices with random numbers in the range [1, 50]
    for(int r = 0; r < 3; ++r){
    	for(int c = 0; c < 3; ++c){
    		matrixA[r][c] = rand()%(MAX_RANDOM_VALUE - MIN_RANDOM_VALUE + 1) + MIN_RANDOM_VALUE;
    		matrixB[r][c] = rand()%(MAX_RANDOM_VALUE - MIN_RANDOM_VALUE + 1) + MIN_RANDOM_VALUE;
    	}
    }

    printf("\nMatrix A\n");
    print_array2D(3, 3, matrixA);

    printf("\nMatrix B\n");
    print_array2D(3, 3, matrixB);

    matrix_add(3, 3, matrixA, matrixB, matrixC);
    printf("\nMatrix C = A + B\n");
    print_array2D(3, 3, matrixC);


	return 0;
}