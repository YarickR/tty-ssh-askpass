all: tty-ssh-askpass

CFLAGS=-c -O2 -Wall -Wextra -Wpedantic
LDFLAGS=

tty-ssh-askpass: tty-ssh-askpass.o
	$(CC) $(LDFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) tty-ssh-askpass tty-ssh-askpass.o

