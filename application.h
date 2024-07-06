
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifdef __EMUL__
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <string.h>
#endif

#include "globals.h"

#define __XSTR(__s) __STR(__s)
#define __STR(__s) #__s

#endif /* APPLICATION_H */
