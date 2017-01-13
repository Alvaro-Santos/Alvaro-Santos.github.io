#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "opengl_math.h"

f_matrix *createSquareMatrix(size_t size) {
	return createMatrix(size, size);
}

f_matrix *createMatrix(size_t rows, size_t columns) {
	f_matrix *m = malloc(sizeof(f_matrix));

	m->rows = rows;
	m->cols = columns;
	m->data = calloc(rows * columns, sizeof(float));
	
	return m;
}

f_matrix *copyMatrix(f_matrix *m) {
	f_matrix *r = createMatrix(m->rows, m->cols);
	
	memcpy(r->data, m->data, m->rows * m->cols * sizeof(float));
	
	return r;
}

void destroyMatrix(f_matrix *m) {
	free(m->data);
	free(m);
}

//Fills as many cells at m[p][p] as possible if m is not a square matrix
void makeIdentity(f_matrix *m) {
	makeDiagonal(m, 1.0f);
}

//Fills as many cells at m[p][p] as possible if m is not a square matrix
void makeDiagonal(f_matrix *m, float val) {
	size_t size = (m->cols < m->rows ? m->cols : m->rows);	//minimum
	size_t i;
	
	for(i = 0; i < size; ++i) {
		setMatrixValue(m, i, i, 1.0f);
	}
}

void setMatrixValue(f_matrix *m, size_t row, size_t col, float val) {
	*(m->data + col*m->rows + row) = val;
}

float getMatrixValue(f_matrix *m, size_t row, size_t col) {
	return *(m->data + col*m->rows + row);
}

void multMatrixValue(f_matrix *m, size_t row, size_t col, float f) {
	setMatrixValue(m, row, col, getMatrixValue(m, row, col) * f);
}

void addMatrixValue(f_matrix *m, size_t row, size_t col, float q) {
	setMatrixValue(m, row, col, getMatrixValue(m, row, col) + q);
}

//Returns NULL on failure
//PURE_MULT -> Preserves both arguments, returns a newly allocated matrix (with createMatrix)
//DESTRUCTIVE_MULT_B -> If successful, destructively alters the right side matrix/the second argument, with the return value being the pointer passed in the 2nd arg
//result is a * b
//invalidates references to b->data when in DESTRUCTIVE_MULT mode (actually, no it doesn't, since it uses realloc, but it's safer to assume so)
//DESTRUCTIVE_MULT_A -> same as B, but for the first matrix
f_matrix *multMatrix(f_matrix *a, f_matrix *b, MULT_MODE mode) {
	if(a->cols != b->rows) {
		return NULL;
	}
	
	float data[a->rows*b->cols];
	f_matrix result;
	result.rows = a->rows;
	result.cols = b->cols;
	result.data = data;
	
	size_t a_r, b_c, i, shared_dim = a->cols;
	for(a_r = 0; a_r < a->rows; ++a_r) {
		for(b_c = 0; b_c < b->cols; ++b_c) {
			float val = 0.0f;
			
			for(i = 0; i < shared_dim; ++i) {
				float v1 = getMatrixValue(a, a_r, i);
				float v2 = getMatrixValue(b, i, b_c);
				
				val += (v1 * v2);
			}
			
			setMatrixValue(&result, a_r, b_c, val);
		}
	}

	f_matrix *m;
		
	switch(mode) {
		case PURE_MULT:
			m = createMatrix(a->rows, b->cols);
			
			break;
		case DESTRUCTIVE_MULT_A:
			if(a->cols != b->cols) { //|| a->rows != a->rows
				a->cols = b->cols;
				a->data = realloc(a->data, a->rows * a->cols * sizeof(float));
			}
			
			m = a;
			
			break;
		case DESTRUCTIVE_MULT_B:
			if(b->rows != a->rows) { //|| b->cols != b->cols
				b->rows = a->rows;
				//free(b->data);
				//b->data = malloc(b->rows * b->cols * sizeof(float));
				
				b->data = realloc(b->data, b->rows * b->cols * sizeof(float));
			}
			
			m = b;
			
			break;
		default:
			return NULL;
	}
	
	memcpy(m->data, result.data, m->rows * m->cols * sizeof(float));
	return m;
}

/*Translation, rotation and scale return, as of now, 4x4 matrices*/
f_matrix *translationMatrix(float x, float y, float z) {
	size_t const matrix_dim = 4;
	
	f_matrix *m = createSquareMatrix(matrix_dim);
	makeIdentity(m);
	
	setMatrixValue(m, 0, 3, x);
	setMatrixValue(m, 1, 3, y);
	setMatrixValue(m, 2, 3, z);
	
	return m;	
}

