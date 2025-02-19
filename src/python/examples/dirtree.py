#!/usr/bin/env python3

import mobius
import sys


def show_entry(r, path='/', level=0):
    indent = ' ' * (level * 2)

    if not r.exists():
        print(f'{indent}? {path}')
        return

    # show metadata
    print()
    print(f'{indent}{"+" if r.is_folder() else "."} {path}')
    print(f'{indent}  User: {r.user_name} ({r.user_id:d})')
    print(f'{indent}  Group: {r.group_name} ({r.group_id:d})')
    print(f'{indent}  Permissions: {r.permissions:o}')
    print(f'{indent}  Last access time (atime): {r.access_time}')
    print(f'{indent}  Last modification time (mtime): {r.modification_time}')
    print(f'{indent}  Last metadata time (ctime): {r.metadata_time}')

    # if resource is a file, show some bytes...
    if r.is_file():
        print(f'{indent}  size: {r.size:d}')

        if r.is_regular_file():
            try:
                reader = r.new_reader()
                data = reader.read(16)
                print(f'{indent}  first 16 bytes: {mobius.encoder.hexstring(data, " ")}')
            except Exception as e:
                print(f'{indent}  <Warning: {e}>')

    # otherwise resource is a folder, so recurse into it
    else:
        try:
            for child in r.get_children():
                if path == '/':
                    cpath = path + child.name
                else:
                    cpath = path + '/' + child.name
                show_entry(child, cpath, level + 1)
        except Exception as e:
            print(f'{indent}  <Warning: {e}>')


for path in sys.argv[1:]:
    entry = mobius.io.new_entry_by_path(path)
    show_entry(entry)
