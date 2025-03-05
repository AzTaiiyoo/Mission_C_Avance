#ifndef ROBOT_COMM_H
#define ROBOT_COMM_H

#include "robot_state.h"

#define ROBOT_SERVER_PORT 12346  // Modifié pour éviter le conflit avec le simulateur

// Structure à envoyer/recevoir
typedef struct {
    int state;               // État du robot (utilise les constantes de robot_state.h)
    int left_sensor;         // Valeur du capteur gauche
    int center_sensor;       // Valeur du capteur central
    int right_sensor;        // Valeur du capteur droit
    int battery_level;       // Niveau de batterie
    char message[32];        // Message supplémentaire
} robot_message_t;

#endif // ROBOT_COMM_H