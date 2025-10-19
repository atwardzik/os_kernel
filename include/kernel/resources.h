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
void block_resource_on_condition(pid_t parent_process, Resource resource, bool (*condition)(void));

pid_t get_resource_acquiring_process(void);

#endif //OS_RESOURCES_H
