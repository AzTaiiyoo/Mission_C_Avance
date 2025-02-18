#ifndef COPILOT_H
#define COPILOT_H

#include "pilot.h"
#include <stdbool.h>

typedef enum {
    PATH_DONE,
    PATH_MOVING,
    PATH_OBSTACLE,
    PATH_ERROR
} path_status_t;

typedef struct {
    move_t* moves;     // Tableau des mouvements
    int step_count;    // Nombre d'étapes
} path_cfg_t;

// Configure le path à suivre
void set_path_config(path_cfg_t config);
// Attend une entrée de l'utilisateur
bool copilot_wait_user_input(move_t *user_move);

void copilot_start_path(void);
path_status_t copilot_stop_at_step_completion(void);
bool copilot_is_path_complete(void);

#endif // COPILOT_H