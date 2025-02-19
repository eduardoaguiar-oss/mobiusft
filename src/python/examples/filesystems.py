#!/usr/bin/env python3

import mobius
import sys


def print_filesystems(vfs):
    for block in vfs.get_blocks():
        if block.type == 'filesystem':
            print()
            for key, value in block.get_attributes().get_values():
                print(f'   {key.capitalize()}: {value}')


app = mobius.core.application()
app.start()

for url in sys.argv[1:]:
    print()
    print(f">> {url}")

    vfs = mobius.vfs.vfs()
    vfs.add_disk(mobius.vfs.new_disk_by_url(url))

    if vfs.is_available():
        print_filesystems(vfs)

app.stop()
