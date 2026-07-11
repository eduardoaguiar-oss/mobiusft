#!/usr/bin/env python3

import pymobius.evidence
import os
import re

# Generate dict from templates
dictionary = []

# I18N masks in templates
for filename in os.listdir('../template'):
    if filename.endswith('.html'):
        src_path = os.path.join('../template', filename)

        with open(src_path, 'r', encoding='utf-8') as src_file:
            content = src_file.read()

            for match in re.finditer(r'I18N\{([^:}]+):([^}]+)\}', content):
                key = match.group(1)
                text = match.group(2)
                dictionary.append((key, text))

# Evidence type names
for et in pymobius.evidence.MODEL:
    key = f'type.{et["id"]}'
    text = et['name']
    dictionary.append((key, text))

# List evidences
for key, text in sorted(set(dictionary)):
    print(f'{key}\t{text}')

