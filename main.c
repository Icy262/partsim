#include <stdio.h>
#include "particle.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int main(){
    srand(time(NULL));
    int nparticles=100;
    struct particle *test=malloc(100 * sizeof(struct particle));
    for(int i=0; i<100; i++) {
        test[i].mass=rand()%50;
    }
    for(int i=0; i<100; i++) {
        test[i].radius=rand()%10;
    }
    for(int i=0; i<100; i++) {
        test[i].dx=rand()%1000;
    }
    for(int i=0; i<100; i++) {
        test[i].dy=rand()%1000;
    }
    for(int i=0; i<100; i++) {
        test[i].vx=rand()%100;
    }
    for(int i=0; i<100; i++) {
        test[i].vy=rand()%100;
    }
}