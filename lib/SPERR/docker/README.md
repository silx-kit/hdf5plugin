## Build Your Own Image

A `Dockerfile` is located [here](https://github.com/NCAR/SPERR/tree/main/docker) so you can build your own SPERR images.
- Build an image: enter the same folder with `Dockerfile`, and then type `docker build -t sperr-docker:tag1 .`.
- Give an existing image a new tag: `docker tag <image-sha256> sperr-docker:<new-tag>`.

## Use An Image

- Run this image in a container: `docker run -it shaomeng/sperr-docker:<tag-name>`
- Inside of this container is a complete development environment. SPERR source code is located at `/home/Odie/SPERR-src` and SPERR is compiled at `/home/Odie/SPERR-src/build`.
- In the build directory, one can run the unit tests (`ctest .`) to verify that the executable works.
- One can also mount a directory from the host system to the container so SPERR can operate on real data:
  `docker run -it --mount type=bind,source=/absolute/path/to/host/directory,target=/data,readonly shaomeng/sperr-docker:<tag-name>`


