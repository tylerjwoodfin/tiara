#pragma once

#include <string>

using namespace std;

struct Config {
  bool tui_enabled = true;
  string default_board = "";
  bool move_card_to_column_bottom = false;
  string data_file_location = "";
  string config_file_location = "";
  bool prefix = false;
  void read_config();
  void write_config();
  
  static string get_data_file_location() {
    static Config config;
    static bool initialized = false;
    if (!initialized) {
      config.read_config();
      initialized = true;
    }
    return config.data_file_location.empty() ? 
           (string(getenv("HOME")) + "/.local/share/tiara/data") : 
           config.data_file_location;
  }
};
