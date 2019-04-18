# Dockerised davix

Files in this directory enable the creation of a Docker image for davix. The
Dockerfile is able to build all branches and tags from the repository. The
branch/tag to build is controlled by the `DAVIX_RELEASE` [build argument].

  [build argument]: https://docs.docker.com/engine/reference/commandline/build#set-build-time-variables---build-arg

## Building

To build this image with the default branch, which is the `devel` branch, run
the following command. This will create an image called `davix`:

```shell
docker build -t davix .
```

To build this image at a given release instead, run a command similar to the
following one. This will create an image tagged after the name of the release:

```shell
docker build -t davix:0.7.2 --build-arg DAVIX_RELEASE=R_0_7_2 .
```

## Running

Given a Docker image called `davix`, you should be able to call the various
`davix` binaries without their leading `davix-` preamble in the binary name.
This is achieved through declaring [`davix.sh`] as the [ENTRYPOINT] of the
image.

  [`davix.sh`]: ./davix.sh
  [ENTRYPOINT]: https://docs.docker.com/engine/reference/builder/#entrypoint

For example, to call the `davix-ls` binary inside the Docker image `davix`, you
could run a command similar to the following:

```shell
docker run -it --rm davix ls --help
```
