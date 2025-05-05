# Mobius Forensic Toolkit development guide

## 1. **Project layers**

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

## 2. **Project main directories**

These are the project main directories and main files:

```text
mobius/
├── CMakeLists.txt		# Top-level CMake file
├── README.md			# Project overview
├── DEVELOPMENT.md		# This file
├── LICENSE
├── include/			# mobius_core and mobius_framework include files
│   ├── CMakeLists.txt
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
│   │   ├── CMakeLists.txt
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
│   └── CMakeLists.txt
├── mobius_bin.py
└── mobius_bin.sh
```

## 3. **Getting Started with Development**

The Mobius Forensic Toolkit provides multiple ways to extend its functionality through different programming approaches:  

### 3.1. **Standalone C++ Programs**  
You can develop independent C++ applications using only the core library (`libmobius_core`). For functional examples, refer to the sample programs in the [`src/tools`](src/tools) directory.  

### 3.2. **Standalone Python Programs**  
If you prefer Python, you can import the `mobius` module directly into your scripts to leverage the toolkit’s capabilities.  

### 3.3. **Integrated Extensions (C++ & Python)**  
For more advanced use cases, explore the [`src/extensions`](src/extensions) directory, which contains fully functional examples demonstrating both:  
- **C++ API** (`libmobius_core` + `libmobius_framework`)  
- **Python API** (`mobius` module)  

## 4. **Creating New Extensions**  
You can develop custom extensions by following the examples in [`src/extensions`](src/extensions). These extensions enhance the Mobius Forensic Toolkit’s functionality and generally fall into one of these categories:  

### 4.1. **Application Support (`app/xxxx`)**  
- Extends support for specific applications by implementing an `evidence_loader_impl` subclass.  
- Enables searching, retrieving, and storing evidence from **VFS (Virtual File System) data sources**.  

### 4.2. **Disk Data Block Handling (`vfs/block`)**  
- Adds detection, parsing, and management of disk structures, including:  
  - **Encrypted volumes** (e.g., BitLocker)  
  - **Partition systems**  
  - **Filesystem metadata**  

### 4.3. **Filesystem Support (`vfs/filesystem`)**  
- Implements support for new filesystems. The toolkit already includes:  
  - **Windows**: NTFS, VFAT  
  - **Linux**: Ext2, Ext3, Ext4
  - **macOS**: HFS/HFS+  
  - **Other**: ExFAT, ISO9660  

### 4.4. **Disk Image Support (`vfs/imagefile`)**  
- Adds compatibility with forensic image formats. Currently supported formats include:  
  - **Forensic images**: EWF (EnCase), RAW (dd), ICS Solo  
  - **Hardware-encrypted images**: Seagate MSR (no decryption keys required)  
  - **Virtual disks**: VHD, VHDX  
  - **Proprietary formats**: Logicube Dossier/Talon  

### **Next Steps**  
- **For C++ development**, study the examples in [`src/tools`](src/tools) and [`src/extensions`](src/extensions).  
- **For Python scripting**, import the `mobius` module and refer to the built-in documentation.  
- **To contribute a new extension**, follow the existing implementations in [`src/extensions`](src/extensions) and submit a pull request.  

## 5. **Mobius Forensic Toolkit Development Cycle**

```text
    Version N
+----------------+
| Implementation |
|   (30 days)    |
|                |   Version N+1
+----------------+----------------+
|    Testing     | Implementation |
|   (20 days)    |    (30 days)   |
+----------------+                |
    Release Vn   +----------------+
                 |    Testing     |
                 |    (20 days)   |
                 +----------------+
                     Release Vn+1
```
