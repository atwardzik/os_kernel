//
// Created by Artur Twardzik on 09/04/2026.
//

#ifndef OS_SD_CARD_H
#define OS_SD_CARD_H

#include "fs/hard_drive.h"

#include <stdint.h>

struct HardDriveOperations *init_sd_card(void);

#endif //OS_SD_CARD_H
