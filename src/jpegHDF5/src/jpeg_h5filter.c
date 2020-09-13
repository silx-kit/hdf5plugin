/*
 * JPEG HDF5 filter
 *
 * Author: Mark Rivers <rivers@cars.uchicago.edu>
 * Created: 2019
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jpeglib.h"
#include "jpeg_h5filter.h"


#define PUSH_ERR(func, minor, str)                                      \
    H5Epush(H5E_DEFAULT, __FILE__, func, __LINE__, H5E_ERR_CLS, H5E_PLINE, minor, str)


size_t jpeg_h5_filter(unsigned int flags, size_t cd_nelmts,
           const unsigned int cd_values[], size_t nbytes,
           size_t *buf_size, void **buf) {

    struct jpeg_error_mgr jpegErr;
    size_t buf_size_out=0;
    void *out_buf=0;

    if (flags & H5Z_FLAG_REVERSE) {
        /* Decompress */
        struct jpeg_decompress_struct jpegInfo;
        unsigned long compressedSize;
        size_t elem_size=1;
        int err=0;
        char msg[80];
        unsigned char *output;

        jpeg_create_decompress(&jpegInfo);
        jpegInfo.err = jpeg_std_error(&jpegErr);
    
        compressedSize = (unsigned long) nbytes;
        jpeg_mem_src(&jpegInfo, *buf,  compressedSize);
        jpeg_read_header(&jpegInfo, TRUE);
        jpeg_start_decompress(&jpegInfo);
        buf_size_out = elem_size * jpegInfo.output_height * jpegInfo.output_width * jpegInfo.output_components;
        out_buf = H5allocate_memory(buf_size_out, false);
        if (out_buf == NULL) {
            PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, 
                    "Could not allocate output buffer.");
            return 0;
        }
        output = out_buf;
    
        while (jpegInfo.output_scanline < jpegInfo.output_height) {
            unsigned char *row_pointer[1] = { output };
    
            err = jpeg_read_scanlines(&jpegInfo, row_pointer, 1);
            if (err != 1) {
                PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, 
                    "Error decoding JPEG.");
                break;
            }
            output += jpegInfo.output_width*jpegInfo.output_components;
        }   
        jpeg_finish_decompress(&jpegInfo);
        if (err == 1) {
            H5free_memory(*buf);
            *buf = out_buf;
            *buf_size = buf_size_out;
            return buf_size_out;
        } else {
            sprintf(msg, "Error in jpeg with error code %d.", err);
            PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, msg);
            H5free_memory(out_buf);
          return 0;
        }

    } else {
         /*
         * cd_values[0] = quality factor
         * cd_values[1] = nx
         * cd_values[2] = ny
         * cd_values[3] = 0=Mono, 1=RGB
         */
        struct jpeg_compress_struct jpegInfo;
        int qualityFactor;
        int colorMode;
        unsigned char *outData = NULL;
        unsigned long outSize = 0;
        JSAMPROW row_pointer[1];
        int sizeX;
        int sizeY;
        int nwrite=0;
        size_t expectedSize;
        unsigned char *pData=NULL;
        
        if (cd_nelmts != 4) {
            PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, "cd_nelmts must be 4");
            return 0;
        }
        qualityFactor = cd_values[0];
        if (qualityFactor < 1)
          qualityFactor = 1;
        if (qualityFactor > 100)
          qualityFactor = 100;
        sizeX         = cd_values[1];
        sizeY         = cd_values[2];
        colorMode     = cd_values[3];
        
        /* Sanity check to make sure we have been passed a complete image */
        expectedSize = sizeX * sizeY;
        if (colorMode == 1) expectedSize *= 3;
        if (expectedSize != nbytes) {
            PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, "nbytes does not match image size");
            return 0;
        }

        jpeg_create_compress(&jpegInfo);
        jpegInfo.err = jpeg_std_error(&jpegErr);
        jpegInfo.image_width  = (JDIMENSION)sizeX;
        jpegInfo.image_height = (JDIMENSION)sizeY;
        
        if (colorMode == 0) {
            jpegInfo.input_components = 1;
            jpegInfo.in_color_space = JCS_GRAYSCALE;
        } else {
            jpegInfo.input_components = 3;
            jpegInfo.in_color_space = JCS_RGB;
        }
    
        jpeg_set_defaults(&jpegInfo);
        jpeg_set_quality(&jpegInfo, qualityFactor, TRUE);
        jpeg_mem_dest(&jpegInfo,
                      (unsigned char **)&outData,
                      (unsigned long*) &outSize);
        jpeg_start_compress(&jpegInfo, TRUE);
    
        pData = (unsigned char *)*buf;
    
        while ((int)jpegInfo.next_scanline < sizeY) {
            row_pointer[0] = pData;
            nwrite = jpeg_write_scanlines(&jpegInfo, row_pointer, 1);
            pData += sizeX * jpegInfo.input_components;
            if (nwrite != 1) {
                PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, 
                    "Error writing JPEG data");
                goto compressFailure;
            }
        }
    
        jpeg_finish_compress(&jpegInfo);
        buf_size_out = outSize;
    
        out_buf = H5allocate_memory(buf_size_out, false);
        if (!out_buf) {
            PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, 
                "Failed to allocate JPEG array");
            goto compressFailure;
        }
        memcpy(out_buf, outData, buf_size_out);

        H5free_memory(*buf);
        if (outData)
            free(outData);
        *buf = out_buf;
        *buf_size = buf_size_out;
        return buf_size_out;
    
    compressFailure:
        PUSH_ERR("jpeg_h5_filter", H5E_CALLBACK, "Error compressing array");
        if (outData)
            free(outData);
        if (out_buf)
            H5free_memory(out_buf);
        return 0;
    }
}


H5Z_class_t jpeg_H5Filter[1] = {{
    H5Z_CLASS_T_VERS,
    (H5Z_filter_t)(JPEG_H5FILTER),
    1, 1,
    "jpeg; see https://github.com/CARS-UChicago/jpegHDF5",
    NULL,
    NULL,
    (H5Z_func_t)(jpeg_h5_filter)
}};


int jpeg_register_h5filter(void){

    int retval;

    retval = H5Zregister(jpeg_H5Filter);
    if(retval<0){
        PUSH_ERR("jpeg_register_h5filter",
                 H5E_CANTREGISTER, "Can't register JPEG filter");
    }
    return retval;
}

