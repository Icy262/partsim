#include <stdio.h>
#include "particle.h"
#include <stdlib.h>

int main(){
    int nparticles=100;
    struct particle* test=malloc(100 * sizeof(struct particle));
    for(int i=0; i<100; i++){
        test[i].mass=rand()%50;
    }
    for(int i=0; i<100; i++){
        test[i].dx=rand()%1000;
    }
    for(int i=0; i<100; i++){
        test[i].dy=rand()%1000;
    }
    for(int i=0; i<100; i++){
        test[i].vx=rand()%1000;
    }
    for(int i=0; i<100; i++){
        test[i].vy=rand()%1000;
    }
    printf("Test");
}