#ifndef COMMON_H
#define COMMON_H

#define PLIST_LITTLE_ENDIAN 0
#define PLIST_BIG_ENDIAN 1

#if defined(__GNUC__) && (__GNUC__ >= 4) && !defined(__CYGWIN__) && !defined(WIN32)
# define _PLIST_INTERNAL      __attribute__((visibility("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
# define _PLIST_INTERNAL      __hidden
#else /* not gcc >= 4 and not Sun Studio >= 8 */
# define _PLIST_INTERNAL
#endif /* GNUC >= 4 */

#endif
