#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// created for sending params to threads in multithreading backtracker solver function
struct params {
	int size;
	int grid[36][36];
};

// created for storing the possible entries for each cell as a part of optmisation
struct cell {
	int value;
	int possibleValues[37];
};

// counting backtracks and changes made in optmisation for benchmarking the results
int backtracks = 0;
int changes = 0;
int solved = 0;

// given read function from skeleton
void read_grid_from_file(int size, char *ip_file, int grid[36][36]) {
	FILE *fp;
	int i, j;
	fp = fopen(ip_file, "r");
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++) {
			fscanf(fp, "%d", &grid[i][j]);
		}
}

// given print function from skeleton
void print_grid(int size, int grid[36][36]) {
	int i, j;
	/* The segment below prints the grid in a standard format. Do not change */
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++)
			printf("%d\t", grid[i][j]);
		printf("\n");
	}
}

// used for checking if the given entry is valid for the optimisation matrix sudoku
int isPositionValidOpt(struct cell optSudoku[36][36], int x, int y, int possibleNumber, int size) {

	// check if the possibleNumber is present in the column
	for (int i = 0; i < size; i++) {
		if (optSudoku[i][y].value == possibleNumber) {
			return 0;
		}
	}

	// check if the possibleNumber is present in the row
	for (int i = 0; i < size; i++) {
		if (optSudoku[x][i].value == possibleNumber) {
			return 0;
		}
	}

	int k = sqrt(size);
	int startx = x - x % k;
	int starty = y - y % k;

	// check if the possibleNumber is present in the box
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < k; j++) {
			if (optSudoku[i + startx][j + starty].value == possibleNumber) {
				return 0;
			}
		}
	}

	// return 1 if the possibleNumber is a valid Number
	return 1;
}

// used for checking if the given entry is valid for the normal sudoku
int isPositionValid(int grid[36][36], int x, int y, int possibleNumber, int size) {

	// check if the possibleNumber is present in the column
	for (int i = 0; i < size; i++) {
		if (grid[i][y] == possibleNumber) {
			return 0;
		}
	}

	// check if the possibleNumber is present in the row
	for (int i = 0; i < size; i++) {
		if (grid[x][i] == possibleNumber) {
			return 0;
		}
	}

	int k = sqrt(size);
	int startx = x - x % k;
	int starty = y - y % k;

	// check if the possibleNumber is present in the box
	for (int i = 0; i < k; i++) {
		for (int j = 0; j < k; j++) {
			if (grid[i + startx][j + starty] == possibleNumber) {
				return 0;
			}
		}
	}

	// return 1 if the possibleNumber is a valid Number
	return 1;
}

void print_gridOpt(int size, struct cell optSudoku[36][36]) {
	int i, j;
	/* The segment below prints the grid in a standard format. Do not change */
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++)
			printf("%d\t", optSudoku[i][j].value);
		printf("\n");
	}
}

// count cells with single possible entry which act as naked single cells
int countSingleEntriedCells(struct cell optSudoku[36][36], int size) {
	// count if the entries are only single
	int counter  = 0;
	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			if (optSudoku[k][l].value == 0) {
				// each grid item cell
				int count = 0;
				for (int m = 1; m <= size; ++m) {
					if (optSudoku[k][l].possibleValues[m] != 0) {
						count++;
					}
				}
				if (count == 1) {
					counter++;
				}
			}

		}
	}
	return counter;
}

// count the empty cells in matrix used to assert the completion of solving the sudoku
int countEmptyCells(int grid[36][36], int size) {
	int counter  = 0;

	for (int k = 0; k < size; k++) {
		for (int l = 0; l < size; l++) {
			if (grid[k][l] == 0) {
				counter++;
			}
		}
	}
	return counter;
}

// used for getting the indices of naked cell
void find_single_cell(struct cell optSudoku[36][36], int size, int *x, int *y) {
	for (int k = *x; k < size; k++) {
		for (int l = *y; l < size; l++) {
			if (optSudoku[k][l].value == 0) {
				// each grid item cell
				int count = 0;
				for (int m = 1; m <= size; ++m) {
					if (optSudoku[k][l].possibleValues[m] != 0) {
						count++;
					}
				}
				if (count == 1) {
					*x = k;
					*y = l;
					return;
				}
			}
		}
	}
	return;
}


