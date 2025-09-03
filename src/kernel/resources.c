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

static void set_process_wait_for_resource(const pid_t parent_process) {
        change_process_state(parent_process, WAITING_FOR_RESOURCE);
}

void __attribute__((naked )) *block_on_resource(const pid_t parent_process, const Resource resource) {
        __asm__("push   {r4, lr}\n\r"
                "mov    r4, r1\n\r"
                "bl     set_process_wait_for_resource\n\r"
                ""                   //TODO: force context switch, but save kernel frame on psp
                "pop    {r4, pc}\n\r"//as we return here the pointer to the resource is already in r0
        );
}

void notify_resource(const Resource resource, const void *data) {
        if (resource == IO_KEYBOARD) {
                //ctx switch to process blocked on io keyboard, restored from psp, as we know it was in handler for sure
        }
}
