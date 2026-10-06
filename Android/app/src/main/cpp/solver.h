#ifndef GEOMETRY_SOLVER_H
#define GEOMETRY_SOLVER_H

#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double x, y;
} Point;

typedef struct {
    double a, b, c; // ax + by = c
} Line;

typedef struct {
    double a, b, r; // (x-a)^2 + (y-b)^2 = r^2
} Circle;

typedef struct {
    int tool;
    int p1Index;
    int p2Index;
} Step;

typedef struct {
    Step* steps;
    int stepCount;
} Solution;

Solution solve_geometry_problem(
        const Point* initPoints, int initPointCount,
        const Line* initLines, int initLineCount,
        const Circle* initCircles, int initCircleCount,
        const Point* targetPoints, int targetPointCount,
        const Line* targetLines, int targetLineCount,
        const Circle* targetCircles, int targetCircleCount,
        const char* mode,
        int stepLimit
);

void free_solution(Solution solution);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRY_SOLVER_H