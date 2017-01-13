#include <stdlib.h>
#include <math.h>

typedef struct {
	size_t rows;
	size_t cols;
	float *data;
} f_matrix;

f_matrix *createSquareMatrix(size_t size);

f_matrix *createMatrix(size_t rows, size_t columns);

f_matrix *copyMatrix(f_matrix *m);

void destroyMatrix(f_matrix *m);

//Fills as many cells at m[p][p] as possible if m is not a square matrix
void makeIdentity(f_matrix *m);

//Fills as many cells at m[p][p] as possible if m is not a square matrix
void makeDiagonal(f_matrix *m, float val);
	
void setMatrixValue(f_matrix *m, size_t row, size_t col, float val);

float getMatrixValue(f_matrix *m, size_t row, size_t col);

void multMatrixValue(f_matrix *m, size_t row, size_t col, float f);

void addMatrixValue(f_matrix *m, size_t row, size_t col, float q);

typedef enum {PURE_MULT, DESTRUCTIVE_MULT_A, DESTRUCTIVE_MULT_B} MULT_MODE;

//Returns NULL on failure
//PURE_MULT -> Preserves both arguments, returns a newly allocated matrix (with createMatrix)
//DESTRUCTIVE_MULT -> If successful, destructively alters the right side matrix/the second argument, with the return value being the pointer passed in the 2nd arg
//result is a * b
//invalidates references to b->data when in DESTRUCTIVE_MULT mode (actually, no it doesn't, since it uses realloc, but it's safer to assume so)
f_matrix *multMatrix(f_matrix *a, f_matrix *b, MULT_MODE mode);

/*Translation, rotation and scale return, as of now, 4x4 matrices*/
f_matrix *translationMatrix(float x, float y, float z);

f_matrix *scaleMatrix(float s);

#define PI acos(-1.0)

//Works in degrees
f_matrix *rotateXMatrix(float theta);

//Works in degrees
f_matrix *rotateYMatrix(float theta);

//Works in degrees
f_matrix *rotateZMatrix(float theta);

float degreesOf(float rad);

float radiansOf(float deg);

//Dumps the contents of a matrix
void DEBUG_matrix_dump(f_matrix *m);

//Dumps the data (array) in a matrix
void DEBUG_data_dump(f_matrix *m);

typedef struct {
	size_t size;
	float *data;
} f_vec;

f_vec *createVec(size_t size);

void destroyVec(f_vec *v);

f_vec *copyVec(f_vec *v);

void setVecValue(f_vec *v, size_t index, float val);

void addVecValue(f_vec *v, size_t index, float val);

void multVecValue(f_vec *v, size_t index, float val);

float getVecValue(f_vec *v, size_t index);

//A - B
//returns NULL on error
f_vec *subtractVec(f_vec *a, f_vec *b);

f_vec *crossProduct(f_vec *a, f_vec *b);

//returns 0 on error
float dotProduct(f_vec *a, f_vec *b);

f_vec *normalizeVec(f_vec *v);

int vecEqual(f_vec *a, f_vec *b);

f_matrix *lookAt(f_vec *eye, f_vec *at, f_vec *up);

//returns NULL on error
//don't forget we consider that z points INTO the screen when using this function - therefore near < far
f_matrix *ortho(float l, float r, float b, float t, float n, float f);

//Dumps the contents of a vector
void DEBUG_vector_dump(f_vec *v);