void find_hidden_cell(struct cell optSudoku[36][36], int size, int *x, int *y, int *entry) {
	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			if (optSudoku[k][l].value == 0) {
				for (int possible = 1; possible <= size; ++possible) {
					if (optSudoku[k][l].possibleValues[possible] != 0) {
						int ii = 0;
						int isNumberPresentInRow = 0;
						int isNumberPresentInCol = 0;
						int isNumberPresentInBox = 0;
						// check in that particular row
						for (int col = 0; col < size; ++col) {
							if (optSudoku[k][col].possibleValues[possible] != 0 && col != l) {
								// exists in the row
								isNumberPresentInRow = 1;
								break;
							}
						}

						// check in that particular col
						for (int row = 0; row < size; ++row) {
							if (optSudoku[row][l].possibleValues[possible] != 0 && row != k) {
								// exists in the row
								isNumberPresentInCol = 1;
								break;
							}
						}

						// check in that house
						int siz = sqrt(size);
						int startx = k - k % siz;
						int starty = l - l % siz;

						// check if the possibleNumber is present in the box
						for (int i = 0; i < siz; i++) {
							for (int j = 0; j < siz; j++) {
								if (optSudoku[i + startx][j + starty].possibleValues[possible] != 0 && ((i + startx != k) && (j + starty != l))) {
									isNumberPresentInBox = 1;
									break;
								}
							}
						}
						ii = isNumberPresentInRow + isNumberPresentInCol + isNumberPresentInBox;
						if(ii==1) {
							*x = k;
							*y = l;
							*entry = possible;
							return;
						}
						// if (isNumberPresentInRow == 0) {
						// 	*x = k;
						// 	*y = l;
						// 	*entry = possible;
						// 	break;
						// } else if (isNumberPresentInCol == 0) {
						// 	*x = k;
						// 	*y = l;
						// 	*entry = possible;
						// 	break;
						// } else if (isNumberPresentInBox == 0) {
						// 	*x = k;
						// 	*y = l;
						// 	*entry = possible;
						// 	break;
						// }
					}
				}
			}
		}
	}
	return;
}

// used for getting the index of next empty cell
void find_next_empty_cell(int grid[36][36], int size, int *x, int *y) {
	for (int k = *x; k < size; k++) {
		for (int l = *y; l < size; l++) {
			if (grid[k][l] == 0) {
				*x = k;
				*y = l;
				return;
			}
		}
	}
	return;
}

// used for getting the possible value in the naked cell at position k,l
int getEntry(struct cell optSudoku[36][36], int size, int k, int l) {
	int val  = 0;

	if (optSudoku[k][l].value == 0) {
		// each grid item cell
		for (int m = 1; m <= size; ++m) {
			if (optSudoku[k][l].possibleValues[m] != 0) {
				return m;
			}
		}
	}
	return -1;
}
// backtracker worker function for multithreaded approach
void *backtracker_multithread(void *args) {
	struct params *param;
	param = (struct params *)args;

	if (countEmptyCells(param->grid, param->size) == 0) {
		print_grid(param->size, param->grid);
		solved = 1;
		return 0;
	}

	int row = 0;
	int col = 0;
	find_next_empty_cell(param->grid, param->size, &row, &col);

	for (int i = 0; i <= param->size; i++) {
		if (isPositionValid(param->grid, row, col, i, param->size)) {
			param->grid[row][col] = i;

			struct params new_param;
			memcpy(new_param.grid, param->grid, sizeof(param->grid));
			new_param.size = param->size;

			backtracker_multithread(&new_param);
			if (solved) {
				return 0;
			}

			// backtracking coz the earlier choice didn't work well
			backtracks++;
			param->grid[row][col] = 0;
		}
	}

	return 0;
}

int countHiddenSingles(struct cell optSudoku[36][36], int size) {
	int counter = 0;
	// What is a hidden single ?
	// There might be more than 1 possible numbers for a given cell
	// But one particular entry might be the only one present in it's associated row, column / box
	// We'll count a cell as hidden single if it has a number not present in it's row / column / box
	// We'll check the row where the cell is, and find if that particular entry is present elsewhere
	// Loop through each of the row entry, and if there is a possible entry there, that means pE[e] = 1
	// so if it !=0, that means the row has the number
	// we change the isPresent = 1;
	// If isPresent = 1, that means the number is present in that particular block
	// But it doesn't signify that it is not present in row / col
	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			if (optSudoku[k][l].value == 0) {
				// int isNumberPresentInRow = 0;
				// int isNumberPresentInCol = 0;
				// int isNumberPresentInBox = 0;
				// int ii = 0;
				for (int possible = 1; possible <= size; ++possible) {
					if (optSudoku[k][l].possibleValues[possible] != 0) {
						int ii = 0;
						int isNumberPresentInRow = 0;
						int isNumberPresentInCol = 0;
						int isNumberPresentInBox = 0;
						// int e = possible;
						// check in that particular row
						for (int col = 0; col < size; ++col) {
							if (optSudoku[k][col].possibleValues[possible] != 0 && (col!=l)) {
								// exists in the row
								isNumberPresentInRow = 1;
								break;
							}
						}

						// check in that particular col
						for (int row = 0; row < size; ++row) {
							if (optSudoku[row][l].possibleValues[possible] != 0 && (row != k)) {
								// exists in the row
								isNumberPresentInCol = 1;
								break;
							}
						}

						// check in that house
						int siz = sqrt(size);
						int startx = k - k % siz;
						int starty = l - l % siz;

						// check if the possibleNumber is present in the box
						for (int i = 0; i < siz; i++) {
							for (int j = 0; j < siz; j++) {
								if (optSudoku[i + startx][j + starty].possibleValues[possible] != 0 && ((i + startx != k) && (j + starty != l))) {
									isNumberPresentInBox = 1;
									break;
								}
							}
						}
						ii = isNumberPresentInRow + isNumberPresentInCol + isNumberPresentInBox;
						if (ii == 1) {
							counter++;
							break;
						}
					}
				}
			}
		}
	}
	return counter;
}

