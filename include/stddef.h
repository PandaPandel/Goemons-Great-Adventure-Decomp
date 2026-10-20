#ifndef _STDDEF_H
#define _STDDEF_H
/* The one libc header zlib 1.1.3 (lib/zlib) needs that ultralib's IDO headers lack. size_t is
 * guarded the way theirs is (compiler/ido/string.h). */

#ifndef _SIZE_T_DEF
#define _SIZE_T_DEF
typedef unsigned size_t;
#endif

typedef int ptrdiff_t;

#ifndef NULL
#define NULL 0
#endif

#endif
