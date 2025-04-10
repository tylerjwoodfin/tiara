#include "Checklist.h"
#include "../../../helpers/consts.h"
#include "../../helpers/win_center_text/win_center_text.h"
#include "../../helpers/win_fit_text/win_fit_text.h"
#include "../Input/Input.h"
#include "../../screens/Board/Board.h"

Checklist::Checklist(int height, int width, int start_y, int start_x,
                     Card *card, DataManager *data_manager)
    : height{height}, width{width}, start_y{start_y}, start_x{start_x},
      checklist_items_window{this->height,
                             this->width,
                             this->start_y,
                             this->start_x,
                             &this->checklist_items,
                             &this->checklist_items_count,
                             bind(&Checklist::draw_checklist_items, this,
                                  placeholders::_1, placeholders::_2),
                             this->height - 3} {
  this->window =
      newwin(this->height, this->width, this->start_y, this->start_x);

  this->card = card;

  this->checklist_items = this->card->checklist;
  this->checklist_items_count = this->checklist_items.size();

  this->highlighted_index = 0;
  this->data_manager = data_manager;
}

void Checklist::setup_window() {
  wattron(this->window, COLOR_PAIR(COLOR_PAIR_BORDER));
  box(this->window, 0, 0);
  wattroff(this->window, COLOR_PAIR(COLOR_PAIR_BORDER));

  string window_title = " Checklist";

  if (this->checklist_items_count > 0) {
    size_t done_count = 0;
    size_t total = this->checklist_items_count;

    for (size_t i = 0; i < total; ++i)
      done_count += this->card->checklist[i].done;

    // add [x/y] to title
    window_title += " [" + to_string(done_count) + "/" + to_string(total) + "]";

    // progress bar |[====  ] zzz%|
    int done_percent = (done_count / (float)total) * 100;

    string progress_bar = "";
    for (int i = 0; i < int((done_percent / 100.0) * (this->width - 9)); ++i)
      progress_bar += '=';

    mvwprintw(this->window, 1, 1, "[%-*s] %3d%%", this->width - 9,
              progress_bar.c_str(), done_percent);
  }

  window_title += " ";

  mvwprintw(this->window, 0, (this->width / 2) - (window_title.length() / 2),
            "%s", window_title.c_str());

  refresh();
  wrefresh(this->window);
}

