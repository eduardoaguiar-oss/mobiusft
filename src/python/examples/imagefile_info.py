#!/usr/bin/env python3

import mobius
import sys


def print_imagefile_info(imagefile):
    print(f'Type: {imagefile.type}')
    print(f'Size: {imagefile.size} bytes')
    print(f'Sectors: {imagefile.sectors}')
    print(f'Sector size: {imagefile.sector_size} bytes')

    for name, value in sorted(imagefile.get_attributes().items()):
        print(f"{name.replace('_', ' ').capitalize()}: {value}")


app = mobius.core.application()
app.start()

for uri in sys.argv[1:]:
    print()
    print(f'>> {uri}')

    imagefile = mobius.vfs.new_imagefile_by_url(uri)

    if imagefile.is_available():
        print_imagefile_info(imagefile)

app.stop()
