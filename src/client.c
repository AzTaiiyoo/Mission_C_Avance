#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <termios.h>
#include "robot_app/robot_comm.h"

static int client_socket;
static volatile int running = 1;
static struct termios orig_termios;


void handle_signal(int signal) {
    running = 0;
}


void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}


void send_command(robot_state_type_t command) {
    robot_message_t message;
    memset(&message, 0, sizeof(message));
    message.state = command;
    
    send(client_socket, &message, sizeof(message), 0);
}


void display_robot_state(const robot_message_t* state) {
    printf("\033[2J\033[H"); 
    printf("État du robot : %s\n", state->message);
    printf("Capteurs : Gauche=%d, Centre=%d, Droite=%d\n", 
           state->left_sensor, state->center_sensor, state->right_sensor);
    printf("Batterie : %d%%\n", state->battery_level);
    printf("\n");
    printf("Commandes :\n");
    printf("  Z - Avancer\n");
    printf("  Q - Tourner à gauche\n");
    printf("  D - Tourner à droite\n");
    printf("  S - Arrêter\n");
    printf("  Esc - Quitter\n");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
    fd_set read_fds;
    char buffer[1024];
    char server_ip[16] = "127.0.0.1"; 
    
   
    if (argc > 1) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
    }
    
   
    signal(SIGINT, handle_signal);
    
    
    enable_raw_mode();
    
    
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erreur création socket");
        disable_raw_mode();
        return EXIT_FAILURE;
    }
    
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ROBOT_SERVER_PORT);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Adresse invalide / non supportée");
        close(client_socket);
        disable_raw_mode();
        return EXIT_FAILURE;
    }
    
    
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur connexion");
        close(client_socket);
        disable_raw_mode();
        return EXIT_FAILURE;
    }
    
    printf("Connecté au serveur robot (%s:%d)\n", server_ip, ROBOT_SERVER_PORT);
    
    
    while (running) {
        struct timeval tv;
        int max_fd;
        
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);
        max_fd = (STDIN_FILENO > client_socket) ? STDIN_FILENO : client_socket;
        
        
        tv.tv_sec = 0;
        tv.tv_usec = 100000; 
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        
        if (activity < 0 && running) {
            perror("select");
            continue;
        }
        
        
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                switch (c) {
                    case 'z':
                    case 'Z':
                        send_command(STATE_MOVING_FORWARD);
                        break;
                    case 'q':
                    case 'Q':
                        send_command(STATE_TURNING_LEFT);
                        break;
                    case 'd':
                    case 'D':
                        send_command(STATE_TURNING_RIGHT);
                        break;
                    case 's':
                    case 'S':
                        send_command(STATE_STOPPED);
                        break;
                    case 27: // Échap
                        running = 0;
                        break;
                }
            }
        }
        
       
        if (FD_ISSET(client_socket, &read_fds)) {
            robot_message_t state;
            int bytes = recv(client_socket, &state, sizeof(state), 0);
            
            if (bytes <= 0) {
                printf("Serveur déconnecté\n");
                running = 0;
            }
            else {
                display_robot_state(&state);
            }
        }
    }
    
    
    close(client_socket);
    disable_raw_mode();
    
    printf("\nClient robot arrêté\n");
    return EXIT_SUCCESS;
}