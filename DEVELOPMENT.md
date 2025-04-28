# Mobius Forensic Toolkit development guide

## 1. Project layers

```
                 +--------------------+
                 | Python extensions  |
+----------------+--------------------+
| C++ extensions |  libmobius_python  |
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
├── CMakeLists.txt                # Future top-level CMake file
├── configure.ac                  # Current Autotools config
├── Makefile.am                   # Top-level: SUBDIRS = src data
├── README.md                     # Project overview
├── DEVELOPMENT.md                # This file
├── LICENSE
├── include/
│   ├── mobius/
│   │   ├── core/
│   │   │   └── *.hpp
│   │   ├── framework/
│   │   │   └── *.hpp
├── src/
│   ├── CMakeLists.txt           # CMake for src/
│   ├── Makefile.am              # SUBDIRS = libmobius_core ...
│   ├── libmobius_core/
│   │   ├── CMakeLists.txt
│   │   ├── Makefile.am
│   │   └── *.cpp
│   ├── libmobius_framework/
│   │   ├── CMakeLists.txt
│   │   ├── Makefile.am
│   │   └── *.cpp
│   ├── libmobius_python/
│   │   ├── core
│   │   │   └── CMakeLists.txt
│   │   ├── framework
│   │   │   └── CMakeLists.txt
│   │   ├── CMakeLists.txt
│   │   ├── Makefile.am
│   │   └── *.cpp
│   ├── extensions/
│   │   ├── CMakeLists.txt
│   │   ├── Makefile.am
│   │   ├── app/ares/
│   │   ├── app/emule/
│   │   ├── ...
│   │   ├── vfs/
│   │   └── ...
│   ├── python/
│   │   ├── CMakeLists.txt
│   │   ├── pymobius/
│   │   └── scripts/
│   ├── tools/                   # C++ tools/examples
│   │   ├── CMakeLists.txt       # CMake for building tools
│   │   ├── Makefile.am          # Autotools for current hybrid build
│   │   ├── mobius-extract/      # Example tool 1
│   │   │   ├── CMakeLists.txt
│   │   │   └── *.cpp
│   │   ├── mobius-analyze/      # Example tool 2
│   │   │   ├── CMakeLists.txt
│   │   │   └── *.cpp
│   │   └── ...                  # Other tools
├── data/
│   └── Makefile.am
├── mobius_bin.py
└── mobius_bin.sh
```

