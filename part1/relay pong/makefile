all: relay-pong-client relay-pong-server

relay-pong-client: relay-pong-client.c
	gcc -o relay-pong-client lib.c relay-pong-client.c -lncurses

relay-pong-server: relay-pong-server.c
	gcc -o relay-pong-server lib.c relay-pong-server.c -lncurses