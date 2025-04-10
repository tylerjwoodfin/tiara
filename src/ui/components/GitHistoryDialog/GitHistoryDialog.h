#ifndef GIT_HISTORY_DIALOG_H
#define GIT_HISTORY_DIALOG_H

#include <ncurses.h>
#include <string>

using namespace std;

class GitHistoryDialog {
public:
  GitHistoryDialog(int height, int width, int starty, int startx, const std::string& url);
  ~GitHistoryDialog();
  void show();

  int height;
  int width;
  int start_y;
  int start_x;
  WINDOW *window;

  string url;
};

#endif // GIT_HISTORY_DIALOG_H 