#include "syscall.h"

int
main()
{
	SpaceId newProc;
	OpenFileId input = ConsoleInput;
	OpenFileId output = ConsoleOutput;
	char prompt[2], ch, buffer[60], redirBuffer[60];
	int i, redir, redirOut, redirIn;

	prompt[0] = '-';
	prompt[1] = '-';

	while( 1 ) {
		Write(prompt, 2, output);
		i = 0;
		ch = '\0';
		redir= 0;
		redirOut = 0;
		redirIn  = 0;
		while (i < 60 && ch != '\n' && redir < 60) {
			Read(&ch, 1, input);
			if (ch == '>') {
				redirOut = 1;
			} else if (ch == '<') {
				redirIn = 1;
			} else {
				/*have not seen any redirection yet*/
				if (!redirIn && !redirOut) {
					if (ch != ' ') {
						buffer[i] = ch;
						i++;
					}
				/*we have seen redirection*/
				} else {
					if (ch != ' ') {
						redirBuffer[redir] = ch;
						redir++;
					}
				}
			}
		}

		/*add null byte to strings*/
		if (redirOut || redirIn) {
			buffer[i] = '\0';
			redirBuffer[--redir] = '\0';
		} else {
			buffer[--i] = '\0';
		}

		if( i > 0 ) {
			newProc = Fork();
			/*child*/
			if (newProc == 0) {
				int fd;
				/*if redirection*/
				if (redir > 0) {
					if (redirIn) {
						fd = Open(redirBuffer);
						if (fd >= 0) {
							Close(0);
							Dup(fd);
							Close(fd);
						}
					} else if (redirOut) {
						Create(redirBuffer);
						fd = Open(redirBuffer);
						if (fd >= 0) {
							Close(1);
							Dup(fd);
							Close(fd);
						}
					}
				}
				/*start command*/
				Exec(buffer);
			/*parent*/
			} else {
				Join(newProc);
			}
		}
	}
}

/* Print a null-terminated string "s" on open file descriptor "file". */

prints(s,file)
char *s;
OpenFileId file;

{
  int count = 0;
  char *p;

  p = s;
  while (*p++ != '\0') count++;
  Write(s, count, file);  

}


/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i, pos=0, divisor=1000000000, d, zflag=1;
  char c;
  char buffer[11];
  
  if (n < 0) {
    buffer[pos++] = '-';
    n = -n;
  }
  
  if (n == 0) {
    Write("0",1,file);
    return;
  }

  for (i=0; i<10; i++) {
    d = n / divisor; n = n % divisor;
    if (d == 0) {
      if (!zflag) buffer[pos++] =  (char) (d % 10) + '0';
    } else {
      zflag = 0;
      buffer[pos++] =  (char) (d % 10) + '0';
    }
    divisor = divisor/10;
  }
  Write(buffer,pos,file);
}
