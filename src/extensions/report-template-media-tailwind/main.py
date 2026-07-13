# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008-2026 Eduardo Aguiar
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
import json
import os.path
import re
import mobius
import pymobius
import shutil

from metadata import *

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Format ID
# @param text Text to format
# @return Formatted text
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def format_id(text):
    if not text:
        return text

    text = text.replace('_', ' ')
    text = text.replace('-', ' ')
    new_text = ''
    prev_char = ''

    for char in text:
        if char.isupper() and prev_char.islower():
            new_text += ' '

        new_text += char
        prev_char = char

    new_text = new_text.strip()

    if new_text and new_text[0].islower():
        new_text = new_text[0].upper() + new_text[1:]

    return new_text

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Media Tailwind CSS report generator
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Generator(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize generator
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.templates = [
            {
                'id' : 'media.tailwind.en_US',
                'type' : 'media',
                'description' : 'Media: Tailwind CSS (en_US)'
            },
            {
                'id' : 'media.tailwind.pt_BR',
                'type' : 'media',
                'description' : 'Media: Tailwind CSS (pt_BR)'
            },
        ]

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run report generator
    # @param model Model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self, model):

        # Set up control variables
        self.__output_dir = model['output_dir']
        self.__items = model['items']
        self.__template_id = model['template_id']
        self.__evidence_types = model['evidence_types']
        self.__language = self.__template_id.split('.')[-1]
        self.__i18n_dict = {}

        # Load I18N dictionary for the current language, if available
        lang_path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'lang', f'{self.__language}.txt')
        f = mobius.core.io.new_file_by_path(lang_path)

        if f.exists():
            reader = mobius.core.io.line_reader(f.new_reader())
            for line in reader:
                line = line.strip()
                if line and not line.startswith('#'):
                    key, value = line.split('\t', 1)
                    self.__i18n_dict[key.strip()] = value.strip()

        # Create output folder, if necessary
        folder = mobius.core.io.new_folder_by_path(self.__output_dir)
        if not folder.exists():
            folder.create()

        # Generate static files, templates, evidence icons, model.js, and evidence.js
        self.__generate_static_files()
        self.__generate_templates()
        self.__generate_evidence_icons()
        self.__generate_model_js()
        self.__generate_evidence_js()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate static files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_static_files(self):
        common_path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'common')
        shutil.copytree(common_path, self.__output_dir, dirs_exist_ok=True)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate static content from templates
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_templates(self):
        template_path = pymobius.mediator.call('extension.get-resource-path', EXTENSION_ID, 'template')

        for filename in os.listdir(template_path):
            src_path = os.path.join(template_path, filename)

            if os.path.isfile(src_path):
                dst_path = os.path.join(self.__output_dir, filename)
                self.__generate_template_from_file(src_path, dst_path)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate static content from a template file
    # @param src_path Source template file path
    # @param dst_path Destination file path
    # It replaces all instances of I18N{key:text} mask with the corresponding
    # text from the I18N dictionary for the current language.
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_template_from_file(self, src_path, dst_path):

        with open(src_path, 'r', encoding='utf-8') as src_file:
            content = src_file.read()
            content = re.sub(r'I18N\{([^:}]+):([^}]+)\}', lambda m: self.__i18n_dict.get(m.group(1), m.group(2)), content)

        with open(dst_path, 'w', encoding='utf-8') as dst_file:
            dst_file.write(content)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate model.js file
    # @param model Model
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_model_js(self):

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Create data folder, if necessary
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        data_path = os.path.join(self.__output_dir, 'data')

        f = mobius.core.io.new_folder_by_path(data_path)
        if not f.exists():
            f.create()

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Create data/model.js file
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        items_js_path = os.path.join(data_path, 'model.js')

        f = mobius.core.io.new_file_by_path(items_js_path)
        fp = mobius.core.io.text_writer(f.new_writer())

        fp.write('// Generated by Mobius Forensic Toolkit\n')
        fp.write('// Warning: This file is automatically generated. Do not edit manually.\n')
        fp.write('\n')

        app = mobius.core.application()
        fp.write(f'window.VERSION = "{app.version}";\n')

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Generate EVIDENCE_TYPES variable
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        types_data = []

        for m in self.__evidence_types:
            m_id = m['id']

            type_data = {
                'id': m_id,
                'name': self.__i18n_dict.get(f'type.{m_id}', m['name']),
                'icon': m['icon'],
            }
            
            for mv in m.get('master_views', []):
                if mv['id'] == 'table':
                    columns = []

                    for c in mv['columns']:
                        c_id = c['id']
                        c_name = self.__i18n_dict.get(f'column.{m_id}.{c_id}', None) or \
                                 self.__i18n_dict.get(f'column.{c_id}', None) or \
                                 c.get('name', None) or \
                                 format_id(c_id)
                        columns.append({
                            'id': c_id,
                            'name': c_name,
                            'format': c.get('format', None),
                            'is_sortable': c.get('is_sortable', False),
                            'first_sortable': c.get('first_sortable', False),
                        })

                    type_data['columns'] = columns

            types_data.append(type_data)

        types_data = sorted(types_data, key=lambda x: x['name'].lower())

        fp.write('\n')
        fp.write('const EVIDENCE_TYPES = ')
        json.dump(types_data, fp, ensure_ascii=False, separators=(',', ':'))
        fp.write(';\n')

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Generate ITEMS variable
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        fp.write('\n')
        fp.write('const ITEMS = ')

        items_data = []
        
        for item in self.__items:
            item_data = self.__generate_item(item)
            items_data.append(item_data)

        json.dump(items_data, fp, ensure_ascii=False, separators=(',', ':'))
        fp.write(';\n')

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Generate window variables
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        fp.write('\n')
        fp.write('window.EVIDENCE_TYPES = EVIDENCE_TYPES;\n')
        fp.write('window.ITEMS = ITEMS;\n')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate model.js item
    # @param item Item
    # @param fp File pointer
    # @param parent Parent item
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_item(self, item, parent=None):
        data = {'uid': item.uid, 'name': item.name, 'category': item.category}

        # Metadata
        metadata = []
        category = mobius.framework.get_category(item.category)

        for attr in category.get_attributes():
            if attr.name and attr.name[0].islower():
                name = attr.name[0].upper() + attr.name[1:]
            else:
                name = attr.name

            if attr.id == 'uid':
                value = item.uid

            elif attr.id == 'category':
                value = item.category

            else:
                value = item.get_attribute(attr.id)

            if value is None:
                value = ''

            metadata.append({'id': attr.id, 'name': name, 'value': value})

        data['metadata'] = metadata

        # Parent item
        if parent:
            data['parent'] = parent.uid

        # Children items
        data['children'] = [child.uid for child in item.get_children()]

        # Datasource data
        datasource = item.get_datasource()

        if datasource:
            if datasource.get_type() == 'vfs':
                data['vfs'] = self.__generate_vfs(datasource)

            elif datasource.get_type() == 'ufdr':
                data['ufdr'] = self.__generate_ufdr(datasource)

        # Evidence counts
        data['evidence_counts'] = item.count_evidences_grouped()

        # Generate children items
        children = []

        for child in item.get_children():
            child_data = self.__generate_item(child, item)
            children.append(child_data)

        data['children'] = children

        # Return data
        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate VFS data
    # @param datasource Datasource object
    # @param fp Writer object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_vfs(self, datasource):
        data = {}
        blocks = []

        vfs = datasource.get_vfs()

        for block in vfs.get_blocks():
            block_data = {'id': block.uid, 'type': block.type, 'size': block.size, 'description': block.get_attribute('description', '')}
            block_data['parents'] = [parent.uid for parent in block.get_parents()]

            metadata = []

            for name, value in block.get_attributes().get_values():
                name = format_id(name)
                value = str(value) if value is not None else ''
                metadata.append({'name': name, 'value': value})

            block_data['metadata'] = metadata
            blocks.append(block_data)

        data['blocks'] = blocks
        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate UFDR data
    # @param datasource Datasource object
    # @param fp Writer object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_ufdr(self, datasource):
        data = []

        # Case info
        case_info = datasource.get_case_info()
        case_data = {'type': 'case_info', 'description': 'Case information', 'metadata': []}
    
        for name, value in case_info.get_values():
            name = format_id(name)
            case_data['metadata'].append({'name': name, 'value': value})
    
        data.append(case_data)
    
        # Extractions
        for e in datasource.get_extractions():
            extraction_data = {'type': 'extraction', 'name': e.name, 'metadata': []}
            extraction_data['metadata'].append({'name': 'ID', 'value': e.id})
            extraction_data['metadata'].append({'name': 'Type', 'value': e.type})
            extraction_data['metadata'].append({'name': 'Name', 'value': e.name})
            extraction_data['metadata'].append({'name': 'Device Name', 'value': e.device_name})

            for name, value in e.get_metadata():
                extraction_data['metadata'].append({'name': format_id(name), 'value': value})

            data.append(extraction_data)

        # Return data
        return data

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate evidence icons
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_evidence_icons(self):

        # Create img/evidences folder, if necessary
        evidences_path = os.path.join(self.__output_dir, 'img', 'evidences')

        f = mobius.core.io.new_folder_by_path(evidences_path)
        if not f.exists():
            f.create()

        # Copy icon files
        for et in self.__evidence_types:
            icon_path = et['icon']
            shutil.copy(icon_path, evidences_path)
            et['icon'] = os.path.join('img', 'evidences', f"{et['id']}.png")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate data/<uid>/<evidence-type>.js files
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_evidence_js(self):

        # Build getters for each attribute and evidence type
        self.__getters = {}

        for m in pymobius.evidence.MODEL:
            evidence_type = m['id']

            for v in m.get('master_views', []):
                if v['id'] == 'table':
                    attributes = []
                    for c in v['columns']:
                        attr_id = c['id']
                        attr_format = c.get('format', None)
                        attr_getter = pymobius.evidence.Getter(attr_id, attr_format)
                        attributes.append((attr_id, attr_getter))
                    self.__getters[evidence_type] = attributes

        # Generate data/<uid>/<evidence-type>.js files for each item and evidence type
        for item in self.__items:
            for evidence_type in item.count_evidences_grouped().keys():
                self.__generate_item_evidence_js(item, evidence_type)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate data/<uid>/<evidence-type>.js files for an item
    # @param item Item
    # @param evidence_type Evidence type
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __generate_item_evidence_js(self, item, evidence_type):

        # Create data/<uid> folder, if necessary
        base_dir = os.path.join(self.__output_dir, 'data', f'{item.uid:04d}')
        folder = mobius.core.io.new_folder_by_path(base_dir)
        if not folder.exists():
            folder.create()

        # Create data/<uid>/<evidence-type>.js file
        path = os.path.join(base_dir, f'{evidence_type}.js')
        f = mobius.core.io.new_file_by_path(path)

        fp = mobius.core.io.text_writer(f.new_writer())
        fp.write('// Generated by Mobius Forensic Toolkit\n')
        fp.write('// Warning: This file is automatically generated. Do not edit manually.\n')
        fp.write('\n')

        app = mobius.core.application()
        fp.write('const EVIDENCES = ')

        # Get evidences
        evidences = []

        for e in item.get_evidences(evidence_type):
            getters = self.__getters.get(evidence_type, [])
            data = {'uid': e.uid,
                    'attrs': dict((k, g(e)) for k, g in getters),
                    'tags': list(e.get_tags()),
                    'hashes': e.get_hashes()
            }

            # Metadata
            if e.has_attribute('metadata'):
                metadata = [{'name': name, 'value': str(value)} for name, value in e.get_attribute('metadata').to_python().items()]
            else:
                metadata = []

            data['metadata'] = metadata
            evidences.append(data)

        json.dump(evidences, fp, ensure_ascii=False, separators=(',', ':'))
        fp.write(';\n\n')
        fp.write('window.EVIDENCES = EVIDENCES;\n')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Start function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start():
    mobius.core.add_resource('report-template.media-tailwind', 'Media Tailwind Report Template', Generator)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Stop function
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_stop():
    mobius.core.remove_resource('report-template.media-tailwind')