void countTotalNumberOfEmptyCells(struct cell optSudoku[36][36], int size) {
	int zeroes = 0;
	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			if (optSudoku[k][l].value == 0) {
				zeroes++;
			}
		}
	}

	printf("[log] Total Number of Empty Cells: %d\n", zeroes);
	return;
}

int main(int argc, char *argv[]) {
	// Uncomment the below lines to start the timer
	clock_t time;
	time = clock();
	int grid[36][36], size;

	if (argc != 3) {
		printf("Usage: ./sudoku.out grid_size inputfile");
		exit(-1);
	}

	size = atoi(argv[1]);
	read_grid_from_file(size, argv[2], grid);

	// Optmising the sudoku initially based on the Naked Singles Optimisation method
	struct cell optSudoku[36][36];

	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			optSudoku[k][l].value = grid[k][l];
			for (int m = 0; m <= 36; ++m)
				optSudoku[k][l].possibleValues[m] = 0;
		}
	}

	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			if (optSudoku[k][l].value == 0) {
				for (int m = 1; m <= size; ++m) {
					if (isPositionValidOpt(optSudoku, k, l, m, size))
						optSudoku[k][l].possibleValues[m] = 1;
				}
			}
		}
	}
	while (countSingleEntriedCells(optSudoku, size) != 0) {
		int x = 0 ; int y = 0;
		find_single_cell(optSudoku, size, &x, &y);
		int e = getEntry(optSudoku, size, x , y);
		optSudoku[x][y].value =  e;
		optSudoku[x][y].possibleValues[e] = 0;
		changes++;
		// remove in the whole row
		for (int k = 0; k < size; ++k) {
			optSudoku[k][y].possibleValues[e] = 0;
		}

		// remove in col
		for (int k = 0; k < size; ++k) {
			optSudoku[x][k].possibleValues[e] = 0;
		}

		//remove in box
		int k = sqrt(size);
		int startx = x - x % k;
		int starty = y - y % k;

		// check if the possibleNumber is present in the box
		for (int i = 0; i < k; i++) {
			for (int j = 0; j < k; j++) {
				optSudoku[i + startx][j + starty].possibleValues[e] = 0;
			}
		}
	}
	// print_gridOpt(size, optSudoku);
	// printf("\n");
	while (countHiddenSingles(optSudoku, size) != 0) {
		int x = 0; int y = 0; int e = 0;
		find_hidden_cell(optSudoku, size, &x, &y, &e);
		optSudoku[x][y].value = e;
		for (int p = 1; p <= size; ++p) {
			optSudoku[x][y].possibleValues[p] = 0;
		}
		changes++;

		for (int k = 0; k < size; ++k) {
			optSudoku[k][y].possibleValues[e] = 0;
		}

		// remove in col
		for (int k = 0; k < size; ++k) {
			optSudoku[x][k].possibleValues[e] = 0;
		}

		//remove in box
		int k = sqrt(size);
		int startx = x - x % k;
		int starty = y - y % k;

		// check if the possibleNumber is present in the box
		for (int i = 0; i < k; i++) {
			for (int j = 0; j < k; j++) {
				optSudoku[i + startx][j + starty].possibleValues[e] = 0;
			}
		}
	}
// 	printf("\n");
// print_gridOpt(size, optSudoku);
	for (int k = 0; k < size; ++k) {
		for (int l = 0; l < size; ++l) {
			grid[k][l] = optSudoku[k][l].value;
		}
	}

	int row = 0;
	int col = 0;

	find_next_empty_cell(grid, size, &row, &col);

	int k;
	int possibleEntries[size];
	int index = 0;

	for (k = 1; k <= size; k++) {
		if (isPositionValid(grid, row, col,k,size)) {
			possibleEntries[index] = k;
			index++;
		}
	}

	pthread_t threads[index];
	pthread_t t;

	struct params params_array[index];

	for (int i = 0; i < index; i++) {
		grid[row][col] = possibleEntries[i];
		struct params new_param;
		memcpy(new_param.grid, grid, sizeof(grid));
		new_param.size = size;
		params_array[i] = new_param;
	}

	for (int th = 0; th < index; th++) {
		int result = pthread_create(&threads[th], NULL, backtracker_multithread, &params_array[th]);
	}

	for (int th = 0; th < index; th++) {
		int result = pthread_join(threads[th], NULL);
	}
	// printf("\n");
	// print_grid(size, grid);

	// Uncomment the below lines to get Debug Logs
	printf("[log]: Total Backtracks : %d\n", backtracks);
	printf("[log]: Total Changes because of optimisation : %d\n", changes);
	time = clock() - time;
	double time_taken = ((double)time) / CLOCKS_PER_SEC;
	printf("[log]: Code took %f seconds to execute\n", time_taken);
	// print_gridOpt(size, optSudoku);
	if(countEmptyCells(grid, size)==0) {
		print_grid(size, grid);
		return 1;
	}

	if (solved == 0) printf("No solution");
	return 0;
}
