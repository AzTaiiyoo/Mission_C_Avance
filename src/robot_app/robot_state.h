#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

typedef struct robot_state_s RobotState;

typedef enum {
    EV_START,
    EV_STOP,
    EV_FORWARD,
    EV_TURN_LEFT,
    EV_TURN_RIGHT,
    EV_OBSTACLE_DETECTED,
    EV_BATTERY_LOW
} robot_event_t;

typedef enum {
    STATE_IDLE,
    STATE_MOVING_FORWARD,
    STATE_TURNING_LEFT,
    STATE_TURNING_RIGHT,
    STATE_STOPPED,
    STATE_ERROR
} robot_state_type_t;

extern RobotState* RobotState_new(void);

extern void RobotState_free(RobotState* this);

extern void RobotState_handle_event(RobotState* this, robot_event_t event);

extern robot_state_type_t RobotState_get_state(RobotState* this);

extern const char* RobotState_get_state_name(RobotState* this);

#endif // ROBOT_STATE_H