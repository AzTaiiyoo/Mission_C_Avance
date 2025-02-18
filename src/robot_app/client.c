#include "client.h"
typedef enum {
    NONE = 0,
    FORWARD,
    LEFT,
    RIGHT,
    U_TURN,
} action_t;

struct client_s {
    int socket;
    action_t action;
    char* message;  
}

extern Client_new(void){
    client_t* this = (client_t*)calloc(1, sizeof(client_t));
    this->socket = 0;
    this->action = NONE;
    this->message = "";
}
