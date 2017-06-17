/*
 * References: 	https://www.kernel.org/doc/Documentation/input/event-codes.txt
 * 				https://www.kernel.org/doc/Documentation/input/input.txt
 */

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <linux/input.h>

#include <string.h>
#include <cstdint>
#include <csignal>

#define APEM_BTN_LEFT      288
#define APEM_BTN_RIGHT     289
#define APEM_BTN_INIT_SYNC 4

void interrupt_handler(int dummy);

void handle_event(struct input_event ev);

static volatile int keep_running = 1;

void interrupt_handler(int dummy) {
    keep_running = 0;
}

int main() {

    signal(SIGINT, interrupt_handler);

    char device[30];

    strcpy(device, "/dev/input/event14");
    int fid = open(device, O_RDONLY);
    if (fid == 0) {
        fprintf(stderr, "Could not open %s!\n", device);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Opened %s!\n", device);

    ssize_t nbytes;

    struct input_event event;

    fprintf(stdout, "Size of event is: %ld\n", sizeof(event));

    while (keep_running) {

        nbytes = read(fid, &event, sizeof(event));
        if (nbytes == sizeof(event)) {
            handle_event(event);
        } else if (nbytes > 0) {
            printf("received fragment of length %ld", nbytes);
        }

    }

    printf("Closing %s", device);
    close(fid);

    return EXIT_SUCCESS;
}

void handle_event(struct input_event event) {
    static int counter = 0;
    char event_code[20], event_type[20];

    switch (event.code) {
        case APEM_BTN_INIT_SYNC:
            switch (event.type) {
                case EV_MSC:
                    strcpy(event_type, "sync");
                    strcpy(event_code, "init");
                    break;
                default:
                    sprintf(event_type, "%d", event.type);
                    strcpy(event_code, "unknown");
                    break;
            }
            break;
        case APEM_BTN_LEFT:
            switch (event.type) {
                case EV_KEY:
                    strcpy(event_type, "button");
                    strcpy(event_code, "left");
                    break;
                default:
                    sprintf(event_type, "%d", event.type);
                    strcpy(event_code, "unknown");
                    break;
            }
            break;
        case APEM_BTN_RIGHT:
            switch (event.type) {
                case EV_KEY:
                    strcpy(event_type, "button");
                    strcpy(event_code, "right");
                    break;
                default:
                    sprintf(event_type, "%d", event.type);
                    strcpy(event_code, "unknown");
                    break;
            }
            break;
        case KEY_RESERVED:
            switch (event.type) {
                case EV_SYN:
                    strcpy(event_type, "sync");
                    strcpy(event_code, "end");
                    break;
                case EV_ABS:
                    strcpy(event_type, "stick");
                    strcpy(event_code, "roll");
                    break;
                case EV_MSC:
                    strcpy(event_type, "sync");
                    strcpy(event_code, "begin");
                    break;
                default:
                    strcpy(event_code, "unknown");
                    break;
            }
            break;
        case KEY_ESC:
            strcpy(event_type, "stick");
            strcpy(event_code, "pitch");
            break;
        case KEY_1:
            strcpy(event_type, "stick");
            strcpy(event_code, "yaw");
            break;
        default:
            sprintf(event_code, "%d", event.code);
    }

    fprintf(stdout,
            "[%d] sec: %-10ld  | us: %-10ld  |  "
                    "type: %-10s  |  code: %-10s  |  value: %-10d\n",
            counter++, event.time.tv_sec, event.time.tv_usec,
            event_type, event_code, event.value);

}
