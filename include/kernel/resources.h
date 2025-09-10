//
// Created by Artur Twardzik on 31/08/2025.
//

#ifndef OS_RESOURCES_H
#define OS_RESOURCES_H

#include "resources_codes.h"

#include <sys/types.h>

typedef uint8_t Resource;

/**
 * Blocks current process on a specified resource. \n
 * Please note that this function <b>HAS</b> to be run in <b>handler mode</b>.
 * @return Pointer to the resource
 */
void *block_on_resource(pid_t parent_process, Resource resource);

void signal_resource(void *resource, Resource resource_type);

#endif //OS_RESOURCES_H
