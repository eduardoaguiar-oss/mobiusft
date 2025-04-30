# Mobius Forensic Toolkit development guide

## 1. Project layers

```
+----------------+--------------------+
| C++ extensions | Python extensions  |
|                +--------------------+
|                |  libmobius_python  |
|       +---------------------+       |
|       | libmobius_framework |       |
+-------------------------------------+
|          libmobius_core             |
+-------------------------------------+
```

## 2. Project main directories

These are the project main directories and main files:

```text
mobius/
├── CMakeLists.txt		# Top-level CMake file
├── configure.ac		# @deprecated Current Autotools config
├── Makefile.am			# @deprecated Top-level Makefile.am
├── README.md			# Project overview
├── DEVELOPMENT.md		# This file
├── LICENSE
├── include/			# mobius_core and mobius_framework include files
│   ├── mobius/
│   │   ├── core/
│   │   │   └── *.hpp
│   │   ├── framework/
│   │   │   └── *.hpp
├── src/
│   ├── CMakeLists.txt
│   ├── libmobius_core/		# Forensic library
│   │   ├── CMakeLists.txt
│   │   ├── crypt/
│   │   ├── database/
│   │   ├── ...
│   │   ├── vfs/
│   │   ├── ...
│   │   └── *.cpp
│   ├── libmobius_framework/	# MobiusFT framework library
│   │   ├── CMakeLists.txt
│   │   ├── ...
│   │   ├── model/
│   │   ├── ...
│   │   └── *.cpp
│   ├── libmobius_python/	# Python 'mobius' module (import mobius)
│   │   ├── core
│   │   │   └── CMakeLists.txt
│   │   ├── framework
│   │   │   └── CMakeLists.txt
│   │   ├── CMakeLists.txt
│   │   └── *.cpp
│   ├── extensions/		# C++ and Python extensions
│   │   ├── CMakeLists.txt
│   │   ├── app/ares/
│   │   ├── app/emule/
│   │   ├── ...
│   │   ├── vfs/
│   │   └── ...
│   ├── pymobius/		# Python 'pymobius' module (import pymobius)
│   │   ├── CMakeLists.txt
│   │   ├── ant/
│   │   ├── app/
│   │   ├── ...
│   ├── tools/                   # C++ tools/examples
│   │   ├── CMakeLists.txt
│   │   └── *.cpp
├── data/
│   └── Makefile.am
├── mobius_bin.py
└── mobius_bin.sh
```
