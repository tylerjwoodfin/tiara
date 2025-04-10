#pragma once

#include <string>
#include "../Config/Config.h"

#define NAME "tiara"
#define VERSION "1.0.0"
#define CONFIG_FILE (string(getenv("HOME")) + "/.config/tiara/config")
#define DATA_FILE (Config::get_data_file_location())
#define DATA_BACKUP_FILE                                                       \
  (string(getenv("HOME")) + "/.local/share/tiara/data_bkp")
#define COLOR_PAIR_FOOTER 1
#define COLOR_PAIR_MODE 2
#define COLOR_PAIR_HEADER 3
#define COLOR_PAIR_BORDER 4
#define COLOR_PAIR_KEY_HINT 5

#define MODE_NORMAL 0
#define MODE_INSERT 1
