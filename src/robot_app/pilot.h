#ifndef PILOT_H
#define PILOT_H
#include "robot.h"

typedef enum{
    MOVE_FORWARD,
    MOVE_ROTATION,
    MOVE_DONE,
    MOVE_OBSTACLE_FORWARD
} move_status_t;

typedef enum{
    FORWARD,
    ROTATION
} move_type_t;

typedef enum{
    RIGHT,
    LEFT,
    U_TURN
} angle_t;

typedef union{
    int distance;
    angle_t angle;
} range_t;

typedef struct{
    move_type_t move_type;
    range_t range;
    speed_pct_t speed;
} move_t;

void pilot_start_move(move_t a_move);
move_status_t pilot_stop_at_target(void);

#endif // PILOT_H