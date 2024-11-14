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
    message_c_t m_client;
    message_s_t m_server;
    m_server.clients[10];
    ball_position_t ball;
    paddle_position_t paddle_pos;
    ball_position_t temp_ball;


    int n_clients = 0;
    int total_clients = 0;
    int nbytes;

    nbytes = recvfrom(sock_fd, &m_client, sizeof(m_client), 0,
                (struct sockaddr *)&client_addr, &client_addr_size);
    int n;
    //Infinite loop to listen to client messages
    do{
        //If we get a connection message
        if(m_client.type == 0){
            //If server is full, reject client
            if(n_clients >= 10){
                m_server.index = -1;                    //Tell the client that the server is full
                sendto(sock_fd, &m_server, sizeof(m_server), 0,
                        (const struct sockaddr *)&client_addr, client_addr_size);
            }
            else{
                //If this is the first client, place a pseudo-random ball
                if(n_clients == 0){
                    place_ball_random(&m_server.ball_pos);
                }
                //Update client info
                m_server.index = total_clients;
                m_server.clients[n_clients].index = total_clients;
                m_server.clients[n_clients].score = 0;
                random_paddle(&m_server.clients[n_clients].paddle_pos, m_server.clients, n_clients, PADDLE_SIZE);
                m_server.clients[n_clients].client_addr = client_addr;
                n_clients++;
                m_server.n_clients = n_clients;
                temp_ball = m_server.ball_pos;
                sendto(sock_fd, &m_server, sizeof(m_server), 0,
                        (const struct sockaddr *)&client_addr, client_addr_size);
                total_clients++;
            }
        }
        //If we receive a Paddle_move message, update paddle positions update ball position
        else if(m_client.type == 1){
            n++;
            for(int i = 0; i < n_clients; i++){
                if(m_client.index == m_server.clients[i].index){
                    //Check if it hits any other paddle while moving, if it does, don't move
                    paddle_pos = m_server.clients[i].paddle_pos;
                    move_paddle(&paddle_pos, m_server.clients, n_clients, m_client.paddle_dir);
                    move_ball(&temp_ball, paddle_pos, m_client.index, m_server.clients, m_client.paddle_dir, n_clients);
                    m_server.clients[i].paddle_pos = paddle_pos;
                    //The server only sends the updated ball position to the client after n
                    //Paddle_move messages received
                    if(n >= n_clients){
                        n = 0;
                        m_server.ball_pos = temp_ball;
                        sendto(sock_fd, &m_server, sizeof(m_server), 0,
                            (const struct sockaddr *)&m_server.clients[i].client_addr, client_addr_size);
                    }
                    //Otherwise, send updated paddle positions
                    else{
                        sendto(sock_fd, &m_server, sizeof(m_server), 0,
                            (const struct sockaddr *)&m_server.clients[i].client_addr, client_addr_size);
                    }
                    break;
                }
            }
        }
        else if(m_client.type == 3){
            //Received disconnect message
            int dc = -1;
            for (int i = 0; i < n_clients; i++){
                if(m_client.index == m_server.clients[i].index){
                    dc = i;
                    break;
                }
            }
            //Delete from array
            if (dc > -1){
                for (int j = dc; j < n_clients; j++){
                    m_server.clients[j] = m_server.clients[j+1];
                }
                n_clients--;
                m_server.n_clients = n_clients;
            }
        }
        nbytes = recvfrom(sock_fd, &m_client, sizeof(m_client), 0,
                    (struct sockaddr *)&client_addr, &client_addr_size);
        
    }while(1);
    return 0;
}

//Defunct code for lists
/*insert_last(&head, 0, 10, client_addr);
insert_last(&head, 1, 20, client_addr);
printf("mid-insert\n");
insert_last(&head, 2, 30, client_addr);
insert_last(&head, 3, 40, client_addr);
aux = head;

while(aux != NULL){
    printf("index: %d \tscore: %d\n", aux->index, aux->score);
    aux = aux->next;
}*/