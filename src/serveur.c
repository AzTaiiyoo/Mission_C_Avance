#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "robot_app/robot.h"
#include "robot_app/robot_state.h"
#include "robot_app/robot_comm.h"

#define MAX_CLIENTS 5

static int server_socket;
static int client_socket = -1;
static volatile int running = 1;


void handle_signal(int signal) {
    running = 0;
}

void send_robot_state(RobotState* state) {
    if (client_socket == -1) return;
    
    robot_message_t message;
    robot_status_t status = robot_get_status();
    

    message.state = RobotState_get_state(state);
    message.left_sensor = status.left_sensor;
    message.center_sensor = status.center_sensor;
    message.right_sensor = status.right_sensor;
    message.battery_level = status.battery;
    strcpy(message.message, RobotState_get_state_name(state));
    
 
    send(client_socket, &message, sizeof(message), 0);
}

void process_client_command(RobotState* state, robot_message_t* command) {
   
    robot_event_t event;
    
    switch(command->state) {
        case STATE_MOVING_FORWARD:
            event = EV_FORWARD;
            break;
        case STATE_TURNING_LEFT:
            event = EV_TURN_LEFT;
            break;
        case STATE_TURNING_RIGHT:
            event = EV_TURN_RIGHT;
            break;
        case STATE_STOPPED:
            event = EV_STOP;
            break;
        default:
            return; 
    }
    
    RobotState_handle_event(state, event);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;
  
    signal(SIGINT, handle_signal);
    

    if (robot_start() != 0) {
        fprintf(stderr, "Erreur lors de l'initialisation du robot\n");
        return EXIT_FAILURE;
    }
    
    
    RobotState* robot_state = RobotState_new();
    if (!robot_state) {
        fprintf(stderr, "Erreur lors de la création de l'état du robot\n");
        robot_close();
        return EXIT_FAILURE;
    }
    
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur création socket");
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
    
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ROBOT_SERVER_PORT);
    
  
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_socket);
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
   
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur listen");
        close(server_socket);
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
    printf("Serveur robot démarré sur le port %d...\n", ROBOT_SERVER_PORT);
    
    
    while (running) {
        fd_set read_fds;
        struct timeval tv;
        int max_fd;
        
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_fd = server_socket;
        
        if (client_socket != -1) {
            FD_SET(client_socket, &read_fds);
            if (client_socket > max_fd) max_fd = client_socket;
        }
        
        
        tv.tv_sec = 0;
        tv.tv_usec = 100000; 
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        
        if (activity < 0 && running) {
            perror("select");
            continue;
        }
        
        
        if (FD_ISSET(server_socket, &read_fds)) {
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            if (client_socket < 0) {
                perror("accept");
                continue;
            }
            
            printf("Nouvelle connexion acceptée\n");
            
           
            send_robot_state(robot_state);
        }
        
        
        if (client_socket != -1 && FD_ISSET(client_socket, &read_fds)) {
            robot_message_t command;
            int bytes = recv(client_socket, &command, sizeof(command), 0);
            
            if (bytes <= 0) {
                
                printf("Client déconnecté\n");
                close(client_socket);
                client_socket = -1;
            }
            else {
                
                process_client_command(robot_state, &command);
                
               
                send_robot_state(robot_state);
            }
        }
        
    
        robot_status_t status = robot_get_status();
        
        
        if (status.center_sensor < 120 || status.left_sensor < 120 || status.right_sensor < 120) {
            RobotState_handle_event(robot_state, EV_OBSTACLE_DETECTED);
            send_robot_state(robot_state);
        }
        
        
        if (status.battery < 20) {
            RobotState_handle_event(robot_state, EV_BATTERY_LOW);
            send_robot_state(robot_state);
        }
    }
    
   
    if (client_socket != -1) {
        close(client_socket);
    }
    close(server_socket);
    RobotState_free(robot_state);
    robot_close();
    
    printf("Serveur robot arrêté\n");
    return EXIT_SUCCESS;
}