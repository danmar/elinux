
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

#define LED_COUNT                                8

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
static volatile int rotatetime = 100;
static volatile ws2811_led_t leds[LED_COUNT];

static void handleleds(void *data);

/* server */

static void ledserver(void);
static int  readline(int fd, char buffer[], int bufsize);
static int  match(const char *str, const char *pattern);

int main(int argc, char *argv[]) {
    pthread_t  ledsthread;

    pthread_create(&ledsthread, NULL, handleleds, NULL);
    ledserver();

    // Wait for handleleds() to quit
    pthread_join(ledsthread, NULL);

    return 0;
}


static void handleleds(void *data) {
    int i,time;
    const ws2811_led_t colors[8] = {
        0xff0000,  // red
        0xff8000,  // orange
        0xffff00,  // yellow
        0x00ff00,  // green
        0x00ffff,  // lightblue
        0x0000ff,  // blue
        0x800080,  // purple
        0xff0080,  // pink
    };
    ws2811_led_t r3colors[LED_COUNT];
    int r3count = 0;
    int r3direction = 0;

    ws2811_init(&ledstring);
    for (i = 0; i < LED_COUNT; i++)
        leds[i] = colors[i];
    time = 0;
    while (quit == 0) {
        if (rotate != 3)
            r3direction = 0;

        ++time;
        if (time >= rotatetime) {
            time = 0;
            if (rotate == 1) {
                const ws2811_led_t color = leds[0];
                for (i = 1; i < LED_COUNT; i++)
                    leds[i-1] = leds[i];
                leds[LED_COUNT-1] = color;
            } else if (rotate == 2) {
                const ws2811_led_t color = leds[LED_COUNT-1];
                for (i = LED_COUNT-2; i >= 0; i--)
                    leds[i+1] = leds[i];
                leds[0] = color;
            } else if (rotate == 3) {
                // initialize 
                if (!r3direction) {
                    for (i = 0; i < LED_COUNT; i++)
                        r3colors[i] = leds[i];
                    r3count = 0;
                    r3direction = 1;
                }

                r3count++;
                if (r3count >= LED_COUNT - 1) {
                    r3direction = (r3direction==1) ? 2 : 1;
                    r3count = 0;
                }
                if (r3direction == 1) {
                    for (i = 0; i < LED_COUNT; i++)
                        leds[i] = (i + r3count >= LED_COUNT) ? 0 : r3colors[i + r3count];
                } else {
                    for (i = 0; i < LED_COUNT; i++)
                        leds[LED_COUNT - 1 - i] = (i + r3count >= LED_COUNT) ? 0 : r3colors[i + r3count];
                }
            }
        }

        for (i = 0; i < LED_COUNT; i++) {
            int r = (leds[i] & 0xff0000) >> 16;
            int g = (leds[i] & 0x00ff00) >> 8;
            int b = leds[i] & 0x0000ff;
            ledstring.channel[0].leds[i] = RGB(r>>4, g>>4, b>>4);
        }
        ws2811_render(&ledstring);
        usleep(1000);
    }
    printf("\nws2811_fini\n");
    ws2811_fini(&ledstring);
}


static void ledserver(void) {
    int sfd, cfd;
    struct sockaddr_in my_addr = {0};
    char buffer[1024] = {0};

    puts("ledserver\n");

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        quit = 1;
        return;
    }

    my_addr.sin_family      = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port        = htons(1234);
    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1) {
        quit = 1;
        close(sfd);
        return;
    }

    if (listen(sfd, 50) == -1) {
        quit = 1;
        close(sfd);
        return;
    }

    while (quit == 0) {
        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1) {
            quit = 1;
            break;
        }

        printf("reading commands from client..");
        while (readline(cfd, buffer, sizeof(buffer))) {
            if (buffer[0] == '\0')
                continue;

            printf("cmd:%s\n",buffer);

            if (match(buffer,"pD=X")) {
                int pin, color;
                pin   = strtol(buffer+1, NULL, 10);
                color = strtol(1+strchr(buffer,'='), NULL, 16);
                leds[pin] = color;
            }

            else if (match(buffer, "rotate=D")) {
                rotate = atoi(buffer+7);
            }

            else if (match(buffer, "rotatetime=D")) {
                rotatetime = atoi(buffer+11);
                if (rotatetime > 2000)
                    rotatetime = 2000;
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


static int readline(int fd, char buffer[], int bufsize)
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
