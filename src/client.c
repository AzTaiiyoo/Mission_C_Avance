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

// Gestionnaire de signal pour arrêt propre
void handle_signal(int signal) {
    running = 0;
}

// Configuration du mode terminal brut
void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Restauration du mode terminal
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Envoi d'une commande au serveur
void send_command(robot_state_type_t command) {
    robot_message_t message;
    memset(&message, 0, sizeof(message));
    message.state = command;
    
    send(client_socket, &message, sizeof(message), 0);
}

// Affichage de l'état du robot
void display_robot_state(const robot_message_t* state) {
    printf("\033[2J\033[H"); // Effacer l'écran et positionner le curseur en haut
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
    char server_ip[16] = "127.0.0.1"; // Adresse par défaut
    
    // Vérifier les arguments
    if (argc > 1) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
    }
    
    // Configurer gestionnaire de signal
    signal(SIGINT, handle_signal);
    
    // Activer le mode terminal brut
    enable_raw_mode();
    
    // Créer le socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erreur création socket");
        disable_raw_mode();
        return EXIT_FAILURE;
    }
    
    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ROBOT_SERVER_PORT);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Adresse invalide / non supportée");
        close(client_socket);
        disable_raw_mode();
        return EXIT_FAILURE;
    }
    
    // Connexion au serveur
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur connexion");
        close(client_socket);
        disable_raw_mode();
        return EXIT_FAILURE;
    }
    
    printf("Connecté au serveur robot (%s:%d)\n", server_ip, ROBOT_SERVER_PORT);
    
    // Boucle principale
    while (running) {
        struct timeval tv;
        int max_fd;
        
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);
        max_fd = (STDIN_FILENO > client_socket) ? STDIN_FILENO : client_socket;
        
        // Configurer le timeout
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        
        if (activity < 0 && running) {
            perror("select");
            continue;
        }
        
        // Entrée utilisateur
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
        
        // Données du serveur
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
    
    // Nettoyage
    close(client_socket);
    disable_raw_mode();
    
    printf("\nClient robot arrêté\n");
    return EXIT_SUCCESS;
}