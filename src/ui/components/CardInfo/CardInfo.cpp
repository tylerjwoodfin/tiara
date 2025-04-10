#include "CardInfo.h"
#include "../../../helpers/consts.h"
#include "../Checklist/Checklist.h"
#include <iostream>
CardInfo::CardInfo(int height, int width, int start_y, int start_x,
                   Card *card) {
  this->height = height;
  this->width = width;
  this->start_y = start_y;
  this->start_x = start_x;

  this->window =
      newwin(this->height, this->width, this->start_y, this->start_x);

  this->card = card;
}

void CardInfo::setup_window(Input *content_input, Input *description_input,
                            bool just_redraw) {
  wattron(this->window, COLOR_PAIR(COLOR_PAIR_BORDER));
  box(this->window, 0, 0);
  wattroff(this->window, COLOR_PAIR(COLOR_PAIR_BORDER));

  // Show prefix in the title if it exists
  string window_title = " Card Info - ";
  string card_title = this->card->content;
  size_t pos = card_title.find("::");
  if (pos != string::npos) {
    window_title += card_title.substr(0, pos);
  }

  mvwprintw(this->window, 0, (this->width / 2) - (window_title.length() / 2),
            "%s", window_title.c_str());
  refresh();
  wrefresh(this->window);

  content_input->show(false, just_redraw);
  description_input->show(false, just_redraw);

  this->focused_input->focus();
}

bool CardInfo::show(DataManager *data_manager) {
  // Create content input with height 3 (single line)
  string content_to_show = this->card->content;
  size_t pos = content_to_show.find("::");
  if (pos != string::npos) {
    content_to_show = content_to_show.substr(pos + 2);
  }
  
  Input content_input =
      Input(3, this->width - 2, this->start_y + 1, this->start_x + 1,
            content_to_show, " Content ");
            
  // Create description input with remaining height (multi-line)
  Input description_input =
      Input(this->height - 5, this->width - 2, this->start_y + 4,
            this->start_x + 1, this->card->description, " Description ", false, true);

  this->focused_input = &content_input;
  this->focused_content_input = true;

  this->setup_window(&content_input, &description_input, false);

  char key;
  bool done = false;
  bool canceled = false;
  while (!done && (key = wgetch(this->window))) {
    switch (key) {
    case 27: // ESC
    case 'q': {
      // cancel and close if in normal mode
      if (this->focused_input->mode == MODE_NORMAL) {
        done = true;
        canceled = true;
      } else
        this->focused_input->handle_key_press(key);

      break;
    }
    case '\n': {
      // Enter
      // submit and close if in normal mode
      if (!this->focused_content_input &&
          this->focused_input->mode == MODE_INSERT)
        this->focused_input->handle_key_press(key);
      else
        done = true;

      break;
    }
    case '\t': {
      // Tab
      // switch focused input
      this->focused_content_input = !this->focused_content_input;

      if (this->focused_content_input) {
        description_input.unfocus();
        content_input.focus();
        this->focused_input = &content_input;
      } else {
        content_input.unfocus();
        description_input.focus();
        this->focused_input = &description_input;
      }

      break;
    }
    case 'c': {
      // open checklist window if in normal mode
      if (this->focused_input->mode == MODE_NORMAL &&
          this->card->content != "") {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int height = (max_y * 0.4) + 2;
        int width = max_x * 0.3;
        int start_y = (max_y / 2) - (height / 2);
        int start_x = (max_x / 2) - (width / 2);

        curs_set(0);
        Checklist checklist_window(height, width, start_y, start_x, this->card,
                                   data_manager);
        checklist_window.show();
        curs_set(1);

        this->setup_window(&content_input, &description_input, true);
      } else
        this->focused_input->handle_key_press(key);

      break;
    }
    default: {
      // any other key gets sent to the highlighted input
      this->focused_input->handle_key_press(key);

      break;
    }
    }
  }

  // Preserve the prefix when updating the content
  string new_content = content_input.get_value();
  if (this->card->content.find("::") != string::npos) {
    string prefix = this->card->content.substr(0, this->card->content.find("::") + 2);
    this->card->content = prefix + new_content;
  } else {
    this->card->content = new_content;
  }
  this->card->description = description_input.get_value();

  content_input.clean_up();
  description_input.clean_up();
  werase(this->window);
  wrefresh(this->window);

  return canceled;
}
