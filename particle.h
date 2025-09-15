#define _USE_MATH_DEFINES
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

/*
FOR ALL EXPLANATIONS AND DERIVATIONS BEHIND THE MATH HERE, SEE THE NOTEBOOK
*/

//TODO: change delta dx and dy to account for radius

extern int num_particles;
extern int window_x;
extern int window_y;
extern int num_points;
extern struct particle *particles;
extern struct wall *walls;
extern double time_since_last_frame;
extern int num_walls;

struct particle {
	double mass;
	double radius;
	//All in m or m/s
	double dx;
	double dy;
	double vx;
	double vy;
};

struct collision {
	int entity_1; //entity 1 is a particle
	int entity_2; //entity 2 can be a particle or wall, depending on the boolean
	bool entity_2_is_wall;
	double time_to_collision; //time in seconds until collision will happen
};

struct wall {
	double orgin_x;
	double orgin_y;
	double direction; //in radians clockwise from vertical
	double length;
	int type; //unused for now, will eventually hold different shapes, eg. line, curve, circle
	double thickness;
	bool solid; //Solid true means it will bounce, false means it will delete the particle (to simulate the particle just continuing off away)
};

bool collision_check(struct particle *p_1, struct particle *p_2) { //check if particles will collide using discriminant
	if(p_1 == p_2) return false;
	double delta_dx = p_1->dx - p_2->dx;
	double delta_dy = p_1->dy - p_2->dy;
	double delta_vx = p_1->vx - p_2->vx;
	double delta_vy = p_1->vy - p_2->vy;
	if(pow(2*(delta_dx*delta_vx + delta_dy*delta_vy), 2) - 4*(pow(delta_vx, 2) + pow(delta_vy, 2))*(pow(delta_dx, 2) + pow(delta_dy, 2) - pow(p_1->radius+p_2->radius, 2)) > 0) return true; //if the particles will collide, and the particles are not the same particle return true
	else return false;
}

double point_of_collision_wall(struct particle *p, struct wall *wall) { //returns the distance along the wall from the orgin that the particle will collide. should not be called with stationary particles, or particles moving in the same direction as the wall. safe to call if the collision check returned true or inside the collision check function after checking for stationary particles and particles moving parallel to the wall
	return (p->vx*(wall->orgin_y - p->dy) - p->vy*(wall->orgin_x - p->dx))/(p->vy*sin(wall->direction) - p->vx*cos(wall->direction));
}

double time_to_collision_wall(struct particle *p, struct wall *wall) { //returns the time to collision with the wall. should not be called without checking a collision will occur or it will return the time to collision with a point not on the wall, or if the particle is not moving (and thus not going to collide), cause a division by 0
	double point_of_collision = point_of_collision_wall(p, wall);
	if(p->vx != 0) {
		return (wall->orgin_x + point_of_collision*sin(wall->direction) - p->dx)/p->vx;
	} else { //we don't need to check for vy == 0 because that would mean a stationary particle, which is an illegal input and can't be handled anyway
		return (wall->orgin_y + point_of_collision*cos(wall->direction) - p->dy)/p->vy;
	}
}

bool collision_check_wall(struct particle *p, struct wall *wall) { //check if a particle will collide with a wall
	//TODO: implement a check for vx or vy 0
	if(atan(p->vx/p->vy) == wall->direction) { //If the particle is travelling parallel to the direction of the wall, it can only hit the end of the wall
		double t_x = (wall->orgin_x - p->dx)/p->vx; //We can find the time it takes for the p->dx to be the orgin_x
		double t_y = (wall->orgin_y - p->dy)/p->vy; //and the time for p->dy to be the orgin_y
		if(t_x == t_y && t_x >= 0) return true; //if the times match, the particle will actually contact the end of the wall. If this time is positive, the collision will actually occur
		else return false;
	} else { //particle is not parallel to wall, normal behaviour
		double point_on_wall = point_of_collision_wall(p, wall);
		if(point_on_wall >= 0 && point_on_wall <= wall->length) {
			if(time_to_collision_wall(p, wall) > 0) return true; //if the time to collision is 0, that means the particle is in contact with the wall. we shouldn't trigger another collision when the particle is already in contact with the wall, as it causes the simulation to become hardlocked
			else return false;
		} else return false;
	}
}

double time_to_collision(struct particle *p_1, struct particle *p_2) { //if two particles will collide, returns the time to the collision
	double delta_dx = p_1->dx - p_2->dx;
	double delta_dy = p_1->dy - p_2->dy;
	double delta_vx = p_1->vx - p_2->vx;
	double delta_vy = p_1->vy - p_2->vy;
	double add = (-1*(delta_dx*delta_vx + delta_dy*delta_vy) + pow(pow(2*(delta_dx*delta_vx + delta_dy*delta_vy), 2) - 4*(pow(delta_vx, 2) + pow(delta_vy, 2))*(pow(delta_dx, 2) + pow(delta_dy, 2) - pow(p_1->radius + p_2->radius, 2)), 0.5))/2*(pow(delta_vx, 2) + pow(delta_vy,2 ));
	double subtract = (-1*(delta_dx*delta_vx + delta_dy*delta_vy) - pow(pow(2*(delta_dx*delta_vx + delta_dy*delta_vy), 2) - 4*(pow(delta_vx, 2) + pow(delta_vy, 2))*(pow(delta_dx, 2) + pow(delta_dy, 2) - pow(p_1->radius + p_2->radius, 2)), 0.5))/2*(pow(delta_vx, 2) + pow(delta_vy,2 ));
	if(add < 0.0) return subtract; //if either value is negative go with the other one because only one value can be negative (the proof for this is trivial) and the negative collision will not happen
	else if(subtract < 0.0) return add;
	return add < subtract ? add : subtract; //returns the lower option
}

