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
import libxml2
import datetime
import inspect
import mobius
import pymobius

XML_ENCODING='utf-8'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Generic object's instance
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class PyObject (object):
  pass

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Persistence layer
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Pickle (object):

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load XML file
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load (self, uri):
    self.__objects = {}

    doc = libxml2.parseFile (uri)
    node = doc.getRootElement ()
    name, value = self.load_item (node)

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load any item
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_item (self, node):
    name = node.prop ('name')
    value = None

    # basic types
    if node.type == 'element' and node.name == 'attribute':
      value, datatype = self.load_attribute (node)

    elif node.type == 'element' and node.name == 'string':
      value = self.load_string (node)

    elif node.type == 'element' and node.name == 'unicode':
      value = self.load_unicode (node)

    # container types
    elif node.type == 'element' and node.name == 'reference':
      value = self.load_reference (node)

    elif node.type == 'element' and node.name == 'tuple':
      value = self.load_tuple (node)

    elif node.type == 'element' and node.name == 'list':
      value = self.load_list (node)

    elif node.type == 'element' and node.name == 'set':
      value = self.load_set (node)

    elif node.type == 'element' and node.name == 'dict':
      value = self.load_dict (node)

    elif node.type == 'element' and node.name == 'pyobject':
      value = self.load_pyobject (node)

    elif node.type == 'element' and node.name == 'object':
      value = self.load_object (node)

    return name, value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <attribute>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_attribute (self, node):
    value = node.prop ('value')
    datatype = node.prop ('datatype')

    if datatype == 'int':
      value = int (value)

    elif datatype == 'long':	# @deprecated since 1.31
      value = int (value)

    elif datatype == 'bool':
      value = (value == "true")

    elif datatype == 'float':
      value = float (value)

    elif datatype == 'unicode':
      pass

    elif datatype == 'NoneType':
      value = None

    elif datatype == 'datetime':
      if value:
        value = datetime.datetime.strptime (value, '%Y-%m-%d %H:%M:%S')

    return value, datatype

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <string>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_string (self, node):
    value = node.getContent ()

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <unicode>
  # @deprecated since 1.31
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_unicode (self, node):
    value = node.getContent ()

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <reference>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_reference (self, node):
    object_id = node.prop ('id')
    return self.__objects.get (object_id)

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <tuple>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_tuple (self, node):
    object_id = node.prop ('id')
    value = []
    self.__objects[object_id] = value

    node = node.children

    while node:
      if node.type == 'element':
        item_name, item_value = self.load_item (node)
        value.append (item_value)
      node = node.next

    value = tuple (value)
    self.__objects[object_id] = value

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <list>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_list (self, node):
    object_id = node.prop ('id')
    value = []
    self.__objects[object_id] = value

    node = node.children

    while node:
      if node.type == 'element':
        item_name, item_value = self.load_item (node)
        value.append (item_value)
      node = node.next

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <set>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_set (self, node):
    object_id = node.prop ('id')
    value = set ()
    self.__objects[object_id] = value

    node = node.children

    while node:
      if node.type == 'element':
        item_name, item_value = self.load_item (node)
        value.add (item_value)
      node = node.next

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <dict>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_dict (self, node):
    object_id = node.prop ('id')
    value = {}
    self.__objects[object_id] = value

    node = node.children

    while node:
      if node.type == 'element':
        item_key, item_value = self.load_tuple (node)
        value[item_key] = item_value

      node = node.next

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <pyobject>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_pyobject (self, node):
    object_id = node.prop ('id')
    value = PyObject ()
    self.__objects[object_id] = value

    node = node.children

    while node:
      item_name, item_value = self.load_item (node)
      if item_name:
        setattr (value, item_name, item_value)
      node = node.next

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Load <object>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def load_object (self, node):
    object_id = node.prop ('id')
    value = pymobius.Data ()
    self.__objects[object_id] = value

    node = node.children

    while node:
      item_name, item_value = self.load_item (node)
      if item_name:
        setattr (value, item_name, item_value)
      node = node.next

    return value

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save XML file
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save (self, uri, value):
    self.__objects = {}

    doc = libxml2.newDoc ('1.0')
    node = self.save_item (value)
    doc.addChild (node)
    doc.saveFormatFileEnc (uri, XML_ENCODING, 1)
    doc.freeDoc ()

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save any item
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_item (self, value):

    # basic types
    if value == None:
      node = self.save_attribute (value)

    elif isinstance (value, (int, bool, float, datetime.datetime)):
      node = self.save_attribute (value)

    elif isinstance (value, str):
      node = self.save_string (value)

    # container types
    else:
      node = self.save_container (value)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <attribute>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_attribute (self, value):
    node = libxml2.newNode ('attribute')

    str_value = None
    str_datatype = None

    # datatype property
    if not isinstance (value, str):
      str_datatype = type (value).__name__

    # value
    if isinstance (value, bool):
      if value:
        str_value = 'true'

    elif isinstance (value, str):
      str_value = value

    elif isinstance (value, datetime.datetime):
      if value.year != 0:
        str_value = value.strftime ('%F %T')

    elif value != None:
      str_value = str (value)

    # set properties
    if str_value != None:
      node.setProp ('value', str_value)

    if str_datatype != None:
      node.setProp ('datatype', str_datatype)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <string>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_string (self, value):

    if '\n' in value:
      node = libxml2.newNode ('string')
      node.addContent (value)
    else:
      node = self.save_attribute (value)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save container objects (dict, set, list, tuple, object)
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_container (self, value):
    object_id = id (value)

    if object_id in self.__objects:
      node = libxml2.newNode ('reference')
      node.setProp ('id', str (object_id))

    else:
      self.__objects[object_id] = value

      if isinstance (value, tuple):
        node = self.save_tuple (value)

      elif isinstance (value, list):
        node = self.save_list (value)

      elif isinstance (value, set):
        node = self.save_set (value)

      elif isinstance (value, dict):
        node = self.save_dict (value)

      else:
        node = self.save_pyobject (value)

      node.setProp ('id', str (object_id))

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <tuple>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_tuple (self, value):
    node = libxml2.newNode ('tuple')

    for item in value:
      child = self.save_item (item)
      node.addChild (child)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <list>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_list (self, value):
    node = libxml2.newNode ('list')

    for item in value:
      child = self.save_item (item)
      node.addChild (child)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <set>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_set (self, value):
    node = libxml2.newNode ('set')

    for item in value:
      child = self.save_item (item)
      node.addChild (child)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <dict>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_dict (self, value):
    node = libxml2.newNode ('dict')

    for key, value in value.items ():
      child = self.save_tuple ((key, value))
      node.addChild (child)

    return node

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief Save <pyobject>
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def save_pyobject (self, value):
    node = libxml2.newNode ('pyobject')

    for name, value in inspect.getmembers (value):
      if not callable (value) and not name.startswith ('__'):
        child = self.save_item (value)
        child.setProp ('name', name)
        node.addChild (child)

    return node
