#ifndef SIGNAMES_H
#define SIGNAMES_H

#include <signal.h>
#ifdef __cplusplus
extern "C" {
#endif

extern const char *const signal_names[NSIG + 1];

#ifdef __cplusplus
}
#endif

#endif
