#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

// Structure opaque pour l'état du robot
typedef struct robot_state_s RobotState;

// Types d'événements possibles
typedef enum {
    EV_START,
    EV_STOP,
    EV_FORWARD,
    EV_TURN_LEFT,
    EV_TURN_RIGHT,
    EV_OBSTACLE_DETECTED,
    EV_BATTERY_LOW
} robot_event_t;

// États possibles du robot
typedef enum {
    STATE_IDLE,
    STATE_MOVING_FORWARD,
    STATE_TURNING_LEFT,
    STATE_TURNING_RIGHT,
    STATE_STOPPED,
    STATE_ERROR
} robot_state_type_t;

// Crée un nouvel état robot
extern RobotState* RobotState_new(void);

// Libère l'état robot
extern void RobotState_free(RobotState* this);

// Traite un événement et met à jour l'état
extern void RobotState_handle_event(RobotState* this, robot_event_t event);

// Obtient l'état actuel du robot
extern robot_state_type_t RobotState_get_state(RobotState* this);

// Obtient une représentation textuelle de l'état
extern const char* RobotState_get_state_name(RobotState* this);

#endif // ROBOT_STATE_H