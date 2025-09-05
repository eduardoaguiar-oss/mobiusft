# Installation Guide
May 31th, 2025 by Eduardo Aguiar


Welcome to the installation guide for Mobius Forensic Toolkit! In this
guide, we will walk you through the process of setting up and installing
Mobius Forensic Toolkit.

The package names mentioned in this guide are specific to OpenSUSE Leap
15.6. If you are using a different Linux distribution, the package names
may differ. The commands provided in this guide are for installing
version 2.15 of the software. If you are installing a different version,
you will need to modify the commands accordingly.

There are two ways to install Mobius Forensic Toolkit: by building from
source files, or by using the .AppImage bundle file. The steps for each
method are provided in the following sections.

## 1. Installing MobiusFT from source code (`.tar.xz` and `.zip`)

In this section, we will walk you through the process of configuring and
installing Mobius Forensic Toolkit using CMake.

CMake is a cross-platform build system that generates native build
scripts (such as Makefiles or Ninja files) from a unified configuration.
It simplifies the compilation process and supports out-of-source builds,
making it easier to manage dependencies and build environments.

Mobius Forensic Toolkit has been designed to be built and installed
using CMake, so you will need to have CMake installed on your system
before proceeding. Additionally, ensure you have a compatible compiler
(such as GCC, Clang, or MSVC) and the necessary development tools for
your platform. If you do not already have CMake installed, you can
typically install it using your system's package manager (e.g., apt,
brew, or choco).

Once CMake and the required tools are set up, follow the steps below to
build and install Mobius Forensic Toolkit:

-   Download the source code and prepare the build directory.
-   Configure the project using CMake (with optional flags for
    customization).
-   Compile and install the toolkit on your system.

We will guide you through each of these steps in detail.

Let's get started!

1.  To begin, use a package manager available on your Linux distribution
    to install both the required packages and optional packages listed
    below:

    ### Required packages

    -   `g++` (or any C++17 compatible compiler)
    -   `Python 3` version 3.6 or newer
    -   `python3-devel`
    -   `python3-cairo`
    -   `python3-gobject-devel`
    -   `python3-gobject-gdk`
    -   `python3-libxml2-python`
    -   `gtk3-devel`
    -   `libgtksourceview-4-0`
    -   `libsqlite3`
    -   `libtsk` (from Sleuthkit package. See
        [https://www.sleuthkit.org)](https://www.sleuthkit.org/sleuthkit)
    -   `libdl` (already installed in most Linux distributions)

    ### Optional packages

    -   `libz` (already installed in most Linux distributions)
    -   `libudev` (already installed in most Linux distributions)
    -   `libsmbclient` (from Samba/Samba client package)

2.  Download the latest version of Mobius Forensic Toolkit using one of
    the links available in this page.This is typically provided as a tar
    archive, which can be extracted using the `tar` command. For
    example:

        tar xvf mobiusft-2.15.tar.xz

3.  Run the configuration command to prepare the build process. This
    will check for dependencies and configure the build to match your
    system. The `configuration` command is usually invoked using the
    following command:

        cmake -B build -S mobiusft-2.15 -DCMAKE_BUILD_TYPE=Release

    You can install Mobius Forensic Toolkit to a specific directory,
    using the `--install-prefix` option. For example:

        cmake -B build -S mobiusft-2.15 -DCMAKE_BUILD_TYPE=Release --install_prefix=/opt/mobiusft-2.15

    If you have installed the Sleuthkit package in a specific directory,
    use the `-DLIBTSK_ROOT=` option. For example:

        cmake -B build -S mobiusft-2.15 -DCMAKE_BUILD_TYPE=Release -DLIBTSK_ROOT=/opt/sleuthkit-4.14.0

    The CMake program will run, trying to identify your system
    configuration and to find both the required packages and the
    optional packages.

    It is highly recommended that you install all the required packages
    and all the optional packages, for a better user experience.

4.  Run the `cmake` command to build Mobius Forensic Toolkit. This will
    compile the source code and create the executable file:

        cmake --build build

5.  If the build was successful, you can install Mobius Forensic Toolkit
    using the `cmake --install` command. This will typically install the
    tool to a system directory such as /usr/local/bin:

        sudo cmake --install build

## 2. Installing MobiusFT using `.AppImage` file

In this section, we will walk you through the process of installing
Mobius Forensic Toolkit on your system using an AppImage file.

An AppImage is a portable application format that allows you to run
applications on a variety of Linux distributions without the need to
install them. It includes all of the necessary dependencies and
libraries, so you can simply download and run the AppImage file to use
the application.

To install and run our application using an AppImage file, you will need
to have the AppImage runtime installed on your system. This is typically
included by default on most modern Linux distributions, but if it is not
installed you can typically install it using your system's package
manager.

Once you have the AppImage runtime set up, you can follow the steps in
this guide to download and install Mobius Forensic Toolkit using the
AppImage file. We'll start by downloading the AppImage file and making
it executable, then we'll show you how to run the application and (if
desired) make it available system-wide.

Let's get started!

1.  If your Linux distribution does not already include Python 3, you
    will need to install it before proceeding.

2.  Download the latest AppImage file for the Mobius Forensic Toolkit,
    using the link available in this page. This is typically a single
    executable file with a .appimage extension.

3.  Make the AppImage file executable. You can do this using the `chmod`
    command, like this:

        chmod +x mobiusft-2.15-x86_64.AppImage

4.  If you want to make the application available system-wide, copy the
    .AppImage to your favorite `bin` directory, such as the
    `/usr/local/bin` directory. This will allow you to run the
    application from any directory by simply typing its name on the
    command line.