void Checklist::show() {
  this->checklist_items_window.scroll_to_top();

  bool done = false;
  char key;
  while (!done && (key = wgetch(this->window))) {
    switch (key) {
    case 'q': {
      // quit
      done = true;

      break;
    }
    case 'k': {
      if (this->checklist_items_count > 0) {
        this->highlight_above();
      }

      break;
    }
    case 'j': {
      if (this->checklist_items_count > 0) {
        this->highlight_below();
      }

      break;
    }
    case 'g': {
      // highlight first item
      if (this->checklist_items_count > 0) {
        this->highlighted_index = 0;
        this->checklist_items_window.scroll_to_top();
      }

      break;
    }
    case 'G': {
      // highlight last board
      if (this->checklist_items_count > 0) {
        this->highlight_last();
      }

      break;
    }
    case 'K': {
      // move highlighted item up
      if (this->checklist_items_count > 0) {
        size_t highlighted_item_index =
            (this->checklist_items_window.window_start -
             this->checklist_items.begin()) +
            this->highlighted_index;
        bool moved = this->data_manager->move_checklist_item_up(
            this->card, highlighted_item_index);

        if (moved) {
          this->checklist_items = this->card->checklist;
          this->checklist_items_window.draw();

          this->highlight_above();
        }
      }

      break;
    }
    case 'J': {
      // move highlighted item down
      if (this->checklist_items_count > 0) {
        size_t highlighted_item_index =
            (this->checklist_items_window.window_start -
             this->checklist_items.begin()) +
            this->highlighted_index;
        bool moved = this->data_manager->move_checklist_item_down(
            this->card, highlighted_item_index);

        if (moved) {
          this->checklist_items = this->card->checklist;
          this->checklist_items_window.draw();

          this->highlight_below();
        }
      }

      break;
    }
    case ' ': {
      // toggle highlighted item (done/not done)
      if (this->checklist_items_count > 0) {
        size_t highlighted_item_index =
            (this->checklist_items_window.window_start -
             this->checklist_items.begin()) +
            this->highlighted_index;
        ChecklistItem highlighted_item =
            this->checklist_items[highlighted_item_index];

        highlighted_item.done = !highlighted_item.done;
        this->data_manager->update_checklist_item(
            this->card, highlighted_item_index, highlighted_item);

        this->checklist_items = this->card->checklist;
        this->checklist_items_window.draw();
      }

      break;
    }
    case 'c': {
      // add an item
      string item_content = this->create_input_window();

      if (item_content.length() > 0) {
        ChecklistItem item = {item_content, false};
        this->data_manager->add_checklist_item(this->card, item);

        this->checklist_items = this->card->checklist;
        this->checklist_items_count = this->checklist_items.size();

        // highlight the just added item
        this->highlighted_index =
            min((size_t)this->checklist_items_window.max_items_in_win - 1,
                this->checklist_items_count - 1);
        this->checklist_items_window.scroll_to_bottom();
      }

      break;
    }
    case 'p': {
      // search git history
      this->searchGitHistory();
      break;
    }
    case 'e': {
      // edit highlighted item's content
      if (this->checklist_items_count > 0) {
        size_t highlighted_item_index =
            (this->checklist_items_window.window_start -
             this->checklist_items.begin()) +
            this->highlighted_index;
        ChecklistItem highlighted_item =
            this->checklist_items[highlighted_item_index];

        string new_content =
            this->create_input_window(highlighted_item.content);

        if (new_content.length() > 0) {
          size_t offset = this->checklist_items_window.window_start -
                          this->checklist_items.begin();

          highlighted_item.content = new_content;
          this->data_manager->update_checklist_item(
              this->card, highlighted_item_index, highlighted_item);

          this->checklist_items = this->card->checklist;
          this->checklist_items_window.scroll_to_offset(offset);
        }
      }

      break;
    }
    case 'd': {
      // delete highlighted item
      if (this->checklist_items_count > 0) {
        size_t highlighted_item_index =
            (this->checklist_items_window.window_start -
             this->checklist_items.begin()) +
            this->highlighted_index;
        size_t prev_offset = this->checklist_items_window.window_start -
                             this->checklist_items.begin();

        this->data_manager->delete_checklist_item(this->card,
                                                  highlighted_item_index);

        this->checklist_items = this->card->checklist;
        this->checklist_items_count = this->checklist_items.size();

        this->highlighted_index = min((size_t)this->highlighted_index,
                                      this->checklist_items_count - 1);
        this->checklist_items_window.scroll_to_offset(prev_offset);
      }

      break;
    }
    }
  }

  werase(this->window);
  wrefresh(this->window);
}

void Checklist::draw_checklist_items(
    vector<ChecklistItem> shown_items,
    [[maybe_unused]] WINDOW *scrollable_window) {
  werase(this->window);
  this->setup_window();

  if (shown_items.size() > 0) {
    // draw items
    for (size_t i = 0; i < shown_items.size(); ++i) {
      // the [x] or [ ] before item content
      string item = "[";
      item += shown_items[i].done ? 'x' : ' ';
      item += "] ";

      // actual item content
      item += shown_items[i].content;

      mvwprintw(this->window, i + 2, 1, "%s",
                win_fit_text(this->window, item).c_str());
    }

    this->highlight_current();
  } else {
    string create_board_hint = "c to add a checklist item";
    int center_x = win_center_x(this->window, &create_board_hint);
    mvwprintw(this->window, 1, center_x, "%s", create_board_hint.c_str());
    
    string git_history_hint = "p to search git history";
    center_x = win_center_x(this->window, &git_history_hint);
    mvwprintw(this->window, 2, center_x, "%s", git_history_hint.c_str());
  }

  wrefresh(this->window);
}

void Checklist::highlight_current() {
  if (this->checklist_items_count > 0) {
    // clear previous highlights
    for (int i = 0; i < this->checklist_items_window.max_items_in_win; ++i)
      mvwchgat(this->window, i + 2, 1, this->width - 2, A_NORMAL, 0, NULL);

    // highlight current
    mvwchgat(this->window, this->highlighted_index + 2, 1, this->width - 2,
             A_NORMAL, COLOR_PAIR_FOOTER, NULL);

    wrefresh(this->window);
  }
}

