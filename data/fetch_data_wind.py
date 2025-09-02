import os
import sys
import xarray as xr
import numpy as np
import s3fs
import caterva as cat


def open_zarr(year, month, datestart, dateend):
    fs = s3fs.S3FileSystem(anon=True)
    datestring = "era5-pds/zarr/{year}/{month:02d}/data/".format(year=year, month=month)
    s3map = s3fs.S3Map(datestring + "eastward_wind_at_10_metres.zarr/", s3=fs)
    wind_zarr = xr.open_dataset(s3map, engine="zarr")
    print("Datestart %s dateend %s slice %s" % (np.datetime64(datestart), np.datetime64(dateend), slice(np.datetime64(datestart), np.datetime64(dateend))))
    print(wind_zarr.info)
    wind_zarr = wind_zarr.sel(time0=slice(np.datetime64(datestart), np.datetime64(dateend)))

    return wind_zarr.eastward_wind_at_10_metres


# WARNING: this is for debugging purposes only. In production comment out the line below!
# if os.path.exists("wind-3m.iarr"): ia.remove_urlpath("wind-3m.iarr")
if os.path.exists("wind-3m.iarr"):
    print("Dataset %s is already here!" % "wind-3m.iarr")
    sys.exit(0)

print("Fetching data from S3 (era5-pds)...")
wind_m0 = open_zarr(2015, 10, "2015-10-01", "2015-10-30 23:59")
wind_m1 = open_zarr(2015, 11, "2015-11-01", "2015-11-30 23:59")
wind_m2 = open_zarr(2015, 12, "2015-12-01", "2015-12-30 23:59")

for path in ("wind1.cat", "wind2.cat", "wind3.cat", "wind-3m.cat"):
    if os.path.exists(path):
        cat.remove(path)

# ia.set_config_defaults(favor=ia.Favor.SPEED)
m_shape = wind_m0.shape
m_chunks = (128, 128, 256)
m_blocks = (16, 32, 64)
cat_wind0 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="wind1.cat", contiguous=True)
cat_wind1 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="wind2.cat", contiguous=True)
cat_wind2 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="wind3.cat", contiguous=True)
cat_wind = cat.empty((3,) + m_shape, itemsize=4, chunks=(1,) + m_chunks, blocks=(1,) + m_blocks,
                       urlpath="wind-3m.cat", contiguous=True)

print("Fetching and storing 1st month...")
values = wind_m0.values
cat_wind0[:] = values
cat_wind[0] = values

print("Fetching and storing 2nd month...")
values = wind_m1.values
cat_wind1[:] = values
cat_wind[1] = values

print("Fetching and storing 3rd month...")
values = wind_m2.values
cat_wind2[:] = values
cat_wind[2] = values
