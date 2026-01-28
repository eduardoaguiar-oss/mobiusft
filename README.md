---
title: Mobius Forensic Toolkit
description: "Open-source forensic framework and tools written in C++/Python"
version: "2.24"
---

# Mobius Forensic Toolkit

**Copyright (C) 2008–2026 Eduardo Aguiar**

---

## Introduction

The **Mobius Forensic Toolkit** is a versatile, open-source forensic framework designed to empower digital investigators with advanced tools for case management and analysis. Built using **C++** and **Python**, Mobius offers a flexible and extensible platform for handling forensic cases, analyzing evidence, and integrating with other forensic tools.

Whether you're a digital forensics expert or a beginner, the Mobius Forensic Toolkit provides the tools and flexibility you need to tackle complex forensic challenges. Download it today and experience the power of open-source forensic analysis.

## Key Features:
Mobius Forensic Toolkit is a powerful, open-source digital forensics framework developed in **C++20** and **Python 3**, offering both native C++ and Python APIs (with comprehensive Python wrappers) for maximum flexibility and extensibility.

- **Broad Data Source Support** — Natively handles a wide range of forensic inputs, including image files (RAW, split RAW, EWF, Talon, Solo, Dossier, MSR, VHD, VHDX), physical devices, and Cellebrite UFDR report files. Automatic, password-free decryption of MSR image files is provided for seamless access.

- **Advanced Windows Artifact Recovery** — Automatically decrypts and extracts Windows secrets, including LM/NT hashes, cached credentials, PSSP keys, LSA secrets, DPAPI blobs (v1 and v2), Windows credentials, and Wi-Fi passwords. Registry files are scanned automatically with full reconstruction of the logical Windows Registry structure.

- **Robust Case Management** — Provides comprehensive tools for creating, organizing, and managing forensic cases. All case items (evidence sources) can be consolidated and processed within a single case, with customizable **case profiles** that control processing scope (e.g., full scan vs. user folders only) and enable/disable specific processors.

- **Modular and Extensible Architecture** — Highly extensible framework allowing users to develop custom C++ or Python extensions and plugins. New evidence types and application parsers can be added easily, supported by modern processing classes like `vfs_processor_impl` for profile-aware evidence collection.

- **Efficient Data Storage and Compatibility** — Utilizes SQLite databases for scalable, reliable storage of forensic evidence and metadata, ensuring easy integration and export to other tools.

- **Comprehensive Application Artifact Parsing** — In-depth support for extracting digital evidence from popular applications, including:
  - **Browsers**: Chromium-based (Google Chrome, Microsoft Edge, Brave, Opera) with automatic decryption of cookies, logins, history, bookmarks, and autofill; Mozilla Firefox; Internet Explorer.
  - **Communication**: Skype (desktop versions 4–14, all SQLite formats including encrypted s4l-*.db).
  - **P2P File Sharing**: aMule/eMule (including Torrent controls), Ares Galaxy, Shareaza, µTorrent/BitTorrent (including µTorrent Web), DC++, DreaMule.
  - **Other**: iTubeGo and more.

- **Additional Capabilities** — Virtual File System (VFS) for advanced block detection and decoding; post-processor framework for generating secondary evidence; integration with external tools like IPED (with enhanced memory management and resume functionality); evidence viewer with processing status, KFF alerts, and specialized views (e.g., voicemails, remote party IPs, credit cards).

Mobius Forensic Toolkit combines performance, extensibility, and specialized depth—particularly in P2P and browser forensics—making it an essential tool for digital investigators, researchers, and cybersecurity professionals.

## Getting Started:
- [**Quick Start Guide**](https://www.nongnu.org/mobiusft/support/getting_started/index.html): Follow the step-by-step guide to set up and start using Mobius Forensic Toolkit in minutes.
- **Comprehensive Documentation**: Access detailed tutorials, API references, and case studies to maximize your efficiency.

## Getting project files
Download the latest version of the Mobius Forensic Toolkit from GitHub:  
[https://www.github.com/eduardoaguiar-oss/mobiusft/releases](https://www.github.com/eduardoaguiar-oss/mobiusft/releases).

You can download the latest development version from GitHub repository:  
[https://www.github.com/eduardoaguiar-oss/mobiusft](https://www.github.com/eduardoaguiar-oss/mobiusft).

## Installation

Run the following shell commands (replace all `<VAR>` for suitable values):

```bash
tar xvf <mobius-tar-file>
cmake -B build -S <mobiusft-dir> --install-prefix <DIR> [-DLIBTSK_ROOT=<DIR>]
cmake --build build -j <threads>
cmake --install build
```

Example:

```bash
tar xvf mobiusft-2.18.tar.xz
cmake -B build -S mobiusft-2.18 --install-prefix /opt/mobiusft-2.18 -DLIBTSK_ROOT=/opt/sleuthkit-4.14.0 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 24
sudo 'cmake --install build'
```

For detailed installation instructions, refer to the official [installation guide](https://github.com/eduardoaguiar-oss/mobiusft/wiki/Installation).

## Support this Project

Please, support this project. Here are the ways you can make a donation:

- Buy me a coffee at https://buymeacoffee.com/mobiusft.
- Send any amount of crypto coins to one of the following addresses:
   - Bitcoin (BTC): bc1qa92rvaru86hr5lup2a6ewqqgnran3p2qf3djah
   - Litecoin (LTC): LhiBHWgrqysZsoZfiuc3tUyzFRWwmDZipo
   - Monero (XMR): 46J7sEedDzqDvbrqzpb8suJgEbiK6RMtKHU7THuuN7FAciSN84npSqdFL5JSRFmsqrDaP9tZmYaTTBXEKU37XvLFGH2mkyi

## Acknowledgments

We extend our gratitude to the following organizations and individuals for their contributions:

- **Open Clip Art** ([https://www.openclipart.org](https://www.openclipart.org)) for providing amazing icons.
- **Everaldo's Crystal Interaction Design Project** ([https://everaldo.com/crystal](https://everaldo.com/crystal)) for their beautiful icon designs.
- **Wikimedia Commons** ([https://commons.wikimedia.org](https://commons.wikimedia.org)) for their extensive collection of resources.
- **GNU Savannah** ([https://savannah.nongnu.org](https://savannah.nongnu.org)) for hosting this project.
- **GitHub** ([https://www.github.com](https://www.github.com)) for hosting this project.
- **Beatae Mariae Virgini.**

Thank you for using the **Mobius Forensic Toolkit**. Your support and feedback are invaluable in helping us improve and grow this project.
