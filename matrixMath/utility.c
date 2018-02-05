#include <stdio.h>
#include "utility.h"

void print_array2D(int rows, int cols, int a[rows][cols])
{
	for(int r = 0; r < rows; ++r){
		for(int c = 0; c < cols; ++c){
			printf("%5d", a[r][c]);
		}
		puts("");		// start each row on a new line
	}
}