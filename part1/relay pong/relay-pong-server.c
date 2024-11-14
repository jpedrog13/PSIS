#include "lib.h"
#include "sock.h"


int main(){
    //Create Socket
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }

    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    int err = bind(sock_fd, (struct sockaddr *)&local_addr,
                    sizeof(local_addr));
    if(err == -1){
        perror("bind");
        exit(-1);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);

    //Declaration of useful variables
    message_t m_client, m_server;
    ball_position_t ball;
    client_info_t clients[100];

    int n_clients = 0;  //number of clients connected
    int nbytes;         //Debug variable to check number of bytes received

    do{
        //Waits for a message from a client
        nbytes = recvfrom(sock_fd, &m_client, sizeof(m_client), 0,
                (struct sockaddr *)&client_addr, &client_addr_size);
        printf("received %d bytes\n", nbytes);

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

                //Send Send_ball message to the recenlty connected client
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
        //Client has released the ball
        if(m_client.type  == 1){
            //Saves ball position            
            m_server.ball_pos = m_client.ball_pos;
            m_server.type = m_client.type;
            //If they're the only client, re-send the ball back
            if(n_clients == 1){
                clients[n_clients-1].play_state = 1;
                client_addr = clients[n_clients-1].client_addr;
                nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                        (const struct sockaddr *)&clients[n_clients-1].client_addr, client_addr_size);
            }
            //Otherwise, send the ball to someone else
            else{
                for(int i = 0; i < n_clients; i++){
                    if(clients[i].play_state == 1){
                        //If we're at the last client in the list, send to the first
                        if(i == n_clients - 1){
                            clients[i].play_state = 0;
                            clients[0].play_state = 1;
                            client_addr = clients[0].client_addr;
                            nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                                    (const struct sockaddr *)&clients[0].client_addr, client_addr_size);
                        }
                        //Otherwise, send to the next client in the list
                        else{
                            clients[i].play_state = 0;
                            clients[i+1].play_state = 1;
                            client_addr = clients[i+1].client_addr;
                            nbytes = sendto(sock_fd, &m_server, sizeof(m_server), 0,
                                    (const struct sockaddr *)&client_addr, client_addr_size);
                        }
                        break;
                    }
                }
            }
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
                        (const struct sockaddr *)&clients[i].client_addr, client_addr_size);
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
        }

    }while(1);

    return 0;
}