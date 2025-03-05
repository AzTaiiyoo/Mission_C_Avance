#include "robot_state.h"
#include "robot.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Définition des constantes internes
#define NB_EVENT 7
#define NB_STATE 6

// Définitions privées des types
typedef enum {
    A_NOP = 0,
    A_START,
    A_STOP,
    A_MOVE_FORWARD,
    A_TURN_LEFT,
    A_TURN_RIGHT,
    A_HANDLE_OBSTACLE,
    A_HANDLE_LOW_BATTERY
} action_t;

// Structure cachée pour l'état du robot
struct robot_state_s {
    int current_state;
};

// Structure de transition
typedef struct {
    int next_state;
    action_t action;
} transition_t;

// Noms des états pour affichage/communication
static const char* state_names[] = {
    "IDLE",
    "MOVING_FORWARD",
    "TURNING_LEFT",
    "TURNING_RIGHT",
    "STOPPED",
    "ERROR"
};

// Prototypes des fonctions d'action privées
static void perform_action(action_t action);
static void action_start(void);
static void action_stop(void);
static void action_move_forward(void);
static void action_turn_left(void);
static void action_turn_right(void);
static void action_handle_obstacle(void);
static void action_handle_low_battery(void);
static void action_nop(void);

// Table de transition
static transition_t transition_table[NB_STATE][NB_EVENT] = {
    // STATE_IDLE
    [0] = {
        [0] = {0, A_START},               // EV_START
        [2] = {1, A_MOVE_FORWARD},        // EV_FORWARD -> STATE_MOVING_FORWARD
        [3] = {2, A_TURN_LEFT},           // EV_TURN_LEFT -> STATE_TURNING_LEFT
        [4] = {3, A_TURN_RIGHT},          // EV_TURN_RIGHT -> STATE_TURNING_RIGHT
        [1] = {4, A_STOP},                // EV_STOP -> STATE_STOPPED
        // autres transitions...
    },
    // STATE_MOVING_FORWARD
    [1] = {
        [1] = {4, A_STOP},                // EV_STOP -> STATE_STOPPED
        [5] = {4, A_HANDLE_OBSTACLE},     // EV_OBSTACLE_DETECTED -> STATE_STOPPED
        // autres transitions...
    },
    // Autres états...
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

void RobotState_handle_event(RobotState* this, int event) {
    // Vérification de validité
    if (event < 0 || event >= NB_EVENT || this->current_state < 0 || this->current_state >= NB_STATE) {
        return;
    }
    
    transition_t transition = transition_table[this->current_state][event];
    
    // Appliquer l'action
    perform_action(transition.action);
    
    // Mettre à jour l'état
    this->current_state = transition.next_state;
}

int RobotState_get_state(RobotState* this) {
    return this->current_state;
}

const char* RobotState_get_state_name(RobotState* this) {
    if (this->current_state >= 0 && this->current_state < NB_STATE) {
        return state_names[this->current_state];
    }
    return "UNKNOWN";
}

// Fonction d'exécution des actions
static void perform_action(action_t action) {
    switch(action) {
        case A_NOP:
            action_nop();
            break;
        case A_START:
            action_start();
            break;
        case A_STOP:
            action_stop();
            break;
        case A_MOVE_FORWARD:
            action_move_forward();
            break;
        case A_TURN_LEFT:
            action_turn_left();
            break;
        case A_TURN_RIGHT:
            action_turn_right();
            break;
        case A_HANDLE_OBSTACLE:
            action_handle_obstacle();
            break;
        case A_HANDLE_LOW_BATTERY:
            action_handle_low_battery();
            break;
        default:
            // Action inconnue, ne rien faire
            break;
    }
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