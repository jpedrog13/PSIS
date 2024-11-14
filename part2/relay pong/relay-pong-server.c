#include "lib.h"
#include "sock.h"

//Create Socket
int sock_fd;

struct sockaddr_in local_addr;
struct sockaddr_in client_addr;
socklen_t client_addr_size;

//Declaration of useful variables
message_t m_client, m_server;
ball_position_t ball;
client_info_t clients[100];

int n_clients = 0;  //Number of clients connected
int nbytes;         //Debug variable to check number of bytes received

pthread_mutex_t m1;

//Thread responsible of changing the active player every 10 seconds
void* handle_clients(void *arg){
    while(1){
        //Otherwise, send the ball to someone else
        sleep(10);
        printf("-----------------------\n");
        printf("number clients: %d\n", n_clients);

        if(n_clients > 1){ //Checks if there's more than 1 player, otherwise no need to change
            for(int i = 0; i < n_clients; i++){
                //Finding the active client
                if(clients[i].play_state == 1){
                    //Release ball message
                    m_server.type = 4;
                    if (nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                            (const struct sockaddr *)&clients[i].client_addr, client_addr_size))
                        printf("--->sent: release\n");

                    printf("stop playing: %d | message: %d\n", i+1, m_server.type);
                    printf("port: %u\n", clients[i].client_addr.sin_port);

                    //Send ball message
                    m_server.type = 1;
                    //If we're at the last client in the list, send to the first
                    if(i == n_clients - 1){

                        clients[i].play_state = 0;
                        clients[0].play_state = 1;
                        client_addr = clients[0].client_addr;
                        if (nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                                (const struct sockaddr *)&client_addr, client_addr_size))
                            printf("--->sent: send ball\n");

                        printf("now playing: %d | message: %d\n", i-n_clients+2, m_server.type);
                        printf("port: %u\n", client_addr.sin_port);
                    }
                    //Otherwise, send ball to the next client in the list
                    else{

                        clients[i].play_state = 0;
                        clients[i+1].play_state = 1;
                        client_addr = clients[i+1].client_addr;
                        if (nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                                (const struct sockaddr *)&client_addr, client_addr_size))
                            printf("--->sent: send ball\n");

                        printf("now playing: %d | message: %d\n", i+2, m_server.type);
                        printf("port: %u\n", client_addr.sin_port);
                    }
                    break;
                }
            }
        }                      
    }
    return NULL;
}

int main(){
    
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    int err = bind(sock_fd, (struct sockaddr *)&local_addr,
                    sizeof(local_addr));
    if(err == -1){
        perror("bind");
        exit(-1);
    }

    client_addr_size = sizeof(struct sockaddr_in);

    pthread_mutex_init(&m1, NULL);
    
    //Declare threads
    pthread_t clients_thread_id;
    pthread_create(&clients_thread_id, NULL, handle_clients, NULL);

    do{
        //Waits for a message from a client
        nbytes = recvfrom(sock_fd, &m_client, sizeof(m_client), 0,
                (struct sockaddr *)&client_addr, &client_addr_size);

        //Client wants to connect
        if(m_client.type == 0){
            //If this is the first client
            if(n_clients == 0){               
                //Place a pseudo-random ball and send it
                place_ball_random(&ball);
                m_server.ball_pos = ball;
                m_server.type = 1;
                clients[n_clients].play_state = 1;
                clients[n_clients].client_addr = client_addr;

                //Send Send_ball message to the recently connected client
                nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                    (const struct sockaddr *)&clients[n_clients].client_addr, client_addr_size);
            }
            //Otherwise, send the current ball position
            else{               
                m_server.type = 2;
                clients[n_clients].play_state = 0;
                clients[n_clients].client_addr = client_addr;
                //Send Send_ball message to the recenlty connected client
                nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                    (const struct sockaddr *)&clients[n_clients].client_addr, client_addr_size);
            }
            n_clients++;
        }   
        //The client has moved the ball
        if(m_client.type == 2){
            //Saves ball position
            m_server.ball_pos = m_client.ball_pos;
            m_server.type = 2;
            //Sends new ball position to all connected clients
            for(int i = 0; i < n_clients; i++){
                client_addr = clients[i].client_addr;
                if(clients[i].play_state == 0){
                    nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                        (const struct sockaddr *)&client_addr, client_addr_size);
                    printf("----->MOVE: %u\n", client_addr.sin_port);
                }
            }
        }
        //The client has disconnected
        if(m_client.type == 3){
            int dc = -1;
            for (int i = 0; i < n_clients; i++){
                if(clients[i].client_addr.sin_port == client_addr.sin_port && clients[i].client_addr.sin_addr.s_addr == client_addr.sin_addr.s_addr){
                    dc = i;
                    break;
                }
            }
            //Deletes client from ""list""
            if (dc > -1){
                for (int j = dc; j < n_clients; j++){
                    clients[j] = clients[j+1];
                }
                n_clients--;
            }
            //If there's only one client or if it's the last client on the list
            //Send ball to the first client
            if (n_clients == 1 || dc==n_clients){
                clients[0].play_state=1;
                client_addr = clients[0].client_addr;
                m_server.type = 1;
                nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                    (const struct sockaddr *)&client_addr, client_addr_size);            
            }
            //Otherwise send ball to the next client on the list
            else{
                clients[dc].play_state=1;
                client_addr = clients[dc].client_addr;
                m_server.type = 1;
                nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                    (const struct sockaddr *)&client_addr, client_addr_size);
            }
        }
    }while(1);
    return 0;
}