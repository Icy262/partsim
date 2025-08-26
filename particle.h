#define _USE_MATH_DEFINES
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>

extern int num_particles;
extern int window_x;
extern int window_y;
extern int num_points;
extern struct particle *particles;

struct particle {
	double mass;
	double radius;
	//All in m or m/s
	double dx;
	double dy;
	double vx;
	double vy;
};

struct next_collision {
	int particle; //index of particle collision will happen with, OR -1 for wall
	double time_to_collision; //time in seconds until collision will happen
};

bool collision_check(struct particle *p_1, struct particle *p_2) { //check if particles will collide using discriminant
	double delta_dx = p_1->dx - p_2->dx;
	double delta_dy = p_1->dy - p_2->dy;
	double delta_vx = p_1->vx - p_2->vx;
	double delta_vy = p_1->vy - p_2->vy;
	if (pow(2*(delta_dx*delta_vx + delta_dy*delta_vy), 2) - 4*(pow(delta_vx, 2) + pow(delta_vy, 2))*(pow(delta_dx, 2) + pow(delta_dy, 2) - pow(p_1->radius+p_2->radius, 2)) > 0) return true;
	else return false;
}

double time_to_collision(struct particle *p_1, struct particle *p_2) { //if two particles will collide, returns the time to the collision
	double delta_dx = p_1->dx - p_2->dx;
	double delta_dy = p_1->dy - p_2->dy;
	double delta_vx = p_1->vx - p_2->vx;
	double delta_vy = p_1->vy - p_2->vy;
	double add = (-1*(delta_dx*delta_vx + delta_dy*delta_vy) + pow(pow(2*(delta_dx*delta_vx + delta_dy*delta_vy), 2) - 4*(pow(delta_vx, 2) + pow(delta_vy, 2))*(pow(delta_dx, 2) + pow(delta_dy, 2) - pow(p_1->radius + p_2->radius, 2)), 0.5))/2*(pow(delta_vx, 2) + pow(delta_vy,2 ));
	double subtract = (-1*(delta_dx*delta_vx + delta_dy*delta_vy) - pow(pow(2*(delta_dx*delta_vx + delta_dy*delta_vy), 2) - 4*(pow(delta_vx, 2) + pow(delta_vy, 2))*(pow(delta_dx, 2) + pow(delta_dy, 2) - pow(p_1->radius + p_2->radius, 2)), 0.5))/2*(pow(delta_vx, 2) + pow(delta_vy,2 ));
	return add < subtract ? add : subtract; //returns the lower option
}

double time_to_collision_wall(struct particle *p) {
	double time_to_x_wall, time_to_y_wall;
	if(p->vx > 0) {
		time_to_x_wall = window_x - p->dx/p->vx;
	} else {
		time_to_x_wall = -1*p->dx/p->vx;
	}
	if(p->vy > 0) {
		time_to_y_wall = window_y - p->dy/p->vy;
	} else {
		time_to_y_wall = -1*p->dy/p->vy;
	}
	return time_to_x_wall < time_to_y_wall ? time_to_x_wall : time_to_y_wall;
}

struct next_collision find_next_collision(int current) {
	double lowest_time=time_to_collision_wall(&particles[current]); //we know that all particles will eventually hit a wall, therefore we should check if there will be a particle collision before then
	int lowest_index=-1; //if a particle collision will happen, this will change, if not we know that it will be a wall collision
	for(int i=0; i<num_particles; i++) {
		if(collision_check(&particles[current], &particles[i])) {
			if(time_to_collision(&particles[current], &particles[i])<lowest_time) {
				lowest_time=time_to_collision(&particles[current], &particles[i]);
				lowest_index=i;
			}
		}
	}
}

void convert_to_alt_FOR(struct particle *p, double axis) { //axis is between 0-2pi rad clockwise from vertical and is perpendicular to a line drawn between the centres of the two particles
	struct particle new_p; //create a new particle so we can avoid interference from the two calcs
	new_p.vx = (p->vy-p->vx/sin(M_PI_2-axis))/(cos(axis)-sin(axis)/sin(M_PI_2-axis));
	new_p.vy = (p->vy-p->vx/tan(axis))/(cos(M_PI_2-axis)-sin(M_PI_2-axis)/tan(axis));
	*p=new_p;
}

void convert_to_regular_FOR(struct particle *p, double axis) {
	struct particle new_p;
	new_p.vx=p->vx*tan(axis)+p->vy*tan(M_PI_2-axis);
	new_p.vy=p->vx*cos(axis)+p->vy*cos(M_PI_2-axis);
	*p=new_p;
}

void collision(struct particle *p1, struct particle *p2) { //assume perfectly elastic collision of point masses
}

float* gen_circle(int current_particle) {
	double a=2*M_PI/num_points; //2PI/numtriangles. a is the internal angle of the triangles;
	float *points=malloc(num_points*9*sizeof(float));
	for(int i=0; i<num_points; i++) {
		int start_of_triangle=i*9;
		points[start_of_triangle]=2*particles[current_particle].dx/window_x-1.0f;
		points[start_of_triangle+1]=2*particles[current_particle].dy/window_x-1.0f;
		points[start_of_triangle+2]=0;

		points[start_of_triangle+3]=2*particles[current_particle].dx/window_x-1.0f + particles[current_particle].radius*sin(i*a);
		points[start_of_triangle+4]=2*particles[current_particle].dy/window_y-1.0f + particles[current_particle].radius*cos(i*a);
		points[start_of_triangle+5]=0;

		points[start_of_triangle+6]=2*particles[current_particle].dx/window_x-1.0f + particles[current_particle].radius*sin((i+1)*a);
		points[start_of_triangle+7]=2*particles[current_particle].dy/window_y-1.0f + particles[current_particle].radius*cos((i+1)*a);
		points[start_of_triangle+8]=0;
	}
	return points;
};