f_matrix *scaleMatrix(float s) {
	size_t const matrix_dim = 4;
	
	f_matrix *m = createSquareMatrix(matrix_dim);
	
	setMatrixValue(m, 0, 0, s);
	setMatrixValue(m, 1, 1, s);
	setMatrixValue(m, 2, 2, s);
	setMatrixValue(m, 3, 3, 1.0f);
	
	return m;	
}

//Works in degrees
f_matrix *rotateXMatrix(float theta) {
	size_t const matrix_dim = 4;
	
	theta = radiansOf(theta);
	
	f_matrix *m = createSquareMatrix(matrix_dim);
	
	setMatrixValue(m, 0, 0, 1.0f);
	
	setMatrixValue(m, 1, 1, cos(theta));
	setMatrixValue(m, 1, 2, -sin(theta));
	
	setMatrixValue(m, 2, 1, sin(theta));
	setMatrixValue(m, 2, 2, cos(theta));
	
	setMatrixValue(m, 3, 3, 1.0f);
	
	return m;	
}

//Works in degrees
f_matrix *rotateYMatrix(float theta) {
	size_t const matrix_dim = 4;
	
	theta = radiansOf(theta);
	
	f_matrix *m = createSquareMatrix(matrix_dim);
	
	setMatrixValue(m, 0, 0, cos(theta));
	setMatrixValue(m, 0, 2, sin(theta));
	
	setMatrixValue(m, 1, 1, 1.0f);
	
	setMatrixValue(m, 2, 0, -sin(theta));
	setMatrixValue(m, 2, 2, cos(theta));
	
	setMatrixValue(m, 3, 3, 1.0f);
	
	return m;	
}

//Works in degrees
f_matrix *rotateZMatrix(float theta) {
	size_t const matrix_dim = 4;
	
	theta = radiansOf(theta);
	
	f_matrix *m = createSquareMatrix(matrix_dim);
	
	setMatrixValue(m, 0, 0, cos(theta));
	setMatrixValue(m, 0, 1, -sin(theta));
	
	setMatrixValue(m, 1, 0, sin(theta));
	setMatrixValue(m, 1, 1, cos(theta));
	
	setMatrixValue(m, 2, 2, 1.0f);
	
	setMatrixValue(m, 3, 3, 1.0f);
	
	return m;	
}

float degreesOf(float rad) {
	return (rad*180)/PI;
}

float radiansOf(float deg) {
	return (deg*PI)/180;
}

//Dumps the contents of a matrix
void DEBUG_matrix_dump(f_matrix *m) {
	size_t c, r;
	
	printf("Matrix:\n");
	for(r = 0; r < m->rows; ++r) {
		printf("\t");
		for(c = 0; c < m->cols; ++c) {
			float result = getMatrixValue(m, r, c);
			
			printf("%f ", result);
		}
		printf("\n");
	}
	printf("\n");
}

//Dumps the data (array) in a matrix
void DEBUG_data_dump(f_matrix *m) {
	printf("Matrix data:\n");
	size_t i;
	for(i = 0; i < m->cols * m->rows; ++i) {
		printf("%f ", *(m->data + i));
	}
	printf("\n\n");
}

f_vec *createVec(size_t size) {
	f_vec *v = malloc(sizeof(f_vec));

	v->size = size;
	v->data = calloc(size, sizeof(float));
	
	return v;
}

void destroyVec(f_vec *v) {
	free(v->data);
	free(v);
}

f_vec *copyVec(f_vec *v) {
	f_vec *n = createVec(v->size);
	
	memcpy(n->data, v->data, v->size * sizeof(float));
	
	return n;
}

void setVecValue(f_vec *v, size_t index, float val) {
	*(v->data + index) = val;
}

void addVecValue(f_vec *v, size_t index, float val) {
	*(v->data + index) += val;
}

void multVecValue(f_vec *v, size_t index, float val) {
	*(v->data + index) *= val;
}

float getVecValue(f_vec *v, size_t index) {
	return *(v->data + index);
}

//A - B
//returns NULL on error
f_vec *subtractVec(f_vec *a, f_vec *b) {
	if(a->size != b->size) {
		return NULL;
	}
	
	f_vec *v = copyVec(a);
	
	size_t i;
	for(i = 0; i < v->size; ++i) {
		addVecValue(v, i, -getVecValue(b, i));
	}
	
	return v;
}

