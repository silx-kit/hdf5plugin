/*
 * ZSTD HDF5 filter
 *
 * Author: Mark Rivers <rivers@cars.uchicago.edu>
 * Created: 2019
 *
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "H5PLextern.h"

#include "zstd.h"

static size_t H5Z_filter_zstd(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[],
                              size_t nbytes, size_t *buf_size, void **buf);

#define H5Z_FILTER_ZSTD 32015

#define PUSH_ERR(func, minor, str)                                                                           \
    H5Epush(H5E_DEFAULT, __FILE__, func, __LINE__, H5E_ERR_CLS, H5E_PLINE, minor, str)

const H5Z_class2_t H5Z_ZSTD[1] = {{
    H5Z_CLASS_T_VERS,              /* H5Z_class_t version */
    (H5Z_filter_t)H5Z_FILTER_ZSTD, /* Filter id number             */
#ifdef FILTER_DECODE_ONLY
    0, /* encoder_present flag (false is not available) */
#else
    1, /* encoder_present flag (set to true) */
#endif
    1, /* decoder_present flag (set to true) */
    "HDF5 zstd filter; see "
    "https://github.com/HDFGroup/hdf5_plugins/blob/master/docs/RegisteredFilterPlugins.md",
    /* Filter name for debugging    */
    NULL,                        /* The "can apply" callback     */
    NULL,                        /* The "set local" callback     */
    (H5Z_func_t)H5Z_filter_zstd, /* The actual filter function   */
}};

H5PL_type_t
H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}
const void *
H5PLget_plugin_info(void)
{
    return H5Z_ZSTD;
}

static size_t
H5Z_filter_zstd(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], size_t nbytes,
                size_t *buf_size, void **buf)
{
    size_t buf_size_out = 0;
    size_t origSize     = nbytes; /* Number of bytes for output (compressed) buffer */
    void  *outbuf       = NULL;
    void  *inbuf        = NULL; /* Pointer to input buffer */
    inbuf               = *buf;

    if (flags & H5Z_FLAG_REVERSE) {
        /* We're decompressing */
        size_t decompSize = ZSTD_getFrameContentSize(*buf, origSize);
        if (NULL == (outbuf = malloc(decompSize)))
            goto error;

        decompSize = ZSTD_decompress(outbuf, decompSize, inbuf, origSize);

#ifdef ZSTD_DEBUG
        fprintf(stderr, "   decompressing nbytes: %ld\n", decompSize);
#endif

        buf_size_out = decompSize;
    }
    else {
        /* We're compressing */
        /*
         * cd_values[0] = aggression
         *
         * As of Zstandard v1.5.7
         * ZSTD_minCLevel() == -1<<17 == -131072
         * ZSTD_maxCLevel() == 22
         *
         * Negative compression levels are faster at the cost of compression
         * aggression >= 20 require more memory
         */
        int aggression;
        if (cd_nelmts > 0)
            aggression = (int)cd_values[0];
        else
            aggression = ZSTD_CLEVEL_DEFAULT;
        if (aggression < ZSTD_minCLevel())
            aggression = ZSTD_minCLevel();
        else if (aggression > ZSTD_maxCLevel())
            aggression = ZSTD_maxCLevel();

        size_t compSize = ZSTD_compressBound(origSize);
        if (NULL == (outbuf = malloc(compSize)))
            goto error;

        compSize = ZSTD_compress(outbuf, compSize, inbuf, origSize, aggression);

#ifdef ZSTD_DEBUG
        fprintf(stderr, "    compressing nbytes: %ld\n", compSize);
#endif

        buf_size_out = compSize;
    }
    free(*buf);
    *buf      = outbuf;
    *buf_size = buf_size_out;
    return buf_size_out;

error:
    if (outbuf)
        free(outbuf);
    return 0;
}
