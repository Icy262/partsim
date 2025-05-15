#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

struct particle {
	double mass;
	double radius;
	//All in m or m/s
	double dx;
	double dy;
	double vx;
	double vy;
};

struct particle* collision(struct particle *p1, struct particle *p2){ //assume perfectly elastic collision of point masses
}

bool collision_check(struct particle *p1, struct particle *p2) { //check if particles will collide using discriminant
	double delta_dx=p1->dx-p2->dx;
	double delta_dy=p1->dy-p2->dy;
	double delta_vx=p1->vx-p2->vx;
	double delta_vy=p1->vy-p2->vy;
	if(pow(2*(delta_dx*delta_vx+delta_dy*delta_vy), 2)-4*(pow(delta_vx, 2)+pow(delta_vy, 2))*(pow(delta_dx, 2)+pow(delta_dy, 2)-pow(p1->radius+p2->radius, 2))>0) return true;
	else return false;
}

float* gen_circle(struct particle *particle, int num_triangles, int window_x, int window_y) {
	double a=6.28318530718/num_triangles; //2PI/numtriangles. a is the internal angle of the triangles;
	float *points=malloc(num_triangles*9*sizeof(float));
	for(int i=0; i<num_triangles; i++) {
		int start_of_triangle=i*9;
		points[start_of_triangle]=2*particle->dx/window_x-1.0f;
		points[start_of_triangle+1]=2*particle->dy/window_x-1.0f;
		points[start_of_triangle+2]=0;

		points[start_of_triangle+3]=2*particle->dx/window_x-1.0f + particle->radius*sin(i*a);
		points[start_of_triangle+4]=2*particle->dy/window_y-1.0f + particle->radius*cos(i*a);
		points[start_of_triangle+5]=0;

		points[start_of_triangle+6]=2*particle->dx/window_x-1.0f + particle->radius*sin((i+1)*a);
		points[start_of_triangle+7]=2*particle->dy/window_y-1.0f + particle->radius*cos((i+1)*a);
		points[start_of_triangle+8]=0;
	}
	return points;
};