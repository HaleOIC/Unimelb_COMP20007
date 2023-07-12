/*
    Written by William Sutherland for 
    COMP20007 Assignment 1 2023 Semester 1
    Modified by Grady Fitzpatrick
    
    Implementation for module which contains map-related 
        data structures and functions.

    Functions in the task description to implement can
        be found here.
    
    Code implemented by <Yuxiang Lei>
*/
#include "map.h"
#include "stack.h"
#include "pq.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

struct map *newMap(int height, int width) {
    struct map *m = (struct map *) malloc(sizeof(struct map));
    assert(m);
    m->height = height;
    m->width = width;

    // Note this means all values of map are 0
    int *points = (int *) calloc(height * width, sizeof(int));
    assert(points);
    m->points = (int **) malloc(width * sizeof(int *));
    assert(m->points);

    for (int i = 0; i < width; i++){
        /* Re-use sections of the memory we 
            allocated. */
        m->points[i] = points + i * height;
    }

    return m;
}

struct point *newPoint(int x, int y) {
    struct point *p = (struct point *) malloc(sizeof(struct point));
    assert(p);
    p->x = x;
    p->y = y;

    return p;
}

void freeMap(struct map *m) {
    /* We only allocate one pointer with the rest
    of the pointers in m->points pointing inside the
    allocated memory. */
    free(m->points[0]);
    free(m->points);
    free(m);
}

void printMap(struct map *m) {
    /* Print half of each row each time so we mirror the hexagonal layout. */
    int printRows = 2 * m->height;
    if(m->width < 2){
        /* If the width is less than 2, simply a vertical column so no need to print 
            last row as it will be empty. */
        printRows -= 1;
    }
    for (int i = 0; i < printRows; i++) {
        for (int j = i % 2; j < m->width; j += 2) {
            if (j == 1){
                /* For odd row, add a spacer in to place the first value after the 0th hex
                    in the printout. */
                printf("       ");
            }
            /* Default to even. Select every second column. */
            int yPos = i / 2;
            if(j % 2 != 0){
                /* If odd, numbering along height is reversed. */
                yPos = m->height - 1 - yPos;
            }
            int val = m->points[j][yPos];

            /* Print value appropriately. */
            if (val < 0){
                printf("S%.3d", val % 1000);
            } else if (val == 0){
                printf("  L  ");
            } else if (val == 100){
                printf("  A  ");
            } else {
                printf("T+%.3d ", val % 1000);
            }
            printf("          ");
        }

        printf("\n");
    }
}

/* IMPLEMENT PART A HERE */
/* Note that for the implementation in this question, you will submit
   an array that contains all the adjacent points and then add an additional
   point at the end of the array with coordinates -1, -1 to signify the
   termination of the array (similar to '\0' terminating a string) */
struct point *getAdjacentPoints(struct map *m, struct point *p) {
    struct point *result = (struct point *) malloc(sizeof(struct point) * 7);

    int height = m->height;
    int width = m->width;
    if ((p->x >= 0) && (p->x < width) && (p->y >= 0) && (p->y < height)) {
        // Prepare coordinates for neighbor points
        int xs[] = {p->x - 1, p->x - 1, p->x, p->x, p->x + 1, p->x + 1};
        int ys[] = {height - 1 - (p->y), height - (p->y), p->y - 1, p->y + 1, height - 1 - (p->y), height - (p->y)};
        // Iterate through the coordinates, put them into list if it is on map
        int j = 0;
        for (int i = 0; i < 6; i++) {
            if ((xs[i] >= 0) && (xs[i] < width) && (ys[i] >= 0) && (ys[i] < height)) {
                result[j].x = xs[i];
                result[j].y = ys[i];
                j++;
            }
        }
        // Fill remaining points with -1
        result[j].x = -1;
        result[j].y = -1;
    } else {
        result[0].x = -1;
    }

    return result;
}
/* IMPLEMENT PART B HERE */
void traversal(struct map *m, struct point *point, int *value, int *count, int **visited) {
    // Mark current point as visited
    visited[point->x][point->y] = 1;

    if (m->points[point->x][point->y] > 0 && m->points[point->x][point->y] != 100) {
        // Current point is treasure, add to treasure count and value
        *count += 1;
        *value *= m->points[point->x][point->y];
    }

    struct point *neighbors = getAdjacentPoints(m, point);
    while (neighbors->x != -1) {
        if (visited[neighbors->x][neighbors->y] == 0 && m->points[neighbors->x][neighbors->y] >= 0) {
            traversal(m, neighbors, value, count, visited);
        }
        neighbors += 1;
    }
}

int mapValue(struct map *m) {
    int sum = 0;
    int **visited = (int**) malloc(sizeof(int*) *m -> width);
    // initialize visited status
    for (int i = 0; i < m->width; i++) {
        visited[i] = (int *) malloc(sizeof(int) *m -> height);
        for (int j = 0; j < m->height; j++) {
            visited[i][j] = 0;
        }
    }
    // iterate through the map
    for (int i = 0; i < m->width; i++) {
        for (int j = 0; j < m->height; j++) {
            if (visited[i][j] == 0) {
                int value = 1;
                int count = 0;
                struct point p = {i, j};
                traversal(m, &p, &value, &count, visited);
                
                sum += value * count;
            }
        }
    }
    return sum;
}

