#include "robot_state.h"
#include "robot.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct robot_state_s {
    robot_state_type_t current_state;
};

typedef struct {
    robot_state_type_t next_state;
    void (*action)(void);
} transition_t;

static void action_start(void);
static void action_stop(void);
static void action_move_forward(void);
static void action_turn_left(void);
static void action_turn_right(void);
static void action_handle_obstacle(void);
static void action_handle_low_battery(void);
static void action_nop(void);


static transition_t transition_table[6][7] = {
    // Depuis STATE_IDLE
    [STATE_IDLE][EV_START] = {STATE_IDLE, action_start},
    [STATE_IDLE][EV_FORWARD] = {STATE_MOVING_FORWARD, action_move_forward},
    [STATE_IDLE][EV_TURN_LEFT] = {STATE_TURNING_LEFT, action_turn_left},
    [STATE_IDLE][EV_TURN_RIGHT] = {STATE_TURNING_RIGHT, action_turn_right},
    [STATE_IDLE][EV_STOP] = {STATE_STOPPED, action_stop},
    
    // Depuis STATE_MOVING_FORWARD
    [STATE_MOVING_FORWARD][EV_TURN_LEFT] = {STATE_TURNING_LEFT, action_turn_left},
    [STATE_MOVING_FORWARD][EV_TURN_RIGHT] = {STATE_TURNING_RIGHT, action_turn_right},
    [STATE_MOVING_FORWARD][EV_STOP] = {STATE_STOPPED, action_stop},
    [STATE_MOVING_FORWARD][EV_OBSTACLE_DETECTED] = {STATE_STOPPED, action_handle_obstacle},
    
    // Depuis STATE_TURNING_LEFT
    [STATE_TURNING_LEFT][EV_FORWARD] = {STATE_MOVING_FORWARD, action_move_forward},
    [STATE_TURNING_LEFT][EV_TURN_RIGHT] = {STATE_TURNING_RIGHT, action_turn_right},
    [STATE_TURNING_LEFT][EV_STOP] = {STATE_STOPPED, action_stop},
    [STATE_TURNING_LEFT][EV_OBSTACLE_DETECTED] = {STATE_STOPPED, action_handle_obstacle},
    
    // Depuis STATE_TURNING_RIGHT
    [STATE_TURNING_RIGHT][EV_FORWARD] = {STATE_MOVING_FORWARD, action_move_forward},
    [STATE_TURNING_RIGHT][EV_TURN_LEFT] = {STATE_TURNING_LEFT, action_turn_left},
    [STATE_TURNING_RIGHT][EV_STOP] = {STATE_STOPPED, action_stop},
    [STATE_TURNING_RIGHT][EV_OBSTACLE_DETECTED] = {STATE_STOPPED, action_handle_obstacle},
    
    // Depuis STATE_STOPPED
    [STATE_STOPPED][EV_START] = {STATE_IDLE, action_start},
    [STATE_STOPPED][EV_FORWARD] = {STATE_MOVING_FORWARD, action_move_forward},
    [STATE_STOPPED][EV_TURN_LEFT] = {STATE_TURNING_LEFT, action_turn_left},
    [STATE_STOPPED][EV_TURN_RIGHT] = {STATE_TURNING_RIGHT, action_turn_right},
    
    // Depuis STATE_ERROR
    [STATE_ERROR][EV_START] = {STATE_IDLE, action_start}
};

static const char* state_names[] = {
    "IDLE",
    "MOVING_FORWARD",
    "TURNING_LEFT",
    "TURNING_RIGHT",
    "STOPPED",
    "ERROR"
};

RobotState* RobotState_new(void) {
    RobotState* this = (RobotState*)calloc(1, sizeof(RobotState));
    if (this) {
        this->current_state = STATE_IDLE;
    }
    return this;
}

void RobotState_free(RobotState* this) {
    free(this);
}

void RobotState_handle_event(RobotState* this, robot_event_t event) {
    transition_t transition = transition_table[this->current_state][event];
    
    if (transition.action) {
        transition.action();
    }
    
    
    if (transition.next_state == 0 && this->current_state != 0) {
        
    } else {
        
        this->current_state = transition.next_state;
    }
}

robot_state_type_t RobotState_get_state(RobotState* this) {
    return this->current_state;
}

const char* RobotState_get_state_name(RobotState* this) {
    return state_names[this->current_state];
}


static void action_start(void) {
    robot_start();
}

static void action_stop(void) {
    robot_set_speed(0, 0);
}

static void action_move_forward(void) {
    robot_set_speed(30, 30);
}

static void action_turn_left(void) {
    robot_set_speed(-20, 20);
}

static void action_turn_right(void) {
    robot_set_speed(20, -20);
}

static void action_handle_obstacle(void) {
    robot_set_speed(0, 0);
    robot_signal_event(ROBOT_OBSTACLE);
}

static void action_handle_low_battery(void) {
    robot_signal_event(ROBOT_PROBLEM);
}

static void action_nop(void) {
    
}