void Checklist::highlight_above() {
  if (--this->highlighted_index == -1) {
    this->highlighted_index = 0;
    this->checklist_items_window.scroll_up();
  } else
    this->highlight_current();
}

void Checklist::highlight_below() {
  this->highlighted_index =
      min(this->checklist_items_count - 1, (size_t)this->highlighted_index + 1);

  if (this->highlighted_index ==
      this->checklist_items_window.max_items_in_win) {
    this->highlighted_index = this->checklist_items_window.max_items_in_win - 1;
    this->checklist_items_window.scroll_down();
  } else
    this->highlight_current();
}

void Checklist::highlight_last() {
  this->highlighted_index =
      min((size_t)this->checklist_items_window.max_items_in_win - 1,
          this->checklist_items_count - 1);
  this->checklist_items_window.scroll_to_bottom();
}

string Checklist::create_input_window(string content) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int height = 3;
  int width = max_x * 0.4;
  int start_y = (max_y / 2) - (height / 2);
  int start_x = (max_x / 2) - (width / 2);

  Input input_bar(height, width, start_y, start_x, content, " Checklist Item ",
                  true);
  input_bar.show(true);
  string input = input_bar.get_value();

  this->checklist_items_window.draw();

  return input;
}

void Checklist::searchGitHistory() {
  // Extract the prefix from the card content if it exists
  string cardPrefix = "";
  string content = this->card->content;
  size_t pos = content.find("::");
  if (pos != string::npos && pos > 0) {
    cardPrefix = content.substr(0, pos);
  }
  
  // If no prefix found, show error
  if (cardPrefix.empty()) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int height = 5;
    int width = 50;
    int start_y = (max_y / 2) - (height / 2);
    int start_x = (max_x / 2) - (width / 2);
    
    WINDOW* error_window = newwin(height, width, start_y, start_x);
    box(error_window, 0, 0);
    mvwprintw(error_window, 1, 2, "No prefix found in card content.");
    mvwprintw(error_window, 2, 2, "Card must have format: prefix::content");
    mvwprintw(error_window, 3, 2, "Press any key to continue");
    wrefresh(error_window);
    wgetch(error_window);
    delwin(error_window);
    return;
  }
  
  // Get the board name from the data manager
  string boardName = "";
  
  // Instead of trying to find the board by comparing card pointers,
  // we'll use the card content to find the matching board
  for (const auto& board : this->data_manager->boards) {
    for (const auto& column : board.columns) {
      for (const auto& card : column.cards) {
        if (card.content == this->card->content) {
          boardName = board.name;
          break;
        }
      }
      if (!boardName.empty()) break;
    }
    if (!boardName.empty()) break;
  }
  
  if (boardName.empty()) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int height = 5;
    int width = 50;
    int start_y = (max_y / 2) - (height / 2);
    int start_x = (max_x / 2) - (width / 2);
    
    WINDOW* error_window = newwin(height, width, start_y, start_x);
    box(error_window, 0, 0);
    mvwprintw(error_window, 1, 2, "Could not determine board name.");
    mvwprintw(error_window, 2, 2, "Press any key to continue");
    wrefresh(error_window);
    wgetch(error_window);
    delwin(error_window);
    return;
  }
  
  // Get GitHub username
  string username = "";
  FILE* pipe = popen("git config --get github.user", "r");
  if (pipe) {
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
      username = string(buffer);
      // Trim whitespace
      username = username.substr(0, username.find_last_not_of(" \n\r\t") + 1);
    }
    pclose(pipe);
  }
  
  if (username.empty()) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int height = 5;
    int width = 60;
    int start_y = (max_y / 2) - (height / 2);
    int start_x = (max_x / 2) - (width / 2);
    
    WINDOW* error_window = newwin(height, width, start_y, start_x);
    box(error_window, 0, 0);
    mvwprintw(error_window, 1, 2, "GitHub username not found.");
    mvwprintw(error_window, 2, 2, "Please set it with 'git config --global github.user YOUR_USERNAME'");
    mvwprintw(error_window, 3, 2, "Press any key to continue");
    wrefresh(error_window);
    wgetch(error_window);
    delwin(error_window);
    return;
  }
  
  // Show "Searching..." dialog immediately
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  
  int height = 5;
  int width = 40;
  int start_y = (max_y / 2) - (height / 2);
  int start_x = (max_x / 2) - (width / 2);
  
  WINDOW* searching_window = newwin(height, width, start_y, start_x);
  box(searching_window, 0, 0);
  mvwprintw(searching_window, 1, 2, "Searching Git history...");
  mvwprintw(searching_window, 2, 2, "Please wait...");
  wrefresh(searching_window);
  
  // Construct the search pattern: boardName-cardPrefix
  string searchPattern = boardName + "-" + cardPrefix;
  
  // Search for commits across repositories
  string command = "gh repo list \"" + username + "\" --limit 1000 --json nameWithOwner | jq -r '.[].nameWithOwner' | while read repo; do "
                  "commits=$(gh api -X GET \"repos/$repo/commits\" -F per_page=100 -f sha=main "
                  "-H \"Accept: application/vnd.github+json\" 2>/dev/null | "
                  "jq -r '.[] | select(.commit.message | test(\"^" + searchPattern + "\"; \"i\")) | \"https://github.com/'$repo'/commit/\" + .sha' 2>/dev/null); "
                  "if [[ -n \"$commits\" ]]; then echo \"$commits\" | head -n 1; break; fi; done";
  
  pipe = popen(command.c_str(), "r");
  if (pipe) {
    char buffer[1024];
    string result = "";
    if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
      result = string(buffer);
      // Trim whitespace
      result = result.substr(0, result.find_last_not_of(" \n\r\t") + 1);
    }
    pclose(pipe);
    
    // Remove the searching window
    delwin(searching_window);
    
    // Show results
    height = 10;
    width = 60;
    start_y = (max_y / 2) - (height / 2);
    start_x = (max_x / 2) - (width / 2);
    
    WINDOW* result_window = newwin(height, width, start_y, start_x);
    box(result_window, 0, 0);
    
    if (!result.empty()) {
      mvwprintw(result_window, 1, 2, "Found commit URL:");
      mvwprintw(result_window, 2, 2, "%s", result.c_str());
      
      mvwprintw(result_window, 4, 2, "Press o to open");
      mvwprintw(result_window, 5, 2, "Press c to copy");
      mvwprintw(result_window, 6, 2, "Press q to quit");
      
      wrefresh(result_window);
      
      bool done = false;
      while (!done) {
        char key = wgetch(result_window);
        switch (key) {
          case 'o': {
            // Open URL in browser
            string open_command = "open '" + result + "'";
            system(open_command.c_str());
            break;
          }
          case 'c': {
            // Copy URL to clipboard
            string clipboard_command = "echo '" + result + "' | pbcopy";
            system(clipboard_command.c_str());
            
            // Show confirmation
            mvwprintw(result_window, 8, 2, "URL copied to clipboard!");
            wrefresh(result_window);
            napms(1000); // Show for 1 second
            mvwprintw(result_window, 8, 2, "                    "); // Clear the message
            wrefresh(result_window);
            break;
          }
          case 'q': {
            done = true;
            break;
          }
        }
      }
    } else {
      mvwprintw(result_window, 1, 2, "No matching commits found for pattern '%s'", searchPattern.c_str());
      mvwprintw(result_window, 3, 2, "Press any key to continue");
      wrefresh(result_window);
      wgetch(result_window);
    }
  } else {
    // Remove the searching window
    delwin(searching_window);
    
    // Show error
    height = 5;
    width = 50;
    start_y = (max_y / 2) - (height / 2);
    start_x = (max_x / 2) - (width / 2);
    
    WINDOW* error_window = newwin(height, width, start_y, start_x);
    box(error_window, 0, 0);
    mvwprintw(error_window, 1, 2, "Error executing Git search command.");
    mvwprintw(error_window, 2, 2, "Press any key to continue");
    wrefresh(error_window);
    wgetch(error_window);
    delwin(error_window);
  }
}
