#include "lib.h"
#include "sock.h"

int sock_fd;
int client_fd;
int client_fd_list[10];
struct sockaddr_in local_addr;
struct sockaddr_in client_addr;
socklen_t client_addr_size;
socklen_t local_addr_size;

pthread_mutex_t m1;

int n_clients = 0;
int total_clients = 0;
int nbytes;

//Declaration of useful variables
message_c_t m_client;
message_s_t m_server;
ball_position_t ball;
paddle_position_t paddle_pos;
ball_position_t temp_ball;

//List of thread identifiers for each client
pthread_t client_t_id[10];

//Thread to move the ball every second
void * handle_ball(void * arg){
    while(1){
        pthread_mutex_lock(&m1);
        move_ball(&ball, paddle_pos, m_client.index, m_server.clients, m_client.paddle_dir, n_clients);
        m_server.ball_pos = ball;
        for(int j = 0; j < 10; j++){
            if(m_server.clients[j].score!=-1){
                m_server.index = m_server.clients[j].index;
                write(client_fd_list[j], &m_server, sizeof(m_server));
            }
        }
        pthread_mutex_unlock(&m1);
        sleep(1);
    }
    return NULL;
}

//Thread that handles messages received by a client
void * handle_client(void * arg){
    int * my_client_fd = (int *) arg;
    while(1){
        nbytes = read(*my_client_fd, &m_client, sizeof(m_client));
        printf("Received %d from %d\n", nbytes, m_client.index);
        if (nbytes > 0){
            pthread_mutex_lock(&m1);
            if(m_client.type == 1){
            //Paddle_move received
                for(int i = 0; i < 10; i++){
                    if (m_server.clients[i].score!=-1){
                        if(m_client.index == m_server.clients[i].index){
                            //Check if it hits any other paddle while moving, if it does, don't move                           
                            paddle_pos = m_server.clients[i].paddle_pos;
                            move_paddle(&paddle_pos, m_server.clients, n_clients, m_client.paddle_dir, &ball, i);
                            m_server.ball_pos = ball;
                            m_server.clients[i].paddle_pos = paddle_pos;
                            break;
                        }
                    }
                }
                //Sends clients info about the paddle move
                for(int j = 0; j < 10; j++){
                    if(m_server.clients[j].score!=-1){
                        m_server.index = m_server.clients[j].index;
                        write(client_fd_list[j], &m_server, sizeof(m_server));
                    }
                }
            }
            else if (m_client.type == 3){
            //Received disconnect message
                int dc = -1;
                for (int i = 0; i < 10; i++){
                    if(m_client.index == m_server.clients[i].index){
                        dc = i;
                        break;
                    }
                }
                //Delete from array
                if (dc > -1){
                    //Deletes all client information
                    client_t_id[dc]=-1;
                    m_server.clients[dc].score=-1;
                    m_server.clients[dc].paddle_pos.length=-1;
                    m_server.clients[dc].paddle_pos.x=-1;
                    m_server.clients[dc].paddle_pos.y=-1;
                    m_server.clients[dc].index=-1;

                    n_clients--;
                    m_server.n_clients = n_clients;
                }
                pthread_mutex_unlock(&m1);
                pthread_exit(0);
            }
            pthread_mutex_unlock(&m1);
        }
    }
    return NULL;
}

int main(){
    //Create Socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }

    for (int i=0;i<10;i++){
        client_t_id[i] = -1;
        m_server.clients[i].score=-1;
        m_server.clients[i].index=-1;
    }
    
    //Create local_addr to store the server's address properties
    memset(&local_addr, '\0', sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr_size = sizeof(struct sockaddr);

    //Bind the socket
    int err = bind(sock_fd, (struct sockaddr *)&local_addr, local_addr_size);
    if(err == -1){
        perror("bind");
        exit(-1);
    }

    listen(sock_fd, 10);
    printf("[+]Listening...\n");

    client_addr_size = sizeof(struct sockaddr);
    
    m_server.clients[10];

    n_clients = 0;
    total_clients = 0;
    int n;
    pthread_mutex_init(&m1, NULL);

    //Thread to handle periodic ball movements
    pthread_t ball_thread_id;
    pthread_create(&ball_thread_id, NULL, handle_ball, NULL);
    while(1){
        client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_size);
        //Checks if there are clients connected
        if (client_fd > 0){
            for (int i = 0; i<10;i++){
                //Checks if the client in that index is playing
                if(m_server.clients[i].score==-1){
                    pthread_mutex_lock(&m1);
                    //Places random ball if it's the first client connecting
                    if(n_clients == 0){
                        place_ball_random(&ball);
                        m_server.ball_pos = ball;
                    }
                    //Updates client info
                    client_fd_list[i] = client_fd;
                    m_server.index = total_clients;
                    m_server.clients[i].index = total_clients;
                    m_server.clients[i].score = 0;
                    //Plaves a random paddle for the client
                    random_paddle(&paddle_pos, m_server.clients, n_clients, PADDLE_SIZE, ball);
                    m_server.clients[i].paddle_pos = paddle_pos;
                    m_server.clients[i].client_addr = client_addr;
                    n_clients++;
                    m_server.n_clients = n_clients;
                    total_clients++;
                    printf("\n-----------------------------\n");
                    printf("[+]Client %d has connected.\n[+]Clients currently connected: %d\n[+]Total number of clients: %d\n", client_fd, n_clients, total_clients);
                    printf("-----------------------------\n\n");
                    write(client_fd, &m_server, sizeof(m_server));
                    pthread_mutex_unlock(&m1);
                    //Creates thread for the client
                    pthread_create(&client_t_id[i], NULL, handle_client, (void *)&client_fd_list[i]);                   
                    break;
                }
            }
        }
        else{
            perror("accept");
        }
    }
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