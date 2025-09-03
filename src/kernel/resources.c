//
// Created by Artur Twardzik on 31/08/2025.
//

#include "resources.h"

#include "proc.h"

//TODO: resource queue to be implemented...
static struct {
        bool none;
        pid_t process;
} process_blocked_on_io_keyboard = {true};

void set_process_wait_for_resource(const pid_t parent_process, const Resource resource) {
        change_process_state(parent_process, WAITING_FOR_RESOURCE);

        if (resource == IO_KEYBOARD) {
                process_blocked_on_io_keyboard.process = parent_process;
                process_blocked_on_io_keyboard.none = false;
        }
}

void notify_resource(const Resource resource, const void *data) {
        if (resource == IO_KEYBOARD) {
                //ctx switch to process blocked on io keyboard, restored from psp, as we know it was in handler for sure
        }
}
