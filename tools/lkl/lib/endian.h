#ifndef _LKL_LIB_ENDIAN_H
#define _LKL_LIB_ENDIAN_H

#ifndef __MINGW32__
#ifdef __FreeBSD__
#include <sys/endian.h>
#else
#include <endian.h>
#endif  /* __FreeBSD__ */
#else  /* !__MINGW32__ */
#define le32toh(x) (x)
#define le16toh(x) (x)
#define htole32(x) (x)
#define htole16(x) (x)
#define le64toh(x) (x)
#endif  /* __MINGW32__ */

#define htonl(x) htole32(x)
#define htons(x) htole16(x)
#define ntohl(x) le32toh(x)
#define ntohs(x) le16toh(x)

#endif /* _LKL_LIB_ENDIAN_H */
