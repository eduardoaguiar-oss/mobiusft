# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
import datetime
import traceback
import xml.dom.pulldom
import zipfile

import mobius
import pymobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# report.xml node structure
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# <project>
#       <caseInformation>
#           <field>...</field>
#       </caseInformation>
#
#       <sourceExtractions>
#           <extractionInfo>
#       </sourceExtractions>
#
#       <taggedFiles>
#           <file>
#           ...
#           </file>
#           ...
#       </taggedFiles>
#
#       <decodedData>
#           <modelType type='xxx'>
#               <model deleted_state,decoding_confidence,id,labels,extractionId,iscarved,type,isrelated,source_index>
#               ...
#               </model>
#               ...
#           </modelType>
#       </decodedData>
# </project>


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# report.xml tags and attributes:
#
# <project name,id,NodeCount,licenseID,ModelCount,reportVersion,xmlns,containsGarbage,extractionType>
#   <HashSetsInfo>
#   <MalwareScanner ScanPerformed>
#   <carvedFiles>
#   <caseInformation>
#      <field name,isSystem,isRequired,fieldType,multipleLines>
#   <decodedData>
#      <modelType type>
#        <model deleted_state,decoding_confidence,id,labels,extractionId,iscarved,type,isrelated,source_index>
#          <field type,name>
#             <empty>
#             <value type,formattedTimestamp,format>
#          <multiField type,name>
#             <value type>
#          <modelField type,name>
#             <empty>
#             <model>
#          <multiModelField type,name>
#             <model>
#          <jumptargets name>
#             <targetid ismodel>
#          <nodeField name>
#             <id name>
#          <RelatedModels>
#   <enrichments>
#   <extraInfos>
#     <extraInfo type,id>
#       <sourceInfo>
#          <imageInfo offset,name>
#          <nodeInfo size,name,id,offset,path,tableName>
#          <nodeInfos>
#             <nodeInfo size,name,id,offset,path,tableName>
#   <images>
#     <image key,size,extractionId,path,type,verify>
#       <metadata section>
#         <item name,systemtype>
#   <infectedFiles>
#   <metadata section>
#      <item group,name,id,sourceExtraction,systemtype>
#   <projectConfigurations>
#     <timeZoneConfigurations enableDaylightSavings,selectedTimeZone,enableAutomaticallyConvertToUTC>
#   <sourceExtractions>
#     <extractionInfo isCustomName,IsPartialData,IsSelectiveExtraction,IsStoppedByUser,name,id,deviceName,fullName,
#                     type,index,IsTriageExtraction>
#   <taggedFiles>
#     <file decodedApplication,size,fsid,id,fs,embedded,labels,extractionId,path,isrelated,decodedBy,source_index,
#           deleted>
#        <accessInfo>
#           <timestamp format,formattedTimestamp,name>
#        <jumptargets name>
#           <targetid ismodel>
#        <metadata section>
#           <item group,name,systemtype>
#        <sourcemodels>
#           <ownerid type,direction>
#       <RelatedNodes>
#          <file>
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Get node datetime
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_datetime_from_node(value):
    value = (value or '').strip()

    if value in ('N/A', ''):
        value = None

    elif len(value) >= 19:
        date_part = value[:19]
        tz_part = value[23:]    # skip milliseconds

        try:
            value = datetime.datetime.strptime(date_part, '%Y-%m-%dT%H:%M:%S')

            if len(tz_part) >= 6 and tz_part != '+00:00':
                seconds = int(tz_part[1:3]) * 3600 + int(tz_part[4:6]) * 60
                if tz_part[0] == '+':
                    seconds = -seconds
                value += datetime.timedelta(seconds=seconds)

        except ValueError as e:
            mobius.core.logf(f"WRN {str(e)}\nValue:<{value}>\n{traceback.format_exc()}")

    return value


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Sink in function, to be the default handler
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def default_handler(*args):
    return False


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief UFDR file parser class
# @author Eduardo Aguiar
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class UFDRParser(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, path, control):
        self.__path = path
        self.__case_info = mobius.core.pod.map()
        self.__extractions = []
        self.__evidence_idx = 0
        self.__evidence_count = 0
        self.__unknown_datatypes = set()
        self.__on_tagged_file_handler = getattr(control, 'on_tagged_file', None)
        self.__on_evidence_handler = getattr(control, 'on_evidence', None)
        self.__on_document_end_handler = getattr(control, 'on_document_end', default_handler)
        self.__on_element_start_handler = getattr(control, 'on_element_start', default_handler)
        self.__on_element_end_handler = getattr(control, 'on_element_end', default_handler)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get case info
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_case_info(self):
        return self.__case_info

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get extractions info
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_extractions(self):
        return self.__extractions

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get unknown datatypes
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_unknown_datatypes(self):
        return self.__unknown_datatypes

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Run agent
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        zfile = zipfile.ZipFile(self.__path)
        fp = zfile.open("report.xml", force_zip64=True)
        doc = xml.dom.pulldom.parse(fp)
        last_fullname = ''

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Process pulldom events
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for event, node in doc:

            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            # start element
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            if event == xml.dom.pulldom.START_ELEMENT:

                # update fullname
                if last_fullname:
                    fullname = f"{last_fullname}.{node.tagName}"
                else:
                    fullname = node.tagName

                # handle relevant nodes
                if fullname == 'project.caseInformation':
                    self.__on_case_information(doc, node)

                elif fullname == 'project.sourceExtractions':
                    self.__on_source_extractions(doc, node)

                elif fullname == 'project.metadata' and node.getAttribute('section') == 'Extraction Data':
                    self.__on_extraction_data(doc, node)

                elif fullname == 'project.metadata' and node.getAttribute('section') == 'Device Info':
                    self.__on_device_info(doc, node)

                elif self.__on_tagged_file_handler and fullname == 'project.taggedFiles.file':
                    if self.__on_tagged_file(doc, node):
                        break

                elif self.__on_evidence_handler and fullname == 'project.decodedData.modelType.model':
                    self.__on_evidence(doc, node)

                else:
                    if fullname == 'project':
                        self.__on_project(node)

                    elif self.__on_element_start_handler(fullname):
                        break

                    last_fullname = fullname

            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            # end element
            # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
            elif event == xml.dom.pulldom.END_ELEMENT:
                if self.__on_element_end_handler(last_fullname):
                    break
                last_fullname = last_fullname.rsplit('.', 1)[0]

        fp.close()
        zfile.close()

        self.__on_document_end_handler()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle __on_project node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_project(self, node):
        try:
            self.__evidence_count = int(node.getAttribute('ModelCount') or '0')

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle __on_case_information node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_case_information(self, doc, node):
        try:
            doc.expandNode(node)

            for child in node.getElementsByTagName('field'):
                varname = child.getAttribute('fieldType')
                content = child.firstChild.nodeValue.strip()
                self.__case_info.set(varname, content)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle <sourceExtractions> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_source_extractions(self, doc, node):
        try:
            doc.expandNode(node)

            for child in node.getElementsByTagName('extractionInfo'):
                obj = pymobius.Data()
                obj.id = int(child.getAttribute('id') or '-1')
                obj.name = child.getAttribute('name')
                obj.device_name = child.getAttribute('fullName')
                obj.extraction_type = child.getAttribute('type')
                obj.metadata = []

                self.__extractions.append(obj)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle <metadata section="Extraction Data"> node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_extraction_data(self, doc, node):
        try:
            doc.expandNode(node)

            for child in node.getElementsByTagName('item'):
                source_extraction = child.getAttribute('sourceExtraction')

                if source_extraction:
                    source = int(source_extraction)
                    name = child.getAttribute('name')
                    value = child.firstChild.nodeValue.strip()
                    self.__extractions[source].metadata.append((name, value))

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle __on_device_info node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_device_info(self, doc, node):
        try:
            doc.expandNode(node)

            for child in node.getElementsByTagName('item'):
                source = int(child.getAttribute('sourceExtraction'))
                name = child.getAttribute('name')
                value = child.firstChild.nodeValue.strip()
                self.__extractions[source].metadata.append((name, value))

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle __on_tagged_file
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_tagged_file(self, doc, node):
        try:
            doc.expandNode(node)

            f = pymobius.Data()
            f.id = node.getAttribute("id")
            f.size = int(node.getAttribute("size") or '-1')
            f.path = node.getAttribute("path")
            f.fs_id = node.getAttribute("fsid")
            f.fs = node.getAttribute("fs")
            f.extraction_id = int(node.getAttribute('extractionId') or '-1')
            f.deleted = node.getAttribute("deleted")
            f.embedded = node.getAttribute("embedded")
            f.is_related = node.getAttribute("isrelated")
            f.decoded_by = node.getAttribute("decodedBy")
            f.decoded_app = node.getAttribute("decodedApplication")
            f.source_index = int(node.getAttribute("source_index") or '-1')
            f.access_info = {}
            f.metadata = {}
            f.file_metadata = {}

            # children
            for child in node.childNodes:

                # accessInfo
                if child.nodeType == child.ELEMENT_NODE and child.tagName == 'accessInfo':
                    for ts_node in child.getElementsByTagName('timestamp'):
                        name = ts_node.getAttribute("name")
                        timestamp = get_datetime_from_node(ts_node.firstChild.nodeValue)
                        f.access_info[name] = timestamp

                # metadata
                elif child.nodeType == child.ELEMENT_NODE and child.tagName == 'metadata':
                    section = child.getAttribute("section")

                    for i_node in child.getElementsByTagName('item'):
                        name = i_node.getAttribute("name")
                        value = None

                        if i_node.firstChild:
                            value = (i_node.firstChild.nodeValue or '').strip()

                        if section == 'File':
                            f.file_metadata[name] = value

                        elif section == 'MetaData':
                            f.metadata[name] = value

                        else:
                            mobius.core.logf(f"DEV unhandled file metadata section: {section}")

            return self.__on_tagged_file_handler(f)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Handle __on_evidence
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_evidence(self, doc, node):
        try:
            doc.expandNode(node)

            e = self.__get_model(node)
            e.evidence_count = self.__evidence_count
            self.__on_evidence_handler(e)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get model from XML node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_model(self, node):
        m = pymobius.Data()

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Attributes
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for name, value in node.attributes.items():
            setattr(m, name, value)

        m.extraction_id = int(node.getAttribute('extractionId'))
        m.is_carved = node.getAttribute("iscarved") == "True"
        m.is_related = node.getAttribute("isrelated") == "True"
        m.metadata = []
        m.jump_targets = []

        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        # Children nodes
        # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE:
                name = child.getAttribute('name')

                if child.tagName == 'field':
                    value = self.__get_field_value(child)
                    m.metadata.append((name, value))

                elif child.tagName == 'multiField':
                    value = self.__get_multifield_value(child)
                    m.metadata.append((name, value))

                elif child.tagName == 'modelField':
                    value = self.__get_modelfield_value(child)
                    m.metadata.append((name, value))

                elif child.tagName == 'multiModelField':
                    value = self.__get_multimodelfield_value(child)
                    m.metadata.append((name, value))

                elif child.tagName == 'dataField':
                    value = self.__get_datafield_value(child)
                    m.metadata.append((name, value))

                elif child.tagName == 'nodeField':
                    value = self.__get_nodefield_value(child)
                    m.metadata.append((name, value))

                elif child.tagName == 'jumptargets':
                    value = self.__get_jumptargets(child)
                    m.jump_targets += value

                else:
                    value = self.__get_related_models(child)
                    m.metadata.append((name, value))

        # RelatedModels

        return m

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_value(self, value_type, value):

        try:
            if value_type == 'Boolean':
                value = (value == 'True')

            elif value_type in ('UInt32', 'Int32', 'Uint64', 'Int64'):
                value = int(value)

            elif value_type == 'Double':
                value = float(value.replace(',', '.'))

            elif value_type == 'TimeStamp':
                value = get_datetime_from_node(value)

            elif value_type != 'String':
                self.__unknown_datatypes.add(value_type)

        except Exception as e:
            mobius.core.logf(f"WRN {str(e)}\n{traceback.format_exc()}")
            value = None

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get field value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_field_value(self, node):
        value = None

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'value':
                value_type = child.getAttribute("type")
                value = self.__get_value(value_type, child.firstChild.nodeValue.strip())

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get multifield value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_multifield_value(self, node):
        values = []

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'value':
                value_type = child.getAttribute("type")

                if child.firstChild:
                    value = self.__get_value(value_type, child.firstChild.nodeValue.strip())
                else:
                    value = None

                values.append(value)

        return values

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get modelField value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_modelfield_value(self, node):
        value = None

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'model':
                value = self.__get_model(child)

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get multimodelfield value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_multimodelfield_value(self, node):
        values = []

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'model':
                model = self.__get_model(child)
                values.append(model)

        return values

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get dataField value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_datafield_value(self, node):
        value = None
        field_type = node.getAttribute('type')

        if field_type == 'MemoryRange':
            for child in node.getElementsByTagName('source'):
                value = pymobius.Data()
                value.length = int(child.getAttribute('length') or '-1')

        else:
            mobius.core.logf(f"DEV unhandled datafield type: {field_type}")

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get nodeField value
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_nodefield_value(self, node):
        value = None

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'id':
                value = pymobius.Data()
                value.name = child.getAttribute("name")
                value.id = child.firstChild.nodeValue.strip()

        return value

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get jump targets from node
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_jumptargets(self, node):
        targets = []

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'targetid':
                target = pymobius.Data()
                target.is_model = child.getAttribute("ismodel") == 'true'
                target.id = child.firstChild.nodeValue.strip()

                targets.append(target)

        return targets

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get related models
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_related_models(self, node):
        values = []

        for child in node.childNodes:
            if child.nodeType == child.ELEMENT_NODE and child.tagName == 'model':
                values.append(self.__get_model(child))

        return values
