#include "robot_state.h"
#include "robot.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Structure cachée pour l'état du robot
struct robot_state_s {
    robot_state_type_t current_state;
};

// Tableau de transitions d'état
typedef struct {
    robot_state_type_t next_state;
    void (*action)(void);
} transition_t;

// Actions possibles
static void action_start(void);
static void action_stop(void);
static void action_move_forward(void);
static void action_turn_left(void);
static void action_turn_right(void);
static void action_handle_obstacle(void);
static void action_handle_low_battery(void);
static void action_nop(void);

// Table de transition
static transition_t transition_table[6][7] = {
    // STATE_IDLE
    [STATE_IDLE] = {
        [EV_START] = {STATE_IDLE, action_start},
        [EV_FORWARD] = {STATE_MOVING_FORWARD, action_move_forward},
        [EV_TURN_LEFT] = {STATE_TURNING_LEFT, action_turn_left},
        [EV_TURN_RIGHT] = {STATE_TURNING_RIGHT, action_turn_right},
        // autres transitions...
    },
    // Autres états...
};

// Noms des états pour affichage/communication
static const char* state_names[] = {
    "IDLE",
    "MOVING_FORWARD",
    "TURNING_LEFT",
    "TURNING_RIGHT",
    "STOPPED",
    "ERROR"
};

// Implémentation des fonctions
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
    
    // Appliquer l'action si définie
    if (transition.action) {
        transition.action();
    }
    
    // Mettre à jour l'état
    this->current_state = transition.next_state;
}

robot_state_type_t RobotState_get_state(RobotState* this) {
    return this->current_state;
}

const char* RobotState_get_state_name(RobotState* this) {
    return state_names[this->current_state];
}

// Implémentation des actions
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
    // Ne rien faire
}