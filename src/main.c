/**
 * main program with copilot, pilot and robot modules
 */

 #include <signal.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <unistd.h>
 
 #include "robot_app/copilot.h"
 #include "robot_app/pilot.h"
 #include "robot_app/robot.h"
 
 /**
  * @mainpage Robot application mission 3.
  * This project aims to move a robot along a predefined trajectory using copilot.
  */
 
 /** @brief Waiting time between two encoder's scans (in microseconds) */
 #define DELAY 1000
 
 /** @brief Max scanning's attempt to check if a move is finished */
 #define ENCODERS_SCAN_NB 1000
 
 #define STEPS_NUMBER 6 /**< number of steps (moves) in the path */
 
 // declaration of private functions
 static void app_loop(void);
 
 /**
  * @brief Global variable used for program clean exit
  */
 static process_state_t running = ALIVE;
 
 /**
  * @brief Function for CTRL+C signal management
  */
 static void sigint_handler(int dummy) { running = STOPPED; }
 
 int main(void) {
     /* start the robot simulator and check its good starting */
     if (robot_start()) {
         printf("Pb lancement mrPizz\n");
         fflush(stdout);
         return EXIT_FAILURE;
     }
     printf("**** Version démo RM3 **** \n");
     printf("**** par JDL **** \n");
     printf("Ctrl+C pour quitter\n");
     fflush(stdout);
 
     /* Ctrl+C to stop the program. */
     signal(SIGINT, sigint_handler);
     /* main loop */
     app_loop();
     /* close the robot simulator */
     robot_close();
     return EXIT_SUCCESS;
 }
 
 void init_path(void) {
     move_t moves[STEPS_NUMBER] = {
         {FORWARD, {5}, 30},
         {ROTATION, {RIGHT}, 20},
         {FORWARD, {5}, 30},
         {ROTATION, {LEFT}, 20},
         {FORWARD, {5}, 30},
         {ROTATION, {U_TURN}, 20}
     };
     
     path_cfg_t path_config = {
         .moves = moves,
         .step_count = STEPS_NUMBER
     };
     
     set_path_config(path_config);
 }
 /**
  * @brief Main loop for the application.
  * Send commands to copilot and display robot's status with a specific period.
  */
 
 static void app_loop() {
     robot_status_t my_status;
     move_t user_move;
     move_status_t move_status;
     
     printf("Mode manuel activé\n");
     printf("Utilisez les flèches ou ZQSD pour contrôler le robot:\n");
     printf("↑ ou Z: Avancer\n");
     printf("← ou Q: Tourner à gauche\n");
     printf("→ ou D: Tourner à droite\n");
     printf("i: Augmenter la vitesse de déplacemenet\n");
     printf("o: Diminuer la vitesse de déplacement\n");
     printf("V: Verrouiller/Déverrouiller les commandes\n");
     printf("Échap ou Ctrl+C pour quitter\n");
     
     while (running) {
         
         my_status = robot_get_status();
         fprintf(stdout, "\rcodeurs: g = %d, d = %d | proxy: g = %d, c = %d, d = %d | batterie: %d %%",
                 my_status.left_encoder, my_status.right_encoder,
                 my_status.left_sensor, my_status.center_sensor, my_status.right_sensor,
                 my_status.battery);
         fflush(stdout);
         
        
         if (copilot_wait_user_input(&user_move)) {
             pilot_start_move(user_move);
             
             
             while (running) {
                 move_status = pilot_stop_at_target();
                 if (move_status == MOVE_DONE || move_status == MOVE_OBSTACLE_FORWARD) {
                     break;  
                 }
                 usleep(1000);
             }
         }
         
         usleep(10000);  
     }
     
     printf("\n");
 }