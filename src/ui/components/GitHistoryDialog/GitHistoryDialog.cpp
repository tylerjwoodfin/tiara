#include "GitHistoryDialog.h"
#include <cstring>

GitHistoryDialog::GitHistoryDialog(int height, int width, int starty, int startx, const std::string& url)
    : url(url) {
    window = newwin(height, width, starty, startx);
    keypad(window, TRUE);
    box(window, 0, 0);
    wrefresh(window);
}

GitHistoryDialog::~GitHistoryDialog() {
    if (window) {
        delwin(window);
    }
}

void GitHistoryDialog::show() {
    // Clear the window
    werase(window);
    box(window, 0, 0);

    // Display title
    mvwprintw(window, 1, 2, "Git History URL:");
    
    // Display URL with word wrap
    int max_width = getmaxx(window) - 4;
    std::string display_url = url;
    int y_pos = 3;
    
    while (!display_url.empty()) {
        std::string line = display_url.substr(0, max_width);
        display_url = display_url.substr(max_width);
        mvwprintw(window, y_pos++, 2, "%s", line.c_str());
    }

    // Display instructions
    mvwprintw(window, getmaxy(window) - 2, 2, "Press any key to close");
    
    wrefresh(window);
    wgetch(window);
} 