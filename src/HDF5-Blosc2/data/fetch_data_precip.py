import os
import sys
import xarray as xr
import numpy as np
import s3fs
import caterva as cat


def open_zarr(year, month, datestart, dateend):
    fs = s3fs.S3FileSystem(anon=True)
    datestring = "era5-pds/zarr/{year}/{month:02d}/data/".format(year=year, month=month)
    s3map = s3fs.S3Map(datestring + "precipitation_amount_1hour_Accumulation.zarr/", s3=fs)
    precip_zarr = xr.open_dataset(s3map, engine="zarr")
    precip_zarr = precip_zarr.sel(time1=slice(np.datetime64(datestart), np.datetime64(dateend)))
    print(precip_zarr.info)
    return precip_zarr.precipitation_amount_1hour_Accumulation


# WARNING: this is for debugging purposes only. In production comment out the line below!
# if os.path.exists("precip-3m.iarr"): ia.remove_urlpath("precip-3m.iarr")
if os.path.exists("precip-3m.iarr"):
    print("Dataset %s is already here!" % "precip-3m.iarr")
    sys.exit(0)

print("Fetching data from S3 (era5-pds)...")
precip_m0 = open_zarr(1987, 10, "1987-10-01", "1987-10-30 23:59")
precip_m1 = open_zarr(1987, 11, "1987-11-01", "1987-11-30 23:59")
precip_m2 = open_zarr(1987, 12, "1987-12-01", "1987-12-30 23:59")

for path in ("precip1.cat", "precip2.cat", "precip3.cat", "precip-3m.cat"):
    if os.path.exists(path):
        cat.remove(path)

# ia.set_config_defaults(favor=ia.Favor.SPEED)
m_shape = precip_m0.shape
m_chunks = (128, 128, 256)
m_blocks = (32, 32, 32)
cat_precip0 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="precip1.cat", contiguous=True)
cat_precip1 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="precip2.cat", contiguous=True)
cat_precip2 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="precip3.cat", contiguous=True)
cat_precip = cat.empty((3,) + m_shape, itemsize=4, chunks=(1,) + m_chunks, blocks=(1,) + m_blocks,
                       urlpath="precip-3m.cat", contiguous=True)

print("Fetching and storing 1st month...")
values = precip_m0.values
cat_precip0[:] = values
cat_precip[0] = values

print("Fetching and storing 2nd month...")
values = precip_m1.values
cat_precip1[:] = values
cat_precip[1] = values

print("Fetching and storing 3rd month...")
values = precip_m2.values
cat_precip2[:] = values
cat_precip[2] = values
