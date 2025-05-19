#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

extern int num_particles;
extern int window_x;
extern int window_y;
extern int num_points;
extern float tick_time;
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

bool collision_check(struct particle *p_1, struct particle *p_2) { //check if particles will collide using discriminant
	double delta_dx=p_1->dx-p_2->dx;
	double delta_dy=p_1->dy-p_2->dy;
	double delta_vx=p_1->vx-p_2->vx;
	double delta_vy=p_1->vy-p_2->vy;
	if(pow(2*(delta_dx*delta_vx+delta_dy*delta_vy), 2)-4*(pow(delta_vx, 2)+pow(delta_vy, 2))*(pow(delta_dx, 2)+pow(delta_dy, 2)-pow(p_1->radius+p_2->radius, 2))>0) return true;
	else return false;
}

void collision(struct particle *p1, struct particle *p2) { //assume perfectly elastic collision of point masses
}

void edge_collision(struct particle *particle) { //TODO: don't allow particle to go past the edge of the screen
	if(particle->dx>window_x||particle->dx<0) {
		particle->vx*=-1;
	}
	if(particle->dy>window_x||particle->dy<0) {
		particle->vy*=-1;
	}
}

void tick_particle(bool *ticked, int current_particle) {
	if(ticked[current_particle]) return;
	for(int i=current_particle; i<num_particles; i++) {
		if(collision_check(&particles[current_particle], &particles[i])) {
			collision(&particles[current_particle], &particles[i]);
			edge_collision(&particles[current_particle]);
			edge_collision(&particles[i]);
			ticked[current_particle]=true;
			ticked[i]=true;
			return;
		}
	}
	//TODO: prevent particles from going past screen border before turning around
	particles[current_particle].dx+=particles[current_particle].vx*tick_time;
	particles[current_particle].dy+=particles[current_particle].vy*tick_time;
	edge_collision(&particles[current_particle]);
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