//Only works for vectors of size 3
//returns NULL on error
f_vec *crossProduct(f_vec *a, f_vec *b) {
	if(a->size != b->size || a->size != 3) {
		return NULL;
	}
	
	f_vec *v = createVec(a->size);
	
	float x, y, z;
	
	x = getVecValue(a, 1) * getVecValue(b, 2) - getVecValue(a, 2) * getVecValue(b, 1);
	y = getVecValue(a, 2) * getVecValue(b, 0) - getVecValue(a, 0) * getVecValue(b, 2);
	z = getVecValue(a, 0) * getVecValue(b, 1) - getVecValue(a, 1) * getVecValue(b, 0);
	
	setVecValue(v, 0, x);
	setVecValue(v, 1, y);
	setVecValue(v, 2, z);
	
	return v;
}

//returns 0 on error
float dotProduct(f_vec *a, f_vec *b) {
	if(a->size != b->size) {
		return 0.0f;
	}
	
	float total = 0.0f;
	
	size_t i;
	for(i = 0; i < a->size; ++i) {
		float val = getVecValue(a, i) * getVecValue(b, i);
		total += val;
	}
	
	return total;
}

f_vec *normalizeVec(f_vec *v) {
	float vecLength = sqrt(dotProduct(v, v));
	
	f_vec *result = copyVec(v);
	
	size_t i;
	for(i = 0; i < result->size; ++i) {
		multVecValue(result, i, 1.0f/vecLength);
	}
	
	return result;
}	

int vecEqual(f_vec *a, f_vec *b) {
	if(a->size != b->size) {
		return 0;
	}
	
	size_t i;
	for(i = 0; i < a->size; ++i) {
		if(getVecValue(a, i) != getVecValue(b, i)) {
			return 0;
		}
	}
	
	return 1;
}

f_matrix *lookAt(f_vec *eye, f_vec *at, f_vec *up) {
	if(eye->size != 3 || at->size != 3 || up->size != 3) {
		return NULL;
	}
	
	f_matrix *m = createSquareMatrix(4);
	
	if(vecEqual(eye, at)) {
		makeIdentity(m);
		
		return m;
	}
	
	f_vec *temp1 = subtractVec(eye, at);
	f_vec *n = normalizeVec(temp1);
	f_vec *temp2 = crossProduct(up, n);
	f_vec *u = normalizeVec(temp2);
	f_vec *temp3 = crossProduct(n, u);
	f_vec *v = normalizeVec(temp3);
	float doteyeu = dotProduct(eye, u);
	float doteyev = dotProduct(eye, v);
	float doteyen = dotProduct(eye, n);
	
	setMatrixValue(m, 0, 0, getVecValue(u, 0));
	setMatrixValue(m, 0, 1, getVecValue(u, 1));
	setMatrixValue(m, 0, 2, getVecValue(u, 2));
	setMatrixValue(m, 0, 3, -doteyeu);
	
	setMatrixValue(m, 1, 0, getVecValue(v, 0));
	setMatrixValue(m, 1, 1, getVecValue(v, 1));
	setMatrixValue(m, 1, 2, getVecValue(v, 2));
	setMatrixValue(m, 1, 3, -doteyev);
	
	setMatrixValue(m, 2, 0, getVecValue(n, 0));
	setMatrixValue(m, 2, 1, getVecValue(n, 1));
	setMatrixValue(m, 2, 2, getVecValue(n, 2));
	setMatrixValue(m, 2, 3, -doteyen);
	
	setMatrixValue(m, 3, 3, 1.0f);
	
	destroyVec(temp1);
	destroyVec(n);
	destroyVec(temp2);
	destroyVec(u);
	destroyVec(temp3);
	destroyVec(v);
	
	return m;
}

//returns NULL on error
//don't forget we consider that z points INTO the screen when using this function - therefore near < far
f_matrix *ortho(float l, float r, float b, float t, float n, float f) {
	if(l == r || b == t || n == f) {
		return NULL;
	}
	
	float w = r - l;
	float h = t - b;
	float d = f - n;
	
	f_matrix *result = createSquareMatrix(4);
	
	setMatrixValue(result, 0, 0, 2.0f/w);
	setMatrixValue(result, 1, 1, 2.0f/h);
	setMatrixValue(result, 2, 2, -2.0f/d);
	
	setMatrixValue(result, 0, 3, -(l + r)/w);
	setMatrixValue(result, 1, 3, -(t + b)/h);
	setMatrixValue(result, 2, 3, -(n + f)/d);
	
	setMatrixValue(result, 3, 3, 1.0f);
	
	return result;
}

//Dumps the contents of a vector
void DEBUG_vector_dump(f_vec *v) {
	printf("Vector:\n");
	size_t i;
	for(i = 0; i < v->size; ++i) {
		printf("%f ", *(v->data + i));
	}
	printf("\n\n");
}
	
	