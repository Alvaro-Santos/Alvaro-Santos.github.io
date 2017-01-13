#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "opengl_math.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

void render(void);

GLuint initShaders() {
	//Creating the vertex shader
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	//Note that these string literals will be automatically concatenated into a single one
	char const * const vtx_shd = "uniform mat4 mModel;"
								 "uniform mat4 mView;"
								 "uniform mat4 mProjection;"
								 "attribute vec3 vPosition;"
								 "void main() {"
									"gl_Position = mProjection * mView * mModel * vec4(vPosition, 1.0);"
								 "}";

	glShaderSource(vertex_shader, 1, &vtx_shd, NULL);
	glCompileShader(vertex_shader);

	//Creating the fragment shader
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	char const * const frg_shd = "uniform bool mode;"
								 "void main() {"
									 "if(mode) {"
										"gl_FragColor = vec4(0.62745098039, 0.55686274509, 0.09411764705, 1.0);"
									 "} else {"
										"gl_FragColor = vec4(0.3725490196, 0.0, 0.90588235294, 1.0);"
									 "}"
								 "}";

	glShaderSource(fragment_shader, 1, &frg_shd, NULL);
	glCompileShader(fragment_shader);

	//Creating the program
	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	return program;
}

GLint mModelLoc;
GLint mViewLoc;
GLint mProjectionLoc;
GLint modeLoc;

GLuint cube_vertex_buffer;
GLuint cube_element_buffer;
GLuint second_cube_element_buffer;

GLuint program;

float cube_vertices[3 * 8] = {-0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5,
							  -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5};

unsigned char cube_indices[8] = {0, 1, 2, 3, 7, 4, 5, 1};
unsigned char second_cube_indices[8] = {6, 2, 3, 7, 4, 5, 1, 2};

float horizontal_movement = 0.0;
float vertical_movement = 0.0;
float depth_movement = 0.0;
void keyboard(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_LEFT:
			horizontal_movement -= 0.01f;
			break;
		case GLUT_KEY_RIGHT:
			horizontal_movement += 0.01f;
			break;
		case GLUT_KEY_UP:
			vertical_movement += 0.01f;
			break;
		case GLUT_KEY_DOWN:
			vertical_movement -= 0.01f;
			break;
		case GLUT_KEY_PAGE_UP:
			depth_movement -= 0.01f;
			break;
		case GLUT_KEY_PAGE_DOWN:
			depth_movement += 0.01f;
			break;
	}
}

int main(int argc, char **argv) {
	//Window initialisation
    glutInit(&argc, argv);

	int const screen_width = glutGet(GLUT_SCREEN_WIDTH);
	int const screen_height = glutGet(GLUT_SCREEN_HEIGHT);

	//Centering the window
    glutInitWindowPosition((screen_width - WINDOW_WIDTH)/2, (screen_height - WINDOW_HEIGHT)/2);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    int window = glutCreateWindow("OpenGL on Windows");

	//Initiate GLEW
	glewInit();

	//Set up the program
	program = initShaders();
	glUseProgram(program);

	GLuint *buffer = malloc(sizeof(GLuint) * 3);
	glGenBuffers(3, buffer);
	cube_vertex_buffer = *buffer;
	cube_element_buffer = *(buffer+1);
	second_cube_element_buffer = *(buffer + 2);

	glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 8, cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned char) * 8, cube_indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, second_cube_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned char) * 8, second_cube_indices, GL_STATIC_DRAW);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	mModelLoc = glGetUniformLocation(program, "mModel");
	mViewLoc = glGetUniformLocation(program, "mView");
	mProjectionLoc = glGetUniformLocation(program, "mProjection");
	
	modeLoc = glGetUniformLocation(program, "mode");

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glutDisplayFunc(render);
	glutIdleFunc(render);

	glutSpecialFunc(keyboard);

	printf("\nIf you're seeing this message, it's because you tried compiling this program without -Wl,--subsystem,windows.\n"
		   "Good on you for trying different things out.\n\n");
	
    glutMainLoop();

	glutDestroyWindow(window);

    return 0;
}

void drawSurface(void);

void render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSurface();

	glutSwapBuffers();
}

