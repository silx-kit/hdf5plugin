/*
 * This file is an example of an HDF5 filter plugin.
 * The plugin can be used with the HDF5 library vesrion 1.8.11+ to read
 * HDF5 datasets compressed with lz4.
 */

#include "lz4_config.h"
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#else
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#endif
#ifdef HAVE_STRING_H
#if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> /* for htonl() */
#else
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#endif

#include "H5PLextern.h"
#include "lz4.h"

static size_t H5Z_filter_lz4(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[],
                             size_t nbytes, size_t *buf_size, void **buf);

#define H5Z_FILTER_LZ4 32004

#define htonll(x) (((uint64_t)(htonl((uint32_t)((x << 32) >> 32))) << 32) | htonl(((uint32_t)(x >> 32))))
#define ntohll(x) htonll(x)

#define htobe16t(x) htons(x)
#define htobe32t(x) htonl(x)
#define htobe64t(x) htonll(x)
#define be16toht(x) ntohs(x)
#define be32toht(x) ntohl(x)
#define be64toht(x) ntohll(x)

#define DEFAULT_BLOCK_SIZE 1 << 30 /* 1GiB. LZ4 needs blocks < 1.97GiB. */

const H5Z_class2_t H5Z_LZ4[1] = {{
    H5Z_CLASS_T_VERS,             /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_LZ4, /* Filter id number             */
#ifdef FILTER_DECODE_ONLY
    0, /* encoder_present flag (false is not available) */
#else
    1, /* encoder_present flag (set to true) */
#endif
    1, /* decoder_present flag (set to true) */
    "HDF5 lz4 filter; see "
    "https://github.com/HDFGroup/hdf5_plugins/blob/master/docs/RegisteredFilterPlugins.md",
    /* Filter name for debugging    */
    NULL,                       /* The "can apply" callback     */
    NULL,                       /* The "set local" callback     */
    (H5Z_func_t)H5Z_filter_lz4, /* The actual filter function   */
}};

H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}
const void *
H5PLget_plugin_info(void)
{
    return H5Z_LZ4;
}

static size_t
H5Z_filter_lz4(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], size_t nbytes,
               size_t *buf_size, void **buf)
{
    void  *outBuf = NULL;
    size_t ret_value;

    if (flags & H5Z_FLAG_REVERSE) {
        uint32_t             *i32Buf;
        uint32_t              blockSize;
        char                 *roBuf; /* pointer to current write position */
        uint64_t              decompSize;
        const char           *rpos     = (char *)*buf; /* pointer to current read position */
        const uint64_t *const i64Buf   = (uint64_t *)rpos;
        const uint64_t        origSize = (uint64_t)(be64toht(*i64Buf)); /* is saved in be format */
        rpos += 8;                                                      /* advance the pointer */

        i32Buf    = (uint32_t *)rpos;
        blockSize = (uint32_t)(be32toht(*i32Buf));
        rpos += 4;
        if (blockSize > origSize)
            blockSize = origSize;

        if (NULL == (outBuf = malloc(origSize))) {
            printf("cannot malloc\n");
            goto error;
        }
        roBuf      = (char *)outBuf; /* pointer to current write position */
        decompSize = 0;
        /// start with the first block ///
        while (decompSize < origSize) {
            uint32_t compressedBlockSize; /// is saved in be format

            if (origSize - decompSize < blockSize) /* the last block can be smaller than blockSize. */
                blockSize = origSize - decompSize;
            i32Buf              = (uint32_t *)rpos;
            compressedBlockSize = be32toht(*i32Buf); /// is saved in be format
            rpos += 4;
            if (compressedBlockSize == blockSize) /* there was no compression */
            {
                memcpy(roBuf, rpos, blockSize);
            }
            else /* do the decompression */
            {
                int compressedBytes = LZ4_decompress_safe(rpos, roBuf, compressedBlockSize, blockSize);
                if (compressedBytes != blockSize) {
                    printf("decompressed size not the same: %d, != %d\n", compressedBytes, blockSize);
                    goto error;
                }
            }

            rpos += compressedBlockSize; /* advance the read pointer to the next block */
            roBuf += blockSize;          /* advance the write pointer */
            decompSize += blockSize;
        }
        free(*buf);
        *buf      = outBuf;
        *buf_size = (size_t)origSize;
        outBuf    = NULL;
        ret_value =
            (size_t)origSize; // should always work, as orig_size cannot be > 2GB (sizeof(size_t) < 4GB)
    }
    else /* forward filter */
    {
        size_t    blockSize;
        size_t    nBlocks;
        size_t    outSize; /* size of the output buffer. Header size (12 bytes) is included */
        size_t    block;
        uint64_t *i64Buf;
        uint32_t *i32Buf;
        size_t    maxDestSize;
        char     *rpos;  /* pointer to current read position */
        char     *roBuf; /* pointer to current write position */

        if (nbytes > INT32_MAX) {
            /* can only compress chunks up to 2GB */
            goto error;
        }

        if (cd_nelmts > 0 && cd_values[0] > 0) {
            blockSize = cd_values[0];
        }
        else {
            blockSize = DEFAULT_BLOCK_SIZE;
        }
        if (blockSize > nbytes) {
            blockSize = nbytes;
        }
        nBlocks     = (nbytes - 1) / blockSize + 1;
        maxDestSize = nBlocks * LZ4_compressBound(blockSize) + 4 + 8 + nBlocks * 4;
        if (NULL == (outBuf = malloc(maxDestSize))) {
            goto error;
        }

        rpos  = (char *)*buf;   /* pointer to current read position */
        roBuf = (char *)outBuf; /* pointer to current write position */
        /* header */
        i64Buf    = (uint64_t *)(roBuf);
        i64Buf[0] = htobe64t((uint64_t)nbytes); /* Store decompressed size in be format */
        roBuf += 8;

        i32Buf    = (uint32_t *)(roBuf);
        i32Buf[0] = htobe32t((uint32_t)blockSize); /* Store the block size in be format */
        roBuf += 4;

        outSize = 12; /* size of the output buffer. Header size (12 bytes) is included */

        for (block = 0; block < nBlocks; ++block) {
            uint32_t compBlockSize; /// reserve space for compBlockSize
            size_t   origWritten = block * blockSize;
            if (nbytes - origWritten < blockSize) /* the last block may be < blockSize */
                blockSize = nbytes - origWritten;

            compBlockSize = LZ4_compress_default(
                rpos, roBuf + 4, blockSize, LZ4_compressBound(blockSize)); /// reserve space for compBlockSize
            if (!compBlockSize)
                goto error;
            if (compBlockSize >= blockSize) /* compression did not save any space, do a memcpy instead */
            {
                compBlockSize = blockSize;
                memcpy(roBuf + 4, rpos, blockSize);
            }

            i32Buf    = (uint32_t *)(roBuf);
            i32Buf[0] = htobe32t((uint32_t)compBlockSize); /* write blocksize */
            roBuf += 4;

            rpos += blockSize;      /* advance read pointer */
            roBuf += compBlockSize; /* advance write pointer */
            outSize += compBlockSize + 4;
        }

        free(*buf);
        *buf      = outBuf;
        *buf_size = outSize;
        outBuf    = NULL;
        ret_value = outSize;
    }

    if (outBuf)
        free(outBuf);
    return ret_value;

error:
    if (outBuf)
        free(outBuf);
    outBuf = NULL;
    return 0;
}
