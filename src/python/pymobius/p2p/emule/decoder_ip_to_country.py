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
import csv
import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief IP to Country table
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class IPTable (object):

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief initialize object
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def __init__ (self, reader):
    self.__values = []
    fp = mobius.io.line_reader (reader, 'utf-8', '\n')
    values = []

    for data in csv.reader (fp):
      if len (data) == 5 and data[2] != 'ZZ':
        values.append ((int (data[0]), int (data[1]), data[3], data[4]))

    self.__values = list (sorted (v for v in values))

  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  # @brief get country
  # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  def get (self, ip):

    # transform IP into int
    v = [ int (x) for x in ip.split ('.') ]
    if len (v) != 4:
      return None, None

    ip = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | (v[3])

    # lookup IP, using binary search
    lo = 0
    hi = len (self.__values)
    old_mid = -1
    found = False

    while (lo < hi) and not found:

      mid = (lo + hi) // 2
      v = self.__values[mid]

      if old_mid == mid:
        lo = hi

      elif v[0] > ip:
        hi = mid

      elif v[1] < ip:
        lo = mid

      else:
        found = True

      old_mid = mid

    # return value, if found
    if found:
      return v[2], v[3]

    else:
      return None, None

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Retrieve data from ip-to-country.csv
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def retrieve (f):
  iptable = None

  # try to read file
  reader = f.new_reader ()
  if reader:
    ip_table = IPTable (reader)

  # return table
  return ip_table
