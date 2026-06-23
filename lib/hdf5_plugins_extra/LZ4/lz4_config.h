#define HAVE_SYS_TYPES_H
#define STDC_HEADERS
#define HAVE_STRING_H
#define HAVE_STDINT_H

#if defined(_WIN32)
#define HAVE_WINSOCK2_H
#else
#define HAVE_ARPA_INET_H
#endif
