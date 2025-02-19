#!/usr/bin/env python3

import mobius
import sys


def get_hash(reader):
    h = mobius.crypt.hash('sha2-512')
    data = reader.read(65536)
    while data:
        h.update(data)
        data = reader.read(65536)

    return h.get_hex_digest()


def print_kff_entry(entry):
    try:
        if entry.is_folder():
            print_kff_folder(entry)

        elif not entry.is_deleted():
            reader = entry.new_reader()
            if reader:
                h = get_hash(reader)
                print(f'{h}\t{entry.path}')
    except Exception as e:
        print(f"Warning: {e}")


def print_kff_folder(folder):
    try:
        if folder.is_reallocated():
            return

        for child in folder.get_children():
            print_kff_entry(child)

    except Exception as e:
        print(f"Warning: {e}")


def print_kff(vfs):
    for entry in vfs.get_root_entries():
        print_kff_entry(entry)


app = mobius.core.application()
app.start()

for url in sys.argv[1:]:
    vfs = mobius.vfs.vfs()
    vfs.add_disk(mobius.vfs.new_disk_by_url(url))

    if vfs.is_available():
        print_kff(vfs)

app.stop()
