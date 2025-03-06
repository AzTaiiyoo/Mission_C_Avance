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

static int server_socket;
static int client_socket = -1;
static volatile int running = 1;

// Gestionnaire de signal pour arrêt propre
void handle_signal(int signal) {
    running = 0;
    printf("\nSignal d'interruption reçu. Arrêt en cours...\n");
    close(server_socket);
}

void send_robot_state(RobotState* state) {
    if (client_socket == -1) return;
    
    robot_message_t message;
    robot_status_t status = robot_get_status();
    
    // Remplir la structure de message
    message.state = RobotState_get_state(state);
    message.left_sensor = status.left_sensor;
    message.center_sensor = status.center_sensor;
    message.right_sensor = status.right_sensor;
    message.battery_level = status.battery;
    strcpy(message.message, RobotState_get_state_name(state));
    
    // Envoyer au client
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
            return; // Ignorer les commandes inconnues
    }
    
    RobotState_handle_event(state, event);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;
    
    // Configurer gestionnaire de signal
    signal(SIGINT, handle_signal);
    
    // Initialiser le robot
    if (robot_start() != 0) {
        fprintf(stderr, "Erreur lors de l'initialisation du robot\n");
        return EXIT_FAILURE;
    }
    
    // Créer l'objet d'état du robot
    RobotState* robot_state = RobotState_new();
    if (!robot_state) {
        fprintf(stderr, "Erreur lors de la création de l'état du robot\n");
        robot_close();
        return EXIT_FAILURE;
    }
    
    // Créer le socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur création socket");
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
    // Configuration pour réutiliser l'adresse
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ROBOT_SERVER_PORT);
    
    // Lier le socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_socket);
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
    // Écouter les connexions
    if (listen(server_socket, 1) < 0) { // Un seul client en attente
        perror("Erreur listen");
        close(server_socket);
        RobotState_free(robot_state);
        robot_close();
        return EXIT_FAILURE;
    }
    
    printf("Serveur robot démarré sur le port %d...\n", ROBOT_SERVER_PORT);
    printf("Appuyez sur Ctrl+C pour quitter\n");
    
    // Boucle principale du serveur
    while (running) {
        // Attendre une connexion client
        printf("En attente d'une connexion client...\n");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (!running) break; // Si interruption pendant accept()
            perror("Erreur accept");
            continue;
        }
        
        printf("Client connecté\n");
        
        // Envoyer l'état initial au client
        send_robot_state(robot_state);
        
        // Boucle de communication avec le client
        while (running) {
            robot_message_t command;
            robot_status_t status;
            int recv_result;
            
            // Configurer un timeout sur la réception
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            
            // Recevoir une commande du client
            recv_result = recv(client_socket, &command, sizeof(command), 0);
            
            if (recv_result > 0) {
                // Commande reçue, la traiter
                process_client_command(robot_state, &command);
                send_robot_state(robot_state);
            } 
            else if (recv_result == 0) {
                // Connexion fermée par le client
                printf("Client déconnecté\n");
                close(client_socket);
                client_socket = -1;
                break;
            } 
            else {
                // Erreur ou timeout sur recv()
                if (!running) break;
                
                // Vérifier l'état du robot pendant les timeouts
                status = robot_get_status();
                
                // Détecter les obstacles
                if (status.center_sensor < 120 || status.left_sensor < 120 || status.right_sensor < 120) {
                    RobotState_handle_event(robot_state, EV_OBSTACLE_DETECTED);
                    send_robot_state(robot_state);
                }
                
                // Détecter batterie faible
                if (status.battery < 20) {
                    RobotState_handle_event(robot_state, EV_BATTERY_LOW);
                    send_robot_state(robot_state);
                }
            }
        }
        
        // Si on sort de la boucle client, fermer le socket client
        if (client_socket != -1) {
            close(client_socket);
            client_socket = -1;
        }
    }
    
    // Nettoyage
    if (client_socket != -1) {
        close(client_socket);
    }
    close(server_socket);
    RobotState_free(robot_state);
    robot_close();
    
    printf("Serveur robot arrêté\n");
    return EXIT_SUCCESS;
}