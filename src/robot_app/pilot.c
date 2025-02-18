#include "pilot.h"
#include "robot.h"
#include "../utils.h"
#include <stdio.h>
#include <stdbool.h>

#define DISTANCE_FORWARD 30
#define DISTANCE_TURN 30
#define OBSTACLE_THRESHOLD 120

static move_status_t robot_moving = MOVE_DONE;
static int target_pos;
static wheel_t reference_wheel;

static move_t previous_move;

void pilot_start_move(move_t a_move) {
    TRACE("Starting move: type=%d, distance/angle=%d, speed=%d\n",
          a_move.move_type, a_move.range.distance, a_move.speed);
    
    // Sauvegarde du mouvement précédent
    previous_move = a_move;
    
    robot_reset_wheel_pos(); 
    
    switch(a_move.move_type) {
        case FORWARD:
            TRACE("Forward movement\n");
            reference_wheel = LEFT_WHEEL;
            target_pos = robot_get_wheel_position(reference_wheel) + 
                        a_move.range.distance * DISTANCE_FORWARD;
            robot_set_speed(a_move.speed, a_move.speed);
            robot_moving = MOVE_FORWARD;
            break;
            
        case ROTATION:
            TRACE("Rotation movement\n");
            switch(a_move.range.angle) {
                case RIGHT:
                    reference_wheel = LEFT_WHEEL;
                    target_pos = robot_get_wheel_position(reference_wheel) + DISTANCE_TURN;
                    robot_set_speed(a_move.speed, -a_move.speed);
                    break;
                case LEFT:
                    reference_wheel = RIGHT_WHEEL;
                    target_pos = robot_get_wheel_position(reference_wheel) + DISTANCE_TURN;
                    robot_set_speed(-a_move.speed, a_move.speed);
                    break;
                case U_TURN:
                    reference_wheel = RIGHT_WHEEL;
                    target_pos = robot_get_wheel_position(reference_wheel) + DISTANCE_TURN * 2;
                    robot_set_speed(-a_move.speed, a_move.speed);
                    break;
            }
            robot_moving = MOVE_ROTATION;
            break;
    }
}

move_status_t pilot_stop_at_target(void) {
    int current_pos = robot_get_wheel_position(reference_wheel);
    robot_status_t status = robot_get_status();
    bool obstacle_before = robot_moving == MOVE_OBSTACLE_FORWARD;
    
    TRACE("Encoder position: %d, Target: %d, Moving status: %d\n", 
          current_pos, target_pos, robot_moving);

    
    if ((robot_moving == MOVE_FORWARD && status.center_sensor < OBSTACLE_THRESHOLD) || 
        status.left_sensor < OBSTACLE_THRESHOLD || 
        status.right_sensor < OBSTACLE_THRESHOLD) {
        TRACE("Obstacle detected, stopping robot\n");
        robot_set_speed(0, 0);
        return MOVE_OBSTACLE_FORWARD;
    }
    
    
    if ((obstacle_before && status.center_sensor >= OBSTACLE_THRESHOLD) || 
        (obstacle_before && status.left_sensor >= OBSTACLE_THRESHOLD) || 
        (obstacle_before && status.right_sensor >= OBSTACLE_THRESHOLD)) {
        if (previous_move.move_type == FORWARD) {
            robot_moving = MOVE_DONE;
            TRACE("Obstacle cleared, resuming previous move\n");
            pilot_start_move(previous_move);
            return robot_moving;
        }
    }
    
    
    if (current_pos >= target_pos) {
        TRACE("Target reached\n");
        robot_set_speed(0, 0);
        return MOVE_DONE;
    }
    
    return robot_moving;
}