struct collision find_any_collision(int current) { //finds a random collision with a wall
	for(int i = 0; i < num_walls; i++) {
		if(collision_check_wall(&particles[current], &walls[i])) {
			return (struct collision) {current, i, true, time_to_collision_wall(&particles[current], &walls[i])};
		}
	}
	//Something has gone wrong if we get here
	return (struct collision) {0};
}

struct collision find_next_collision_wall(int current)  { //this function could accept a pointer to a particle, but for consistency with the particle version, which requires a index, we take indexes
	struct collision soonest_collision = find_any_collision(current);
	for(int i = 0; i < num_walls; i++) {
		if(collision_check_wall(&particles[current], &walls[i])) {
			if(time_to_collision_wall(&particles[current], &walls[i]) < soonest_collision.time_to_collision) {
				soonest_collision.time_to_collision = time_to_collision_wall(&particles[current], &walls[i]);
				soonest_collision.entity_2 = i;
			}
		}
	}
	return soonest_collision;
}

struct collision find_next_collision_particle(int current) {
	struct collision soonest_collision = find_any_collision(current); //initialize
	for(int i=0; i<num_particles; i++) {
		if(collision_check(&particles[current], &particles[i])) {
			if(time_to_collision(&particles[current], &particles[i]) < soonest_collision.time_to_collision) {
				soonest_collision.time_to_collision = time_to_collision(&particles[current], &particles[i]);
				soonest_collision.entity_2 = i;
				soonest_collision.entity_2_is_wall = false;
			}
		}
	}
	return soonest_collision;
}

struct collision find_next_collision(int current) { //find the next collision for any given particle, not necessarily the next collision
	//we don't need to initialize it this time because wall_time_to_impact is a guaranteed value
	double wall_time_to_impact = find_next_collision_wall(current).time_to_collision;
	double particle_time_to_impact = find_next_collision_particle(current).time_to_collision;
	if(wall_time_to_impact >3 || particle_time_to_impact>3) {
		find_next_collision_wall(current);
		find_next_collision_particle(current);
	}
	return wall_time_to_impact <= particle_time_to_impact ? (struct collision) {current, find_next_collision_wall(current).entity_2 , true, wall_time_to_impact} : (struct collision) {current, find_next_collision_particle(current).entity_2, false, particle_time_to_impact}; //the condition must be <=, not just <, because if not, the case where a particle will not collide with any other particles, and therefore the particle_time_to_impact is equal to the wall time to impact, will cause it to return the wall time to impact as a particle collision
}

void convert_to_alt_FOR(struct particle *p, double axis) { //uses projections to convert between a regular frame of reference and a frame of reference defined as having x+ parallel to the axis of collision
	//axis is the angle in radians clockwise from vertical that the collision takes place on
	double a_x = sin(axis); //a is a unit vector parallel to the axis
	double a_y = cos(axis);
	double p_dot_a = p->vx*a_x + p->vy*a_y; //dot product of p and a. we are using it a lot, so it make sense to just compute it once and reuse for faster and cleaner code
	//calc vy before vx because changing the value of vx will affect vy calc
	p->vy = pow(pow(p->vx, 2) + pow(p->vy, 2) - pow(p_dot_a, 2), 0.5); //the new y component is the magnitude of of the perpendicular of p on a, see notes for derivation
	p->vx = p_dot_a; //the new x component is the magnitude of the projection of p on a, which is simply p dot a
}

void convert_to_regular_FOR(struct particle *p, double axis) {
	double new_vx = p->vx*sin(axis) + p->vy*cos(axis); //the conventional FOR x component is just the sum of the conventional axis x components of vx and vy
	p->vy = p->vx*cos(axis) - p->vy*sin(axis); //same idea as for vx. we assign directly to p to save an extra variable allocation and read/write. the compiler might optimize this anyway
	p->vx = new_vx;
}

void particle_collision(struct particle *p1, struct particle *p2) { //assume perfectly elastic collision of point masses
	// initial x velocity stays identical along the axis of collision
	// y velocity changes as if there was a 1d head-on collision
	double axis = tan((p1->dx - p2->dx)/(p1->dy - p2->dy)) + M_PI_2;
	convert_to_alt_FOR(p1, axis);
	convert_to_alt_FOR(p2, axis);
	double p1_total = pow(pow(p1->vx, 2) + pow(p1->vy, 2), 0.5);
	double p2_total = pow(pow(p2->vx, 2) + pow(p2->vy, 2), 0.5);
	p1->vy = p1_total*(p1->mass - p2->mass)/(p1->mass + p2->mass) + 2*p2_total*p2->mass/(p1->mass + p2->mass);
	p2->vy = p2_total*(p2->mass - p1->mass)/(p1->mass + p2->mass) + 2*p2_total*p2->mass/(p1->mass + p2->mass);
	convert_to_regular_FOR(p1, axis);
	convert_to_regular_FOR(p2, axis);
}

void wall_collision(struct particle *p, struct wall *wall) {
	convert_to_alt_FOR(p, wall->direction);
	convert_to_regular_FOR(p, wall->direction);
}

void collision(struct collision collision) {
	if(collision.entity_2_is_wall) {
		wall_collision(&particles[collision.entity_1], &walls[collision.entity_2]);
	} else {
		particle_collision(&particles[collision.entity_1], &particles[collision.entity_2]);
	}
}

void do_movement(double time) { //will not check for or process collisions and should not be called with a time greater than the amount of time until the next collision
	for(int i = 0; i < num_particles; i++) {
		particles[i].dx += particles[i].vx*time;
		particles[i].dy += particles[i].vy*time;
	}
	time_since_last_frame += time;
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
