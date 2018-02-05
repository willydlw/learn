#include "matrix.h"

/** 
* matrices may only be added together if they have the same dimensions
*	same number of rows, same number of cols
*/
void matrix_add(int rows, int cols, int a[rows][cols], int b[rows][cols], int c[rows][cols])
{
	for(int i = 0; i < rows; ++i){
		for(int j = 0; j < cols; ++j){
			c[i][j] = a[i][j] + b[i][j];
		}
	}
}