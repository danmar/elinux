rpi:	rpi.c
	gcc -o rpi -Irpi_ws281x rpi.c -lws2811 -lpthread
