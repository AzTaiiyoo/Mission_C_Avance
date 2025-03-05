#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

// Structure opaque pour l'état du robot
typedef struct robot_state_s RobotState;

// Définition des constantes d'événements (en tant que macros)
#define EV_START              0
#define EV_STOP               1
#define EV_FORWARD            2
#define EV_TURN_LEFT          3
#define EV_TURN_RIGHT         4
#define EV_OBSTACLE_DETECTED  5
#define EV_BATTERY_LOW        6

// Définition des constantes d'états (en tant que macros)
#define STATE_IDLE            0
#define STATE_MOVING_FORWARD  1
#define STATE_TURNING_LEFT    2
#define STATE_TURNING_RIGHT   3
#define STATE_STOPPED         4
#define STATE_ERROR           5

// Déclaration des fonctions externes
extern RobotState* RobotState_new(void);
extern void RobotState_free(RobotState* this);
extern void RobotState_handle_event(RobotState* this, int event);
extern int RobotState_get_state(RobotState* this);
extern const char* RobotState_get_state_name(RobotState* this);

#endif // ROBOT_STATE_H