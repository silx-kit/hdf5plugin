import os
import sys
import xarray as xr
import numpy as np
import s3fs
import caterva as cat


def open_zarr(year, month, datestart, dateend):
    fs = s3fs.S3FileSystem(anon=True)
    datestring = "era5-pds/zarr/{year}/{month:02d}/data/".format(year=year, month=month)
    s3map = s3fs.S3Map(datestring + "snow_density.zarr/", s3=fs)
    snow_zarr = xr.open_dataset(s3map, engine="zarr")
    print("Datestart %s dateend %s slice %s" % (np.datetime64(datestart), np.datetime64(dateend), slice(np.datetime64(datestart), np.datetime64(dateend))))
    print(snow_zarr.info)
    snow_zarr = snow_zarr.sel(time0=slice(np.datetime64(datestart), np.datetime64(dateend)))

    return snow_zarr.snow_density


# WARNING: this is for debugging purposes only. In production comment out the line below!
# if os.path.exists("snow-3m.iarr"): ia.remove_urlpath("snow-3m.iarr")
if os.path.exists("snow-3m.iarr"):
    print("Dataset %s is already here!" % "snow-3m.iarr")
    sys.exit(0)

print("Fetching data from S3 (era5-pds)...")
snow_m0 = open_zarr(2015, 10, "2015-10-01", "2015-10-30 23:59")
snow_m1 = open_zarr(2015, 11, "2015-11-01", "2015-11-30 23:59")
snow_m2 = open_zarr(2015, 12, "2015-12-01", "2015-12-30 23:59")

for path in ("snow1.cat", "snow2.cat", "snow3.cat", "snow-3m.cat"):
    if os.path.exists(path):
        cat.remove(path)

# ia.set_config_defaults(favor=ia.Favor.SPEED)
m_shape = snow_m0.shape
m_chunks = (128, 128, 256)
m_blocks = (16, 32, 64)
cat_snow0 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="snow1.cat", contiguous=True)
cat_snow1 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="snow2.cat", contiguous=True)
cat_snow2 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="snow3.cat", contiguous=True)
cat_snow = cat.empty((3,) + m_shape, itemsize=4, chunks=(1,) + m_chunks, blocks=(1,) + m_blocks,
                       urlpath="snow-3m.cat", contiguous=True)

print("Fetching and storing 1st month...")
values = snow_m0.values
cat_snow0[:] = values
cat_snow[0] = values

print("Fetching and storing 2nd month...")
values = snow_m1.values
cat_snow1[:] = values
cat_snow[1] = values

print("Fetching and storing 3rd month...")
values = snow_m2.values
cat_snow2[:] = values
cat_snow[2] = values
