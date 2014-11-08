
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"

#include "ws2811.h"


static volatile int quit;


/* leds */

#define TARGET_FREQ                              WS2811_TARGET_FREQ
#define GPIO_PIN                                 18
#define DMA                                      5

#define WIDTH                                    8
#define HEIGHT                                   1
#define LED_COUNT                                (WIDTH * HEIGHT)

#define RGB(R,G,B)                               (((R)<<16) | ((G)<<8) | (B))

static ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
        },
    },
};

static volatile int rotate;
static volatile ws2811_led_t matrix[WIDTH][HEIGHT];

static void handleleds(void *data);
static void matrix_render(void);

/* server */

static void server(void *data);
static int ReadLine(int fd, char buffer[], int bufsize);
static int match(const char *str, const char *pattern);

int main(int argc, char *argv[]) {
    pthread_t  thread1, thread2;

    pthread_create(&thread1, NULL, server, NULL);
    pthread_create(&thread2, NULL, handleleds, NULL);

    // Wait for server() to quit
    pthread_join( thread1, NULL);

    // Wait for handleleds() to quit
    pthread_join( thread2, NULL); 

    return 0;
}


static void handleleds(void *data) {
    int i;
    ws2811_led_t colors[8] = {
        0x200000,  // red
        0x201000,  // orange
        0x202000,  // yellow
        0x002000,  // green
        0x002020,  // lightblue
        0x000020,  // blue
        0x100010,  // purple
        0x200010,  // pink
    };
    ws2811_init(&ledstring);
    for (i = 0; i < 8; i++)
        matrix[i][0] = colors[i];
    while (quit == 0) {
        matrix_render();
        ws2811_render(&ledstring);
        usleep(10000);
    }
    printf("\nws2811_fini\n");
    ws2811_fini(&ledstring);
}

static void matrix_render(void) {
    int x, y;
    for (x = 0; x < WIDTH; x++) {
        for (y = 0; y < HEIGHT; y++) {
            ledstring.channel[0].leds[(y * WIDTH) + x] = matrix[x][y];
        }
    }
}

static void server(void *data) {
    int sfd, cfd;
    struct sockaddr_in my_addr = {0};
    char buffer[1024] = {0};

    printf("server socket..\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        printf("error");
        quit = 1;
        return;
    }

    printf("bind..\n");
    my_addr.sin_family      = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port        = htons(1234);
    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1) {
        printf("error");
        quit = 1;
        close(sfd);
        return;
    }

    printf("listen..\n");
    if (listen(sfd, 50) == -1) {
        printf("error");
        quit = 1;
        close(sfd);
        return;
    }

    while (quit == 0) {
        printf("accept..\n");
        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1) {
            printf("error");
            quit = 1;
            break;
        }

        printf("reading commands from client..");
        while (ReadLine(cfd, buffer, sizeof(buffer))) {
            if (buffer[0] == '\0')
                continue;

            printf("cmd:%s\n",buffer);

            if (match(buffer,"pD=X")) {
                int pin, color;
                pin   = strtol(buffer+1, NULL, 10);
                color = strtol(1+strchr(buffer,'='), NULL, 16);
                printf("setpixel pin=%i color=%i\n", pin, color);
                matrix[pin][0] = color;
            }

            else if (strcmp(buffer, "rotate=1") == 0) {
                printf("rotate=1\n");
                rotate = 1;
            }
            
            else if (strcmp(buffer, "rotate=0") == 0) {
                printf("rotate=0\n");
                rotate = 1;
            }
            
            else if (strcmp(buffer, "quit") == 0) {
                quit = 1;
                break;
            }
        }

        close(cfd);
    }

    close(sfd);
}


static int ReadLine(int fd, char buffer[], int bufsize)
{
    char c = 0;
    int  pos = 0;
    while (pos < bufsize - 1) {
        if (read(fd, &c, 1) != 1) {
            // some read failure
            return 0;
        } else {
            if (c=='\r' || c=='\n') {
                buffer[pos] = 0;
                return 1;
            } else {
                buffer[pos++] = c;
            }
        }
    }
    buffer[bufsize-1] = 0;
    return 1;
}

static int match(const char *str, const char *pattern)
{
    while (*str && *pattern) {
        if (*pattern == 'D') { // decimal number
            if (!isdigit(*str))
                return 0;
            while (isdigit(*str))
                str++;
        } else if (*pattern == 'X') { // hexadecimal number
            if (!isxdigit(*str))
                return 0;
            while (isxdigit(*str))
                str++;
        } else if (*str == *pattern) {
            str++;
        } else {
            return 0; // str doesn't match pattern
        }
        pattern++;
    }

    return (*str=='\0' && *pattern=='\0');
}

