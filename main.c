#include <stdio.h>
#include "particle.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

int num_particles = 100;
int window_x = 1000;
int window_y = 1000;
int num_points = 32;
struct particle *particles;
struct wall *walls;
double time_since_last_frame = 0;
int framerate = 60;

int main() {
	srand(time(NULL));
	particles=malloc(num_particles * sizeof(struct particle));
	walls=malloc(4 * sizeof(struct wall));
	double orgin_x;
	double orgin_y;
	double direction; //in radians clockwise from vertical
	double length;
	walls[0] = (struct wall) {0, 0, M_PI_2, 1000};
	walls[1] = (struct wall) {1000, 0, M_PI, 1000};
	walls[2] = (struct wall) {1000, 1000, 3*M_PI_2, 1000};
	walls[3] = (struct wall) {0, 1000, 0, 1000};
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
	int particle_colliding_next; //particle colliding next
	struct next_collision next_overall_collision; //what it will collide with
	bool next_overall_collision_outdated = true; //if this collision has happened or is pending. exists to allow collisions that will not happen till the next frame to not have to be rechecked
	while ( !glfwWindowShouldClose( window ) ) {
		if(!next_overall_collision_outdated && next_overall_collision.time_to_collision <= 1.0/framerate - time_since_last_frame) {
			do_movement(next_overall_collision.time_to_collision);
			collision(&particles[particle_colliding_next], &particles[next_overall_collision.other_particle]);
			next_overall_collision_outdated=true;
		} else if(!next_overall_collision_outdated && next_overall_collision.time_to_collision > 1.0/framerate - time_since_last_frame) {
			do_movement(1.0/framerate - time_since_last_frame);
			next_overall_collision.time_to_collision -= 1.0/framerate - time_since_last_frame;
		}

		while(time_since_last_frame < 1.0/framerate) {
			double lowest_time = time_to_collision_wall(&particles[0]); //we need to initialize with some value and this seemed convenient
	printf("test %lf\n", lowest_time);
			int index = 0; //in the case that particle 0 colliding with a wall actually is the next collision, it's a good idea to set this
			for(int i = 0; i < num_particles; i++) {
				if(find_next_collision(i).time_to_collision < lowest_time) {
					lowest_time = find_next_collision(i).time_to_collision;
					index = i;
				}
			}
			next_overall_collision = find_next_collision(index);
			if(time_since_last_frame + lowest_time < 1.0/framerate) {
				do_movement(lowest_time);
				collision(&particles[index], &particles[next_overall_collision.other_particle]);
			} else {
				do_movement(1.0/framerate - time_since_last_frame);
				next_overall_collision.time_to_collision -= lowest_time;
				particle_colliding_next = index;
				next_overall_collision_outdated = false;
			}			
		}
		printf("NEW RENDER");
		system("pause");
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
	glfwTerminate();
}
