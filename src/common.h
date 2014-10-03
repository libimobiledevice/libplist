#ifndef COMMON_H
#define COMMON_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define PLIST_LITTLE_ENDIAN 0
#define PLIST_BIG_ENDIAN 1

#ifndef PLIST_BYTE_ORDER
#if __BIG_ENDIAN__ == 1
#define PLIST_BYTE_ORDER PLIST_BIG_ENDIAN
#endif
#if __LITTLE_ENDIAN__ == 1
#define PLIST_BYTE_ORDER PLIST_LITTLE_ENDIAN
#endif
#endif

#endif