/* IMPLEMENT PART D HERE */
int minTime(struct map *m, struct point *start, struct point *end) {
    // Initialize priority queue
    struct pq *pq = createPQ();

    // Keep records of visited points and their distance
    int **visited = (int**) malloc(m->width * sizeof(int *));
    int **distance = (int**) malloc(m->width * sizeof(int *));

    // Initialize visited and distance matrix
    for (int i = 0; i < m->width; i++) {
        visited[i] = (int*) malloc(m->height* sizeof(int));
        distance[i] = (int*) malloc(m->height* sizeof(int));

        for (int j = 0; j < m->height; j++) {
            // Set initial time to a large value
            distance[i][j] = 99999999;
            visited[i][j] = 0;
        }
    }

    // Initialize distance for initial point
    distance[start->x][start->y] = 0;
    insert(pq, start, 0);


    // Loop to find best time
    while(!isEmpty(pq)) {
        // Get closest point from queue
        struct point* current = (struct point *) pull(pq);

        // Check visit state
        if (visited[current->x][current->y] > 0) {
            continue;
        } else {
            visited[current->x][current->y] = 1;
        }

        // See if point is airport, find distance with other airports
        int x = current->x;
        int y = current->y;
        int currentTime = distance[x][y];
        if (m->points[x][y] == 100) {
            for (int i2 = 0; i2 < m->width; i2++) {
                for (int j2 = 0; j2 < m->height; j2++) {
                    if (i2 == x && j2 == y) {
                        continue;
                    }
                    if (m->points[i2][j2] != 100) {
                        continue;
                    }
                    if (visited[i2][j2] != 0) {
                        continue;
                    }
                    int v = (x - i2) * (x - i2) - 85;
                    int weight = v < 15 ? 15 : v;
                    int newdistance = weight + currentTime;
                    if (newdistance < distance[i2][j2]) {
                        distance[i2][j2] = newdistance;
                        insert(pq, newPoint(i2, j2), -newdistance);
                    }
                }
            }
        }

        struct point *neighbors = getAdjacentPoints(m, current);
        while (neighbors->x != -1) {
            int newdistance = currentTime + 5;
            if (m->points[x][y] < 0) {
                int value = m->points[x][y];
                newdistance = currentTime + 2 + value * value / 1000;
                if ((value * value) % 1000 != 0) {
                    newdistance += 1;
                }
            }
            if (newdistance < distance[neighbors->x][neighbors->y]) {
                distance[neighbors->x][neighbors->y] = newdistance;
                insert(pq, neighbors, -newdistance);
            }

            neighbors += 1;
        }
    }

    free(pq);
    return distance[end->x][end->y];
}

/* IMPLEMENT PART E HERE */
int minTimeDry(struct map *m, struct point *start, struct point *end, struct point *airports, int numAirports) {
    // Initialize priority queue
    struct pq *pq = createPQ();

    // Keep records of visited points and their distance
    int **visited = (int**) malloc(m->width * sizeof(int *));
    int **distance = (int**) malloc(m->width * sizeof(int *));

    // Initialize visited and distance matrix
    for (int i = 0; i < m->width; i++) {
        visited[i] = (int*) malloc(m->height* sizeof(int));
        distance[i] = (int*) malloc(m->height* sizeof(int));

        for (int j = 0; j < m->height; j++) {
            // Set initial time to a large value
            distance[i][j] = 999999999;
            visited[i][j] = 0;
        }
    }

    // Initialize distance for initial point
    distance[start->x][start->y] = 0;
    insert(pq, start, 0);

    // Loop to find best time
    while(!isEmpty(pq)) {
        // Get closest point from queue
        struct point* current = pull(pq);

        // Check visit state
        if (visited[current->x][current->y] > 0) {
            continue;
        } else {
            visited[current->x][current->y] = 1;
        }

        // See if point is airport, find distance with other airports
        if (m->points[current->x][current->y] == 100) {
            for (int k = 0; k < numAirports; k++) {
                int i2 = airports[k].x;
                int j2 = airports[k].y;
            
                int v = abs(current->x - i2) * abs(current->x - i2) - 85;
                int weight = v < 15 ? 15 : v;
                int newdistance = weight + distance[current->x][current->y];
                if (newdistance < distance[i2][j2]) {
                    distance[i2][j2] = newdistance;
                    insert(pq, newPoint(i2, j2), -newdistance);
                }
            }
        }

        struct point *neighbors = getAdjacentPoints(m, current);
        struct point *temp = neighbors;
        while (neighbors->x != -1) {
            int newdistance = distance[current->x][current->y] + 5;
            if (m->points[neighbors->x][neighbors->y] < 0) {
                int value = m->points[neighbors->x][neighbors->y];
                newdistance = distance[current->x][current->y] + 2 + value * value / 1000;
                if (value % 1000 != 0) {
                    newdistance += 1;
                }
            }
            if (newdistance < distance[neighbors->x][neighbors->y]) {
                distance[neighbors->x][neighbors->y] = newdistance;
                insert(pq, newPoint(neighbors->x, neighbors->y), -newdistance);
            }

            neighbors += 1;
        }
        free(temp);
        free(current);
    }

    free(pq);
    return distance[end->x][end->y];
}
