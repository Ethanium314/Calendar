#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>

using namespace std;

int text[12][31][200][200];

int months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

string makePrintable(int num, int digits)
{
	string printable = to_string(num);
	string zeros = "";

	for (int i = 0; i < digits - printable.length(); i++)
	{
		zeros += "0";
	}
	return zeros + printable;
}

string secondsToTime(int minutes)
{
	string h;
	string m;
	int hours = int(minutes / 60);
	minutes -= hours * 60;
	hours %= 24;
	h = makePrintable(hours, 2);
	m = makePrintable(minutes, 2);
	return(h + ":" + m);
}

int weekDay(int y, int m, int d)
{
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    return ( y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

int clamp(int& num, int min, int max)
{
	if (num < min)
	{
		num = min;
		return -1;
	}
	if (num > max)
	{
		num = max;
		return 1;
	}
	return 0;
}

class calendar
{
	public:
		WINDOW* win;
		string type = "month";
		int day = 0;
		int month = 0;
		int year = 2021;

		int start_day;

		int cursor_x = 8;
		int cursor_y = 1;
		int screen_y = 0;
		int screen_x = 0;

	void initialize()
	{
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 31; j++)
			{
				for (int k = 0; k < 200; k++)
				{
					for (int l = 0; l < 200; l++)
					{
						text[i][j][k][l] = ' ';
					}
				}
			}
		}
	}

	void clearScreen()
	{
		for (int i = 0; i < LINES; i++)
		{
			for (int j = 0; j < COLS; j++)
			{
				mvwaddch(win, i, j, ' ');
			}
		}
	}

	void normalize()
	{
		if (type == "day")
		{
			if (day < 0)
			{
				month--;
			}
			else if (day > months[month]-1)
			{
				month++;
				day = 0;
			}

		if (month < 0)
		{
			month = 11;
			day = months[month]-1;
			year--;
		}
		else if (month > 11)
		{
			month = 0;
			day = 0;
			year++;
		}

		if (cursor_y < 1)
			{
				cursor_y = 1;
				if (screen_y > 0)
				{
					screen_y--;
				}
			}
		if (cursor_y > LINES-1)
		{
			cursor_y = LINES-1;
			if (screen_y < 97 - LINES)
			{
				screen_y++;
			}
		}
			clamp(cursor_x, 8, COLS);
		}

		else if (type == "month")
		{
			clamp(cursor_y, 2, (5 * round(LINES / 6) + 2));

			if (cursor_x > (6 * round(COLS / 7) + 1) && cursor_y != (5 * round(LINES / 6) + 2))
			{
				cursor_x = 1;
				cursor_y += round(LINES / 6);
			}
			else if (cursor_x < 1 && cursor_y != 2)
			{
				cursor_x = (6 * round(COLS / 7) + 1);
				cursor_y -= round(LINES / 6);
			}
			else
			{
				day -= clamp(cursor_x, 1, (6 * round(COLS / 7) + 1));
			}

			if (month < 0)
			{
				month = 11;
				year--;
			}
			else if (month > 11)
			{
				month = 0;
				year++;
			}
		}
	}

	void drawInfo(int start_time)
	{
		if (type == "day")
		{
			for (int i = 1; i < LINES; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					mvwaddch(win, i, j, secondsToTime((start_time + i - 1) * 15).at(j));
				}
				mvwaddch(win, i, 6, '|');
			}

			string title = makePrintable(year, 4) + " " + makePrintable(month+1, 2) + " " + makePrintable(day+1, 2);

			for (int i = 0; i < title.length(); i++)
			{
				mvwaddch(win, 0, i, title.at(i));
			}
		}
		else if (type == "month")
		{
			int month_lines = 6;
			int month_cols = 7;
			int month_line_size = round(LINES / month_lines);
			int month_col_size = round(COLS / month_cols);
			for (int i = 0; i < 6; i++)
			{
				for (int j = 0; j < 7; j++)
				{
					mvwaddch(win, (i * month_line_size + 2), (j * month_col_size + 1), ' ');
					mvwaddch(win, (i * month_line_size + 2), (j * month_col_size + 2), ' ');
					for (int k = 0; k < month_line_size; k++)
					{
						for (int l = 0; l < month_col_size; l++)
						{
							mvwaddch(win, ( i      * month_line_size + k) + 1, ( j      * month_col_size    ), ACS_CKBOARD);
							mvwaddch(win, ( i      * month_line_size + k) + 1, ((j + 1) * month_col_size    ), ACS_CKBOARD);
							mvwaddch(win, ( i      * month_line_size	  ) + 1, ( j      * month_col_size + l), ACS_CKBOARD);
							mvwaddch(win, ((i + 1) * month_line_size    ) + 1, ( j      * month_col_size + l), ACS_CKBOARD);
						}
					}
				}
			}
			start_day = weekDay(year, month + 1, 1);
			for (int i = start_day; i < start_day + months[month]; i++)
			{
				int line = floor(i / 7);
				int col = i % 7;
				string date = makePrintable(i - start_day + 1, 2);
				for (int j = 0; j < 2; j++)
				{
					mvwaddch(win, (line * month_line_size + 2), (col * month_col_size + 1 + j), date[j]);
				}
			}

			string title = makePrintable(year, 4) + " " + makePrintable(month+1, 2);
			for (int i = 0; i < title.length(); i++)
			{
				mvwaddch(win, 0, i, title.at(i));
			}
		}
	}

	bool userInput()
	{
		int ch = getch();
		if (type == "day")
		{
			switch(ch)
			{
				case 31 ... 126:
					text[month][day][cursor_y][cursor_x] = ch;
					cursor_x++;
					return true;
					break;

				case 127:
					cursor_x--;
					text[month][day][cursor_y][cursor_x] = ' ';
					return true;
					break;

				case KEY_DC:
					text[month][day][cursor_y][cursor_x] = ' ';
					cursor_x++;
					return true;
					break;

				case 19:
					//save(text, file);
					return true;
					break;

				case 17:
					endwin();
					//file.close();
					exit(0);
					break;

				case 27:
					cursor_x = (weekDay(year, month+1, day+1) * round(COLS / 7) + 1);
					cursor_y = (floor(day / 6) * round(LINES / 6) + 2);
					clearScreen();
					type = "month";
					return true;
					break;

				case KEY_SRIGHT:
					day++;
					return true;
					break;

				case KEY_SLEFT:
					day--;
					return true;
					break;

				case KEY_UP:
					cursor_y--;
					cursor_x = 8;
					return true;
					break;

				case KEY_DOWN:
					cursor_y++;
					cursor_x = 8;
					return true;
					break;

				case KEY_LEFT:
					cursor_x--;
					return true;
					break;

				case KEY_RIGHT:
					cursor_x++;
					return true;
					break;
			}
		}

		else if (type == "month")
		{
			switch(ch)
			{
			case 17:
				endwin();
				//file.close();
				exit(0);
				return true;
				break;
			case KEY_SRIGHT:
				month++;
				normalize();
				cursor_x = (weekDay(year, month+1, 1) * round(COLS / 7) + 1);
				day = 0;
				return true;
				break;
			case KEY_SLEFT:
				month--;
				normalize();
				cursor_x = (weekDay(year, month+1, 1) * round(COLS / 7) + 1);
				day = 0;
				return true;
				break;
			case KEY_UP:
				cursor_y -= round(LINES / 6);
				day -= 7;
				return true;
				break;
			case KEY_DOWN:
				cursor_y += round(LINES / 6);
				day += 7;
				return true;
				break;
			case KEY_LEFT:
				cursor_x -= round(COLS / 7);
				day--;
				return true;
				break;
			case KEY_RIGHT:
				cursor_x += round(COLS / 7);
				day++;
				return true;
				break;
			case 10:
				clearScreen();
				if (day >= 0 && day < months[month]) type = "day";
				cursor_x = 8;
				cursor_y = 1;
				return true;
				break;
			}
		}
		return false;
	}

	void drawText()
	{
		if (type == "day")
		{
			for (int i = 1; i < LINES; i++)
			{
				for (int j = 8; j < COLS; j++)
				{
					mvwaddch(win, i - screen_y, j, char(text[month][day][i][j]));
				}
			}
		}

		else if (type == "month")
		{

		}

		wmove(win, cursor_y, cursor_x);
	}
};

int main (int argc, char **argv)
{
	initscr();
	cbreak();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	WINDOW* win = newwin(LINES, COLS, 0, 0);

	calendar cal;

	cal.win = win;
	cal.type = "month";
	cal.cursor_x = (5 * round(COLS / 7) + 1);
	cal.cursor_y = 2;

	cal.initialize();
	cal.userInput();
	cal.normalize();
	cal.drawInfo(cal.screen_y);
	cal.drawText();
	wrefresh(win);

	while (1)
	{
		if (cal.userInput())
		{
			cal.normalize();
			cal.drawInfo(cal.screen_y);
			cal.drawText();
			wrefresh(win);
		}
	}

	//file.close();
	endwin();

    return 0;
}
