/**
   Il FILE terminale_lib.h.c è parte del
   QUBIT: sistema di computazione quantistica
  Copyright (C) 2018  Francesco Sisini (francescomichelesisini@gmail.com)
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "terminale_lib.h"




struct termios orig_termios;
int rows,cols;

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

int dimensioni_finestra(int *rows, int *cols) 
{
  struct winsize ws;  
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
      return -1;
    } else
    {
      *cols = ws.ws_col;
      *rows = ws.ws_row;
      return 0;
    }
}

  
void terminale_cucinato()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void terminale_crudo()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) 
    die("tcgetattr");
  
  atexit(terminale_cucinato);
  
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
    die("tcsetattr");  
}

int init()
{
  terminale_crudo();
  dimensioni_finestra(&rows, &cols);
  if(rows<40 || cols<70)
    {
      die("\n*** Ridimensionare il terminale ad almeno 70x40\n");
    }
  
  setlocale(LC_CTYPE, "");
}

int leggi_tastiera() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[')
      {
	if (seq[1] >= '0' && seq[1] <= '9')
	  {
	    if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
	    if (seq[2] == '~') {
	      switch (seq[1]) {
	      case '1': return HOME_KEY;
	      case '3': return DEL_KEY;
	      case '4': return END_KEY;
	      case '5': return PAGE_UP;
	      case '6': return PAGE_DOWN;
	      case '7': return HOME_KEY;
	      case '8': return END_KEY;
	      }
	    }
	  } else
	  {
	    switch (seq[1])
	      {
	      case 'A': return ARROW_UP;
	      case 'B': return ARROW_DOWN;
	      case 'C': return ARROW_RIGHT;
	      case 'D': return ARROW_LEFT;
	      case 'H': return HOME_KEY;
	      case 'F': return END_KEY;
	      }
	  }
      }
    
    else if (seq[0] == 'O')
      {
	switch (seq[1]) {
	case 'H': return HOME_KEY;
	case 'F': return END_KEY;
	}
      }
    return '\x1b';
    
    
  } else
    {
      return c;
      
    }
}

