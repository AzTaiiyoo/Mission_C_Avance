// robot.c

#include "robot.h"
#include "mrpiz.h"
#include <errno.h>
#include <stdio.h>

int robot_start(void) {
  int result = 0;
  if (mrpiz_init() == -1) {
    // fprintf(stderr, "Error initializing the robot\n");
    result = -1;
  }
  else {
  robot_signal_event(ROBOT_IDLE);
  }

  return result;
}

void robot_set_speed(speed_pct_t left, speed_pct_t right) {
   // @todo
   int left_status;
   int right_status;

  if (left > 100 || left < -100 || right > 100 || right < -100) {
    // fprintf(stderr, "Invalid speed values\n");
    robot_signal_event(ROBOT_PROBLEM);
    return;
  }

  left_status = mrpiz_motor_set(MRPIZ_MOTOR_LEFT, left);
  right_status = mrpiz_motor_set(MRPIZ_MOTOR_RIGHT, right);

  if (left_status == -1 || right_status == -1) {
    robot_signal_event(ROBOT_PROBLEM);
  }
  else {
    robot_signal_event(ROBOT_OK);
  }
}


int robot_get_wheel_position(wheel_t wheel_id) {
    return mrpiz_motor_encoder_get(
        (wheel_id == LEFT_WHEEL) ? MRPIZ_MOTOR_LEFT : MRPIZ_MOTOR_RIGHT
    );
}

void robot_reset_wheel_pos(void) {
  // @todo
  mrpiz_motor_encoder_reset(MRPIZ_MOTOR_BOTH);
}

robot_status_t robot_get_status(void) {
  // @todo
  robot_status_t status;
  //reading the values of the sensors and encoders
  status.left_encoder = mrpiz_motor_encoder_get(MRPIZ_MOTOR_LEFT);
  status.right_encoder = mrpiz_motor_encoder_get(MRPIZ_MOTOR_RIGHT);

  //reading the values of the sensors
  status.left_sensor = mrpiz_proxy_sensor_get(MRPIZ_PROXY_SENSOR_FRONT_LEFT);
  status.center_sensor = mrpiz_proxy_sensor_get(MRPIZ_PROXY_SENSOR_FRONT_CENTER);
  status.right_sensor = mrpiz_proxy_sensor_get(MRPIZ_PROXY_SENSOR_FRONT_RIGHT);

  //reading the battery level
  status.battery = mrpiz_battery_voltage();

  if (status.left_sensor > 200 || status.right_sensor > 200 || status.center_sensor > 200) {
    robot_signal_event(ROBOT_OBSTACLE);
  }

  if (status.battery < 20) {
    robot_signal_event(ROBOT_PROBLEM);
  }
return status;
}

void robot_signal_event(notification_t event) {
  // @todo
  switch (event) {
    case ROBOT_IDLE:
      mrpiz_led_rgb_set(MRPIZ_LED_OFF);
      break;
    case ROBOT_OK:
      mrpiz_led_rgb_set(MRPIZ_LED_GREEN);
      break;
    case ROBOT_OBSTACLE:
      mrpiz_led_rgb_set(MRPIZ_LED_RED);
      // printf("Obstacle detected\n");
      break;
    case ROBOT_PROBLEM:
    mrpiz_led_rgb_set(MRPIZ_LED_BLUE);
    // printf("Battery level is low : %d\n", mrpiz_battery_voltage());
  }
}

void robot_close(void) {
  robot_set_speed(0, 0);
  mrpiz_close();
}