void printPosition(void) {
	size_t i;
	
	glWindowPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
	char *msg1 = "Your eye is currently at";
	size_t msg1_len = strlen(msg1);
	for(i = 0; i < msg1_len; ++i) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *(msg1 + i));
	}
	
	glWindowPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - (20 + 13));
	char *msg2_formatter = "X (horizontal, pointing right) = %.2f";
	char msg2[strlen(msg2_formatter) - 4 + 4 + (horizontal_movement < 0 ? 1 : 0)];
	sprintf(msg2, msg2_formatter, horizontal_movement);
	size_t msg2_len = strlen(msg2);
	for(i = 0; i < msg2_len; ++i) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *(msg2 + i));
	}
	
	glWindowPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - (20 + 26));
	char *msg3_formatter = "Y (vertical, pointing up) = %.2f";
	char msg3[strlen(msg3_formatter) - 4 + 4 + (vertical_movement < 0 ? 1 : 0)];
	sprintf(msg3, msg3_formatter, vertical_movement);
	size_t msg3_len = strlen(msg3);
	for(i = 0; i < msg3_len; ++i) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *(msg3 + i));
	}
	
	glWindowPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - (20 + 39));
	char *msg4_formatter = "Z (depth, pointing towards you) = %.2f";
	char msg4[strlen(msg4_formatter) - 4 + 4 + (depth_movement < 0 ? 1 : 0)];
	sprintf(msg4, msg4_formatter, depth_movement + 1.0f);
	size_t msg4_len = strlen(msg4);
	for(i = 0; i < msg4_len; ++i) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *(msg4 + i));
	}
}

const float surfaceUnitLength = 0.125f;
void drawSurface(void) {
	glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer);
	GLint vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	f_matrix *modelMatrix = scaleMatrix(surfaceUnitLength);

	f_vec *eye = createVec(3), *at = createVec(3), *up = createVec(3);

	setVecValue(eye, 0, 0.0f + horizontal_movement);
	setVecValue(eye, 1, 0.0f + vertical_movement);
	setVecValue(eye, 2, 1.0f + depth_movement);

	printPosition();
	
	setVecValue(at, 0, 0.0f);
	setVecValue(at, 1, 0.0f);
	setVecValue(at, 2, 0.0f);

	setVecValue(up, 0, 0.0f);
	setVecValue(up, 1, 1.0f);
	setVecValue(up, 2, 0.0f);

	f_matrix *viewMatrix = lookAt(eye, at, up);
	f_matrix *translation;
	
	const unsigned int surface_width = 3;
	const unsigned int surface_length = 3;

	f_matrix *projectionMatrix = ortho(-1, 1, -1, 1, 0, 2);

	size_t c, a;
	for(a = 0; a < surface_length; ++a) {
		for(c = 0; c < surface_width; ++c) {
			translation = translationMatrix(c*surfaceUnitLength - (surface_width * surfaceUnitLength * 0.5f),
											0.0f,
											a*surfaceUnitLength - (surface_length * surfaceUnitLength * 0.5f));
			multMatrix(translation, modelMatrix, DESTRUCTIVE_MULT_A);
		
			glUniformMatrix4fv(mModelLoc, 1, GL_FALSE, translation->data);
			glUniformMatrix4fv(mViewLoc, 1, GL_FALSE, viewMatrix->data);
			glUniformMatrix4fv(mProjectionLoc, 1, GL_FALSE, projectionMatrix->data);

			//modeLoc true (drawing surfaces)
			glUniform1i(modeLoc, 1);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buffer);
			glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_BYTE, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, second_cube_element_buffer);
			glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_BYTE, 0);

			//modeLoc false (drawing lines)
			f_matrix *scaleLines = scaleMatrix(1.01f);
			multMatrix(translation, scaleLines, DESTRUCTIVE_MULT_B);

			glUniformMatrix4fv(mModelLoc, 1, GL_FALSE, scaleLines->data);

			glUniform1i(modeLoc, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buffer);
			glDrawElements(GL_LINE_LOOP, 8, GL_UNSIGNED_BYTE, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, second_cube_element_buffer);
			glDrawElements(GL_LINE_LOOP, 8, GL_UNSIGNED_BYTE, 0);

			destroyMatrix(translation);
			destroyMatrix(scaleLines);
		}
	}

	destroyMatrix(modelMatrix);
	destroyVec(eye);
	destroyVec(at);
	destroyVec(up);
	destroyMatrix(viewMatrix);
	destroyMatrix(projectionMatrix);
}
