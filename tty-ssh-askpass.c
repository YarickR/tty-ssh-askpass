#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

void printusage(char *argv0, unsigned long buflen) {
  fwprintf(stderr, L"Usage: %s [-c <symbol to use as a mask, default '*'>] [-l <max password length>, default %ld symbols] [ -n start in hidden mode, Tab to switch to mask mode, default to start in mask mode] [-h this help]", argv0, buflen); 
}
int main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind, opterr, optopt;
  wchar_t *buf; 
  unsigned long buflen;
  wchar_t starchar[3];
  int hidden;
  mbstate_t mbs;
  buflen = 1024; /* sane default */
  setlocale(LC_CTYPE, "");
  wcsncpy(starchar, L"*", 3);
  memset(&mbs, 0, sizeof(mbs));
  mbsinit(&mbs);
  hidden = 0;
  int nextarg = 0;
  while (nextarg != -1) {
    nextarg = getopt(argc, argv, "c:l:nhi");
    wchar_t charline[3]; /* windows may use two wchar_t to represent one symbol */
    switch (nextarg) {
      case -1:
        break;
      case '?':
        printusage(argv[0], buflen);
        return 1;
      case 'c':
        memset(charline, 0, sizeof(charline));
        mbrtowc(charline, optarg, 4, &mbs);
        if (errno == 0) {
          memset(starchar, 0, sizeof(starchar));
          wcsncpy(starchar, charline, 3);
        } else {
          fwprintf(stderr, L"Error processing mask symbol argument, will use default %ls\n", starchar);
          fflush(stderr);
        };
        break;
      case 'l':
        buflen = strtoul(optarg, NULL, 10);
        if ((buflen == 0) || (buflen > 1024)) {
          buflen = 1024; /* No password could be 1K symbols long, or GTFO */
#if defined DEBUG
          fwprintf(stderr, L"Sanitizing buffer length by adjusting it to %ld\n", buflen);
          fflush(stderr);
#endif
        };
        break;
      case 'n':
        hidden = 1;
        break;
      case 'h':
        printusage(argv[0], buflen);
        return 1;
      default:
        printusage(argv[0], buflen);
        return 2;
      };
  };
  if ((optind > 0) && (argv[optind] != NULL)) {
    fwprintf(stderr, L"%s", argv[optind]);
    fflush(stderr);
  };
  buf = malloc((buflen + 1) * sizeof(wchar_t));
  wmemset(buf, 0, buflen + 1);
  unsigned long bufptr = 0;
  int stop = 0;
#if defined DEBUG 
  fwprintf(stderr, L"Using %ls as mask character, buffer is %ld symbols long\n", starchar, buflen);
  fflush(stderr);
#endif
  struct termios term; /* straight from stackoverflow */
  tcgetattr(fileno(stdin), &term);
  term.c_lflag &= ~(ECHO | ICANON);
  term.c_cc[VMIN] = 1;
  term.c_cc[VTIME] = 0;
  tcsetattr(fileno(stdin), 0, &term);
  while (!stop) {
    wint_t c = getwchar();
    switch (c) {
      case L'\015':
      case L'\012':
        stop = 1;
        break;
      case WEOF:
        stop = 1;
        break;
      case L'\010':
      case L'\177':
        if (bufptr > 0) {
          fwprintf(stderr, L"\b \b");
          fflush(stderr);
          buf[--bufptr] = L'\0';
        };
        break;
      case L'\011':
        hidden = !hidden;
        break;
      default:
        if (!hidden) {
          fwprintf(stderr, L"%ls", starchar);
          fflush(stderr);
        };
        buf[bufptr++] = c;
        if (bufptr >= buflen) {
          stop = 1;
        };
        break;
    };
  };
  fwprintf(stderr, L"\n");
  fflush(stderr);
  wprintf(L"%ls\n", buf);
  free(buf);
  term.c_lflag |= (ECHO | ICANON);
  tcsetattr(fileno(stdin), 0, &term);
  return 0;
}
