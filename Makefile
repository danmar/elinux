server:	server.c
	gcc -o server -Irpi_ws281x server.c -lws2811 -lpthread
