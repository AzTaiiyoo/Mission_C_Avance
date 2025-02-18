#ifndef ROBOT_H
#define ROBOT_H

/**
* @file robot.h
* @brief Interface for controlling a robot, providing basic movement and sensor operations.
* @author Your Name
* @date January 2024
*/

/**
* @brief Enumeration to identify robot wheels
*/
typedef enum {
   LEFT_WHEEL,    /**< Left wheel identifier */
   RIGHT_WHEEL    /**< Right wheel identifier */
} wheel_t;

/**
* @brief Structure containing the complete status of the robot
*/
typedef struct {
   int left_encoder;    /**< Left wheel encoder position */
   int right_encoder;   /**< Right wheel encoder position */ 
   int left_sensor;     /**< Left proximity sensor value (0-255) */
   int center_sensor;   /**< Center proximity sensor value (0-255) */
   int right_sensor;    /**< Right proximity sensor value (0-255) */
   int battery;         /**< Battery level in percentage */
} robot_status_t;

/**
* @brief Process state enumeration for program control
*/
typedef enum { 
   STOPPED = 0,   /**< Process should stop */
   ALIVE         /**< Process is running */
} process_state_t;

/**
* @brief Movement state enumeration for robot control
*/
typedef enum { 
   IDLE,    /**< Robot is not moving */
   ACTING   /**< Robot is executing a movement */
} move_state_t;

/**
* @brief Robot status notification types for LED signaling
*/
typedef enum {
   ROBOT_IDLE,      /**< Robot is inactive */
   ROBOT_OK,        /**< Robot is operating normally */
   ROBOT_OBSTACLE,  /**< Robot has detected an obstacle */
   ROBOT_PROBLEM    /**< Robot has encountered a problem */
} notification_t;

/**
* @brief Type definition for motor speed as percentage
* @details Speed is expressed as a percentage from -100 to +100
*          where negative values indicate reverse direction
*/
typedef int speed_pct_t;

/**
* @brief Initializes and starts the robot
* @return 0 on success, -1 on failure
* @details Initializes all robot systems including motors, sensors,
*          and communication interfaces
*/
int robot_start(void);

/**
* @brief Sets the speed of the robot's wheels
* @param left Speed percentage for left wheel (-100 to +100)
* @param right Speed percentage for right wheel (-100 to +100)
* @details Positive values move the wheel forward, negative values move it backward,
*          0 stops the wheel
*/
void robot_set_speed(speed_pct_t left, speed_pct_t right);

/**
* @brief Gets the position of a specific wheel's encoder
* @param wheel_id Identifier of the wheel (LEFT_WHEEL or RIGHT_WHEEL)
* @return Current encoder position
* @details Encoder values increment positively for forward movement
*          and negatively for backward movement
*/
int robot_get_wheel_position(wheel_t wheel_id);

/**
* @brief Resets the position counters of both wheels to zero
* @details This function should be called when starting a new movement sequence
*          to prevent encoder overflow
*/
void robot_reset_wheel_pos(void);

/**
* @brief Gets the current comprehensive status of the robot
* @return Structure containing all sensor values and robot state information
* @details Includes encoder positions, proximity sensor values,
*          and battery level
*/
robot_status_t robot_get_status(void);

/**
* @brief Signals a robot state or event using the LED
* @param event The type of notification to signal
* @details Uses different LED colors to indicate:
*          - OFF: Robot idle
*          - GREEN: Normal operation
*          - RED: Obstacle detected
*          - BLUE: Problem encountered
*/
void robot_signal_event(notification_t event);

/**
* @brief Stops the robot and closes all connections
* @details Stops all motors, turns off LED, and closes communication
*          interfaces cleanly
*/
void robot_close(void);

#endif // ROBOT_H