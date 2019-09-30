#ifndef _HDF5_DYNAMIC_LOADING_H
#define _HDF5_DYNAMIC_LOADING_H

#include "hdf5.h"
#include <dlfcn.h>


/*Function types*/
/*H5*/
typedef herr_t (*DL_func_H5open)(void);
/*H5E*/
typedef herr_t (* DL_func_H5Epush1)(
    const char *file, const char *func, unsigned line,
    H5E_major_t maj, H5E_minor_t min, const char *str);
typedef herr_t (* DL_func_H5Epush2)(
    hid_t err_stack, const char *file, const char *func, unsigned line,
    hid_t cls_id, hid_t maj_id, hid_t min_id, const char *msg, ...);
/*H5P*/
typedef herr_t (* DL_func_H5Pget_filter_by_id2)(hid_t plist_id, H5Z_filter_t id,
    unsigned int *flags/*out*/, size_t *cd_nelmts/*out*/,
    unsigned cd_values[]/*out*/, size_t namelen, char name[]/*out*/,
    unsigned *filter_config/*out*/);
typedef int (* DL_func_H5Pget_chunk)(
	hid_t plist_id, int max_ndims, hsize_t dim[]/*out*/);
typedef herr_t (* DL_func_H5Pmodify_filter)(
    hid_t plist_id, H5Z_filter_t filter,
    unsigned int flags, size_t cd_nelmts,
    const unsigned int cd_values[/*cd_nelmts*/]);
/*H5T*/
typedef size_t (* DL_func_H5Tget_size)(
    hid_t type_id);
typedef H5T_class_t (* DL_func_H5Tget_class)(hid_t type_id);
typedef hid_t (* DL_func_H5Tget_super)(hid_t type);
typedef herr_t (* DL_func_H5Tclose)(hid_t type_id);
/*H5Z*/
typedef herr_t (* DL_func_H5Zregister)(
    const void *cls);


static struct {
    /*H5*/
    DL_func_H5open H5open;
    /*H5E*/
    DL_func_H5Epush1 H5Epush1;
    DL_func_H5Epush2 H5Epush2;
    /*H5P*/
    DL_func_H5Pget_filter_by_id2 H5Pget_filter_by_id2;
    DL_func_H5Pget_chunk H5Pget_chunk;
    DL_func_H5Pmodify_filter H5Pmodify_filter;
    /*H5T*/
    DL_func_H5Tget_size H5Tget_size;
    DL_func_H5Tget_class H5Tget_class;
    DL_func_H5Tget_super H5Tget_super;
    DL_func_H5Tclose H5Tclose;
    /*H5T*/
    DL_func_H5Zregister H5Zregister;
} DL_H5Functions = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};


/*HDF5 variables*/
hid_t H5E_CANTREGISTER_g = -1;
hid_t H5E_CALLBACK_g = -1;
hid_t H5E_PLINE_g = -1;
hid_t H5E_ERR_CLS_g = -1;


static bool is_init = false;


void init_plugin(const char * libname)
{
  	void * handle;
    
    handle = dlopen(libname, RTLD_LAZY | RTLD_LOCAL);

    if (handle != NULL) {
        /*H5*/
        DL_H5Functions.H5open = (DL_func_H5open)dlsym(handle, "H5open");
        /*H5E*/
        DL_H5Functions.H5Epush1 = (DL_func_H5Epush1)dlsym(handle, "H5Epush1");
        DL_H5Functions.H5Epush2 = (DL_func_H5Epush2)dlsym(handle, "H5Epush2");
        /*H5P*/
        DL_H5Functions.H5Pget_filter_by_id2 = (DL_func_H5Pget_filter_by_id2)dlsym(handle, "H5Pget_filter_by_id2");
        DL_H5Functions.H5Pget_chunk = (DL_func_H5Pget_chunk)dlsym(handle, "H5Pget_chunk");
        DL_H5Functions.H5Pmodify_filter = (DL_func_H5Pmodify_filter)dlsym(handle, "H5Pmodify_filter");
        /*H5T*/
        DL_H5Functions.H5Tget_size = (DL_func_H5Tget_size)dlsym(handle, "H5Tget_size");
        DL_H5Functions.H5Tget_class = (DL_func_H5Tget_class)dlsym(handle, "H5Tget_class");
        DL_H5Functions.H5Tget_super = (DL_func_H5Tget_super)dlsym(handle, "H5Tget_super");
        DL_H5Functions.H5Tclose = (DL_func_H5Tclose)dlsym(handle, "H5Tclose");
        /*H5Z*/
        DL_H5Functions.H5Zregister = (DL_func_H5Zregister)dlsym(handle, "H5Zregister");

        /*Variables*/
        H5E_CANTREGISTER_g = *((hid_t *)dlsym(handle, "H5E_CANTREGISTER_g"));
        H5E_CALLBACK_g = *((hid_t *)dlsym(handle, "H5E_CALLBACK_g"));
        H5E_PLINE_g = *((hid_t *)dlsym(handle, "H5E_PLINE_g"));
        H5E_ERR_CLS_g = *((hid_t *)dlsym(handle, "H5E_ERR_CLS_g"));

        is_init = true;
    }
};


#define CALL(fallback, func, ...)\
    if(DL_H5Functions.func != NULL) {\
        return DL_H5Functions.func(__VA_ARGS__);\
    } else {\
        return fallback;\
    }


/*Function wrappers*/
/*H5*/
herr_t H5open(void)
{
CALL(0, H5open)
};

/*H5E*/
herr_t H5Epush1(const char *file, const char *func, unsigned line,
    H5E_major_t maj, H5E_minor_t min, const char *str)
{
CALL(0, H5Epush1, file, func, line, maj, min, str)
}

herr_t H5Epush2(hid_t err_stack, const char *file, const char *func, unsigned line,
    hid_t cls_id, hid_t maj_id, hid_t min_id, const char *msg, ...)
{
    return 0;  /*TODO*/
}

/*H5P*/
herr_t H5Pget_filter_by_id2(hid_t plist_id, H5Z_filter_t id,
    unsigned int *flags/*out*/, size_t *cd_nelmts/*out*/,
    unsigned cd_values[]/*out*/, size_t namelen, char name[]/*out*/,
    unsigned *filter_config/*out*/)
{
CALL(0, H5Pget_filter_by_id2, plist_id, id, flags, cd_nelmts, cd_values, namelen, name, filter_config)
}

int H5Pget_chunk(hid_t plist_id, int max_ndims, hsize_t dim[]/*out*/)
{
CALL(0, H5Pget_chunk, plist_id, max_ndims, dim)
}

herr_t H5Pmodify_filter(hid_t plist_id, H5Z_filter_t filter,
    unsigned int flags, size_t cd_nelmts,
    const unsigned int cd_values[/*cd_nelmts*/])
{
CALL(0, H5Pmodify_filter, plist_id, filter, flags, cd_nelmts, cd_values)
}

/*H5T*/
size_t H5Tget_size(hid_t type_id)
{
CALL(0, H5Tget_size, type_id)
}

H5T_class_t H5Tget_class(hid_t type_id)
{
CALL(H5T_NO_CLASS, H5Tget_class, type_id)
}


hid_t H5Tget_super(hid_t type)
{
CALL(0, H5Tget_super, type)
}

herr_t H5Tclose(hid_t type_id)
{
CALL(0, H5Tclose, type_id)
}

/*H5Z*/
herr_t H5Zregister(const void *cls)
{
CALL(0, H5Zregister, cls)
}

#endif
