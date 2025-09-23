---
title: Mobius Forensic Toolkit
description: "Open-source forensic framework and tools written in C++/Python"
version: 2.20
---

# Mobius Forensic Toolkit

**Copyright (C) 2008–2025 Eduardo Aguiar**

---

## Introduction

The **Mobius Forensic Toolkit** is a versatile, open-source forensic framework designed to empower digital investigators with advanced tools for case management and analysis. Built using **C++** and **Python**, Mobius offers a flexible and extensible platform for handling forensic cases, analyzing evidence, and integrating with other forensic tools.

Whether you're a digital forensics expert or a beginner, the Mobius Forensic Toolkit provides the tools and flexibility you need to tackle complex forensic challenges. Download it today and experience the power of open-source forensic analysis.

#### Key Features:

- **Comprehensive Case Management**: Organize and manage forensic cases and case items efficiently.
- **Flexible Input Sources**: Supports **image files** (RAW, EWF, VHD, VHDX, and more), **physical devices**, and **Cellebrite's UFDR report files** for forensic analysis.
- **Extensible Framework**: Easily create custom extensions and plugins to tailor the toolkit to your specific needs and unlock support for new evidence types.
- **SQLite Database Integration**: Ensures reliable and scalable data storage for forensic evidence and metadata.
- **Cross-Platform Compatibility**: Works seamlessly across multiple operating systems, making it accessible to a wide range of users.
- **Support for many applications**: Extracts evidence from Google Chrome, Mozilla Firefox, Ares Galaxy, Shareaza, uTorrent, eMule, iTubeGo, and more!

#### Why Choose Mobius Forensic Toolkit?

- **Open Source**: Freely available under the GNU General Public License, ensuring transparency and community-driven development.
- **User-Friendly Interface**: Designed with both beginners and advanced users in mind, offering intuitive workflows and documentation.
- **Community Support**: Join the active community of forensic professionals and developers to share insights, ask questions, and contribute to the toolkit's growth.

#### Getting Started:
- **Quick Start Guide**: Follow the step-by-step guide to set up and start using Mobius Forensic Toolkit in minutes.
- **Comprehensive Documentation**: Access detailed tutorials, API references, and case studies to maximize your efficiency.
- **Mailing List**: Stay updated with the latest developments and connect with other users through the official Mobius mailing list.

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

For detailed installation instructions, refer to the official [installation guide](docs/INSTALL.md).

## Usage

To get started with the Mobius Forensic Toolkit, check out the **Quick Start Guide**:  
[https://www.nongnu.org/mobiusft/support/getting_started/index.html](https://www.nongnu.org/mobiusft/support/getting_started/index.html).

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
