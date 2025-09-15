#include <stdio.h>
#include "particle.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

int num_particles = 2;
int window_x = 1000;
int window_y = 1000;
int num_points = 32;
struct wall *walls;
struct particle *particles;
double time_since_last_frame = 0;
int framerate = 60;
int num_walls = 4;

int main() {
	srand(time(NULL));
	walls = malloc(num_walls*sizeof(struct wall));
	particles = malloc(num_particles*sizeof(struct particle));
	//There must be a wall containing the entire simulation, or undefined behaviour will occur when checking for wall collisions
	walls[0] = (struct wall) {0, 0, 0, window_y, 0, 0.0, true};
	walls[1] = (struct wall) {0, window_y, M_PI_2, window_x, 0, 0.0, true};
	walls[2] = (struct wall) {window_x, window_y, M_PI, window_y, 0, 0.0, true};
	walls[3] = (struct wall) {window_x, 0, 3*M_PI_2, window_x, 0, 0.0, true};
	for(int i=0; i<num_particles; i++) {
		particles[i].mass=rand()%50;
	}
	for(int i=0; i<num_particles; i++) {
		particles[i].radius=0.01;//set radius to be 1% of the screen
	}
	for(int i=0; i<num_particles; i++) {
		particles[i].dx=rand()%window_x;
	}
	for(int i=0; i<num_particles; i++) {
		particles[i].dy=rand()%window_y;
	}
	for(int i=0; i<num_particles; i++) {
		float angle=2*M_PI*rand()/RAND_MAX;
		particles[i].vx=500*sin(angle);
		particles[i].vy=500*cos(angle);
	}
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(window_x, window_y, "partsim", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL();
	
	GLuint vbo = 0;
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );

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

	float *render = malloc(num_points*9*sizeof(float)*num_particles);
	struct collision next_collision;
	bool next_collision_outdated = true;
	while ( !glfwWindowShouldClose( window ) ) {
		if(!next_collision_outdated && next_collision.time_to_collision <= 1.0/framerate - time_since_last_frame) {
			do_movement(next_collision.time_to_collision);

			collision(next_collision);
			next_collision_outdated=true;
		} else if(!next_collision_outdated && next_collision.time_to_collision > 1.0/framerate - time_since_last_frame) {
			double time_to_step = 1.0/framerate - time_since_last_frame; //we need to make a copy of this value, because do_movement will change the time_since_last_frame
			do_movement(time_to_step);
			next_collision.time_to_collision -= time_to_step;
		}

		while(time_since_last_frame < 1.0/framerate) {
			next_collision = find_any_collision(0); //initialize by finding a random collision, in this case we used 0 for convenience's sake
			for(int i = 0; i < num_particles; i++) {
				if(find_next_collision(i).time_to_collision < next_collision.time_to_collision) {
					next_collision = find_next_collision(i);
				}
			}
			if(time_since_last_frame + next_collision.time_to_collision < 1.0/framerate) {
				do_movement(next_collision.time_to_collision);
				collision(next_collision);
			} else {
				double time_to_tick = 1.0/framerate - time_since_last_frame; //do_movement changes the time since last frame, causing inaccurate timekeeping
				do_movement(time_to_tick);
				next_collision.time_to_collision -= time_to_tick;
				next_collision_outdated = false;
			}		
		}
		//rendering particles
		time_since_last_frame = 0;
		for(int i=0; i<num_particles; i++) {
			memcpy(&render[num_points*9*i], gen_circle(i), num_points*9*sizeof(float));
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
	free(render);
	free(walls);
	free(particles);
	glfwTerminate();
}
