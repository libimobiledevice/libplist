# libplist

*A small portable C library to handle Apple Property List files in binary, XML,
JSON, or OpenStep format.*

![](https://github.com/libimobiledevice/libplist/workflows/build/badge.svg)
![](https://github.com/libimobiledevice/libplist/workflows/CodeQL/badge.svg)

## Table of Contents
- [Features](#features)
- [Building](#building)
  - [Prerequisites](#prerequisites)
    - [Linux (Debian/Ubuntu based)](#linux-debianubuntu-based)
    - [macOS](#macos)
    - [Windows](#windows)
  - [Configuring the source tree](#configuring-the-source-tree)
  - [Building and installation](#building-and-installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [Links](#links)
- [License](#license)
- [Credits](#credits)

## Features

The project provides an interface to read and write plist files in binary,
XML, JSON, or OpenStep format alongside a command-line utility named `plistutil`.

Some key features are:

- **Formats:** Supports binary, XML, JSON, and OpenStep format
- **Utility:** Provides a `plistutil` utility for the command-line
- **Python:** Provides Cython based bindings for Python
- **Tested:** Uses fuzzing ([OSS-Fuzz](https://github.com/google/oss-fuzz)) and data compliance tests
- **Efficient:** Lean library with performance and resources in mind

## Building

### Prerequisites

You need to have a working compiler (gcc/clang) and development environent
available. This project uses autotools for the build process, allowing to
have common build steps across different platforms.
Only the prerequisites differ and they are described in this section.

#### Linux (Debian/Ubuntu based)

* Install all required dependencies and build tools:
  ```shell
  sudo apt-get install \
	build-essential \
	checkinstall \
	git \
	autoconf \
	automake \
	libtool-bin
  ```

  If you want to optionally build the documentation or Python bindings use:
  ```shell
  sudo apt-get install \
  	doxygen \
  	cython3
  ```

#### macOS

* Make sure the Xcode command line tools are installed. Then, use either [MacPorts](https://www.macports.org/)
  or [Homebrew](https://brew.sh/) to install `automake`, `autoconf`, and `libtool`.

  Using MacPorts:
  ```shell
  sudo port install libtool autoconf automake
  ```

  Using Homebrew:
  ```shell
  brew install libtool autoconf automake
  ```

  In case you want to build the documentation, install `doxygen` using the corresponding install command from above.

  If you want to build Python bindings, you need to install cython:
  ```shell
  pip3 install cython
  ```

  You might need to set a few environment variables if building of the Python bindings fail. For example, the [automated build via GitHub actions](https://github.com/libimobiledevice/libplist/blob/master/.github/workflows/build.yml)
  is setting the following environment variables:
  ```shell
  PYTHON3_BIN=`xcrun -f python3`
  export PYTHON=$PYTHON3_BIN
  PYTHON_VER=`$PYTHON3_BIN -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('VERSION'))"`
  PYTHON_EXEC_PREFIX=`$PYTHON3_BIN -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('exec_prefix'))"`
  PYTHON_LIBS_PATH=$PYTHON_EXEC_PREFIX/lib
  PYTHON_FRAMEWORK_PATH=$PYTHON_EXEC_PREFIX/Python3
  export PYTHON_LIBS="-L$PYTHON_LIBS_PATH -lpython$PYTHON_VER"
  export PYTHON_EXTRA_LDFLAGS="-Wl,-stack_size,1000000  -framework CoreFoundation $PYTHON_FRAMEWORK_PATH"
  ```

#### Windows

* Using [MSYS2](https://www.msys2.org/) is the official way of compiling this project on Windows. Download the MSYS2 installer
  and follow the installation steps.

  It is recommended to use the _MSYS2 MinGW 64-bit_ shell. Run it and make sure the required dependencies are installed:

  ```shell
  pacman -S base-devel \
  	git \
  	mingw-w64-x86_64-gcc \
  	make \
  	libtool \
  	autoconf \
  	automake-wrapper
  ```
  NOTE: You can use a different shell and different compiler according to your needs. Adapt the above command accordingly.

  If you want to optionally build Python bindings, you need to also install `cython`
  and make sure you have a working python environment.

  ```shell
  pacman -S cython
  ```

### Configuring the source tree

You can build the source code from a git checkout, or from a `.tar.bz2` release tarball from [Releases](https://github.com/libimobiledevice/libplist/releases).
Before we can build it, the source tree has to be configured for building. The steps depend on where you got the source from.

* **From git**

  If you haven't done already, clone the actual project repository and change into the directory.
  ```shell
  git clone https://github.com/libimobiledevice/libplist.git
  cd libplist
  ```

  Configure the source tree for building:
  ```shell
  ./autogen.sh
  ```

* **From release tarball (.tar.bz2)**

  When using an official [release tarball](https://github.com/libimobiledevice/libplist/releases) (`libplist-x.y.z.tar.bz2`)
  the procedure is slightly different.

  Extract the tarball:
  ```shell
  tar xjf libplist-x.y.z.tar.bz2
  cd libplist-x.y.z
  ```

  Configure the source tree for building:
  ```shell
  ./configure
  ```

Both `./configure` and `./autogen.sh` (which generates and calls `configure`) accept a few options, for example `--enable-debug` to allow
printing debug messages in the final product, or `--without-cython` to skip building the Python bindings.
You can simply pass them like this:

```shell
./autogen.sh --prefix=/usr/local --enable-debug --without-cython
```
or
```shell
./configure --prefix=/usr/local --enable-debug
```

Once the command is successful, the last few lines of output will look like this:
```
[...]
config.status: creating config.h
config.status: executing depfiles commands
config.status: executing libtool commands

Configuration for libplist 2.3.1:
-------------------------------------------

  Install prefix ..........: /usr/local
  Debug code ..............: yes
  Python bindings .........: yes

  Now type 'make' to build libplist 2.3.1,
  and then 'make install' for installation.
```

### Building and installation

If you followed all the steps successfully, and `autogen.sh` or `configure` did not print any errors,
you are ready to build the project. This is simply done with

```shell
make
```

If no errors are emitted you are ready for installation. Depending on whether
the current user has permissions to write to the destination directory or not,
you would either run
```shell
make install
```
_OR_
```shell
sudo make install
```

If you are on Linux, you want to run `sudo ldconfig` after installation to
make sure the installed libraries are made available.

## Usage

Usage is simple; `libplist` has a straight-forward API. It is used in [libimobiledevice](https://github.com/libimobiledevice/libimobiledevice)
and [corresponding projects](https://github.com/libimobiledevice/).

Furthermore, it comes with a command line utility `plistutil` that is really easy to use:
```shell
plistutil -i foobar.plist -o output.plist
```
This converts the `foobar.plist` file to the opposite format, e.g. binary to
XML or vice versa, and outputs it to the `output.plist` file.

To convert to a specific format - and also to convert from JSON or OpenStep
format - use the `-f` command line switch:
```shell
plistutil -i input.plist -f json
```
This will convert `input.plist`, regardless of the input format, to JSON. The
code auto-detects the input format and parses it accordingly.

Please consult the usage information or manual page for a full documentation of
available command line options:
```shell
plistutil --help
```
or
```shell
man plistutil
```

## Contributing

We welcome contributions from anyone and are grateful for every pull request!

If you'd like to contribute, please fork the `master` branch, change, commit and
send a pull request for review. Once approved it can be merged into the main
code base.

If you plan to contribute larger changes or a major refactoring, please create a
ticket first to discuss the idea upfront to ensure less effort for everyone.

Please make sure your contribution adheres to:
* Try to follow the code style of the project
* Commit messages should describe the change well without being too short
* Try to split larger changes into individual commits of a common domain
* Use your real name and a valid email address for your commits

## Links

* Homepage: https://libimobiledevice.org/
* Repository: https://git.libimobiledevice.org/libplist.git
* Repository (Mirror): https://github.com/libimobiledevice/libplist.git
* Issue Tracker: https://github.com/libimobiledevice/libplist/issues
* Mailing List: https://lists.libimobiledevice.org/mailman/listinfo/libimobiledevice-devel
* Twitter: https://twitter.com/libimobiledev

## License

This project is licensed under the [GNU Lesser General Public License v2.1](https://www.gnu.org/licenses/lgpl-2.1.en.html),
also included in the repository in the `COPYING` file.

## Credits

Apple, iPhone, iPad, iPod, iPod Touch, Apple TV, Apple Watch, Mac, iOS,
iPadOS, tvOS, watchOS, and macOS are trademarks of Apple Inc.

This project is an independent software library and has not been authorized,
sponsored, or otherwise approved by Apple Inc.

README Updated on: 2024-02-21

