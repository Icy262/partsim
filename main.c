#include <stdio.h>
#include "particle.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main(){
	srand(time(NULL));
	int num_particles=100;
	int window_x=1000;
	int window_y=1000;
	int num_points=32;
	struct particle *test=malloc(100 * sizeof(struct particle));
	for(int i=0; i<num_particles; i++) {
		test[i].mass=rand()%50;
	}
	for(int i=0; i<num_particles; i++) {
		test[i].radius=0.01;//set radius to be 1% of the screen
	}
	for(int i=0; i<num_particles; i++) {
		test[i].dx=rand()%window_x;
	}
	for(int i=0; i<num_particles; i++) {
		test[i].dy=rand()%window_y;
	}
	for(int i=0; i<num_particles; i++) {
		test[i].vx=rand()%20;
	}
	for(int i=0; i<num_particles; i++) {
		test[i].vy=rand()%20;
	}
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(window_x, window_y, "partsim", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL();
	
	GLuint vbo = 0;
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );

	float *render=malloc(num_points*9*sizeof(float)*num_particles);
	for(int i=0; i<num_particles; i++) {
		memcpy(&render[num_points*9*i], gen_circle(&test[i], num_points, window_x, window_y), num_points*9*sizeof(float));
	};
	glBufferData( GL_ARRAY_BUFFER, num_points*9*sizeof(float)*num_particles, render, GL_STATIC_DRAW );

	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	const char* vertex_shader =
	"#version 410 core\n"
	"in vec3 vp;"
	"void main() {"
	"  gl_Position = vec4( vp, 1.0 );"
	"}";
	const char* fragment_shader =
	"#version 410 core\n"
	"out vec4 frag_colour;"
	"void main() {"
	"  frag_colour = vec4( 0.5, 0.0, 0.5, 1.0 );"
	"}";
	GLuint vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &vertex_shader, NULL );
	glCompileShader( vs );
	GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragment_shader, NULL );
	glCompileShader( fs );
	GLuint shader_program = glCreateProgram();
	glAttachShader( shader_program, fs );
	glAttachShader( shader_program, vs );
	glLinkProgram( shader_program );

	while ( !glfwWindowShouldClose( window ) ) {
		for(int i=0; i<num_particles; i++) {
			test[i].dx+=test[i].vx;
			test[i].dy+=test[i].vy;
			if(test[i].dx>window_x||test[i].dx<0) {
				test[i].vx*=-1;
			}
			if(test[i].dy>window_y||test[i].dy<0) {
				test[i].vy*=-1;
			}
		}

		float *render=malloc(num_points*9*sizeof(float)*num_particles);
		for(int i=0; i<num_particles; i++) {
			memcpy(&render[num_points*9*i], gen_circle(&test[i], num_points, window_x, window_y), num_points*9*sizeof(float));
		};
		glBufferData( GL_ARRAY_BUFFER, num_points*9*sizeof(float)*num_particles, render, GL_STATIC_DRAW );
		
		// Update window events.
		glfwPollEvents();
		
		// Wipe the drawing surface clear.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		// Put the shader program, and the VAO, in focus in OpenGL's state machine.
		glUseProgram( shader_program );
		glBindVertexArray( vao );

		// Draw points 0-3 from the currently bound VAO with current in-use shader.
		glDrawArrays( GL_TRIANGLES, 0, 3*num_points*num_particles );
		
		// Put the stuff we've been drawing onto the visible area.
		glfwSwapBuffers( window );
	}
	glfwTerminate();
}