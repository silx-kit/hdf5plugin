import os
import sys
import xarray as xr
import numpy as np
import s3fs
import caterva as cat


def open_zarr(year, month, datestart, dateend):
    fs = s3fs.S3FileSystem(anon=True)
    datestring = "era5-pds/zarr/{year}/{month:02d}/data/".format(year=year, month=month)
    s3map = s3fs.S3Map(datestring + "integral_wrt_time_of_surface_direct_downwelling_shortwave_flux_in_air_1hour_Accumulation.zarr/", s3=fs)
    solar_zarr = xr.open_dataset(s3map, engine="zarr")
    print("Datestart %s dateend %s slice %s" % (np.datetime64(datestart), np.datetime64(dateend), slice(np.datetime64(datestart), np.datetime64(dateend))))
    print(solar_zarr.info)
    solar_zarr = solar_zarr.sel(time1=slice(np.datetime64(datestart), np.datetime64(dateend)))

    return solar_zarr.integral_wrt_time_of_surface_direct_downwelling_shortwave_flux_in_air_1hour_Accumulation


# WARNING: this is for debugging purposes only. In production comment out the line below!
# if os.path.exists("solar-3m.iarr"): ia.remove_urlpath("solar-3m.iarr")
if os.path.exists("solar-3m.iarr"):
    print("Dataset %s is already here!" % "solar-3m.iarr")
    sys.exit(0)

print("Fetching data from S3 (era5-pds)...")
solar_m0 = open_zarr(2015, 10, "2015-10-01", "2015-10-30 23:59")
solar_m1 = open_zarr(2015, 11, "2015-11-01", "2015-11-30 23:59")
solar_m2 = open_zarr(2015, 12, "2015-12-01", "2015-12-30 23:59")

for path in ("solar1.cat", "solar2.cat", "solar3.cat", "solar-3m.cat"):
    if os.path.exists(path):
        cat.remove(path)

# ia.set_config_defaults(favor=ia.Favor.SPEED)
m_shape = solar_m0.shape
m_chunks = (128, 128, 256)
m_blocks = (16, 32, 64)
cat_solar0 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="solar1.cat", contiguous=True)
cat_solar1 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="solar2.cat", contiguous=True)
cat_solar2 = cat.empty(m_shape, itemsize=4, chunks=m_chunks, blocks=m_blocks,
                        urlpath="solar3.cat", contiguous=True)
cat_solar = cat.empty((3,) + m_shape, itemsize=4, chunks=(1,) + m_chunks, blocks=(1,) + m_blocks,
                       urlpath="solar-3m.cat", contiguous=True)

print("Fetching and storing 1st month...")
values = solar_m0.values
cat_solar0[:] = values
cat_solar[0] = values

print("Fetching and storing 2nd month...")
values = solar_m1.values
cat_solar1[:] = values
cat_solar[1] = values

print("Fetching and storing 3rd month...")
values = solar_m2.values
cat_solar2[:] = values
cat_solar[2] = values
