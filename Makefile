rpi:	rpi.c
	gcc -o rpi -Irpi_ws281x rpi.c -L. -lws2811 -lpthread
