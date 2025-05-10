#include <stdbool.h>
#include <math.h>

struct particle {
	double mass;
	double radius;
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