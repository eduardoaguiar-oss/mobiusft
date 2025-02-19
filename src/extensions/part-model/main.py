# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023 Eduardo Aguiar
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
import threading
import sqlite3
import mobius

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'part-model'
EXTENSION_NAME = 'Part Model'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '1.6'
EXTENSION_DESCRIPTION = 'Part model'


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Part
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Part(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.category = None
        self.id = None
        self.attributes = {}


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief database object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Database(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, path):
        self.__db = sqlite3.connect(path, check_same_thread=False)
        self.__db.text_factory = str
        self.__db.execute('PRAGMA foreign_keys=ON')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief destroy object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __del__(self):
        if self.__db:
            self.close()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief close database
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def close(self):
        self.__db.close()
        self.__db = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief commit database
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def commit(self):
        self.__db.commit()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief rollback database
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def rollback(self):
        self.__db.rollback()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief execute statement
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def execute(self, *args):
        return self.__db.execute(*args)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief create cursor to database
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def cursor(self):
        return self.__db.cursor()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief create tables, if necessary
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def create_tables(self):
        cursor = self.__db.cursor()

        cursor.execute('''CREATE TABLE IF NOT EXISTS part (
                        uid INTEGER PRIMARY KEY AUTOINCREMENT,
                        category TEXT NOT NULL,
                        id TEXT NOT NULL
                       )''')

        cursor.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_part ON part (category, id)")

        cursor.execute('''CREATE TABLE IF NOT EXISTS attribute (
                        uid INTEGER PRIMARY KEY AUTOINCREMENT,
                        part_id INTEGER NOT NULL,
                        attr_id TEXT NOT NULL,
                        attr_value TEXT,
                        FOREIGN KEY(part_id) REFERENCES part(uid) ON DELETE CASCADE
                       )''')

        cursor.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_attribute ON attribute (part_id, attr_id)")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return last insert rowid
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_last_insert_rowid(self):
        query = self.__db.execute("select last_insert_rowid ()")
        data = query.fetchone()

        if data:
            return data[0]

        return None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief select one part by category and id
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_part(self, category_id, part_id):

        # get part uid
        part_uid = self.__get_part_uid(category_id, part_id)
        if part_uid == None:
            return None

        # populate part object
        part = Part()
        part.category = category_id
        part.id = part_id

        # get attributes
        query = self.__db.execute('''SELECT attr_id, attr_value
                                    FROM attribute
                                   WHERE part_id = ?''', [part_uid])

        part.attributes.update(query.fetchall())
        query.close()

        return part

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set part
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_part(self, part):

        # insert part, if necessary
        part_uid = self.__get_part_uid(part.category, part.id)

        if part_uid == None:
            self.__db.execute('''INSERT INTO part
                                 VALUES (null, ?, ?)''', [part.category, part.id])

            part_uid = self.get_last_insert_rowid()

        # delete old attributes, if any
        self.__db.execute('''DELETE FROM attribute
                                WHERE part_id = ?''', [part_uid])

        # insert attributes
        values = [(part_uid, attr_id, attr_value) for (attr_id, attr_value) in part.attributes.items() if
                  attr_value != None]
        self.__db.executemany('''INSERT INTO attribute
                                   VALUES (null, ?, ?, ?)''', values)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief creat new part
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def new_part(self, category_id, part_id):
        self.__db.execute('''INSERT INTO part
                               VALUES (null, ?, ?)''', [category_id, part_id])

        part = Part()
        part.category = category_id
        part.id = part_id

        return part

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief remove part
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove_part(self, part):
        self.__db.execute('''DELETE FROM part
                                WHERE category = ?
                                  AND id = ?''', [part.category, part.id])

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief select all parts
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def select_part_all(self):
        query = self.__db.execute('''SELECT p.category, p.id, a.attr_id, a.attr_value
                                    FROM part p
                                         LEFT OUTER JOIN attribute a
                                                      ON a.part_id = p.uid''')
        part_list = self.__build_part_list(query)
        query.close()

        return part_list

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief select parts by category
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def select_part_by_category(self, category_id):
        query = self.__db.execute('''SELECT p.category, p.id, a.attr_id, a.attr_value
                                    FROM part p LEFT OUTER JOIN attribute a ON a.part_id = p.uid
                                   WHERE p.category = ?''', [category_id])
        part_list = self.__build_part_list(query)
        query.close()

        return part_list

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get part uid
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __get_part_uid(self, category_id, part_id):
        query = self.__db.execute('''SELECT uid
                                    FROM part
                                   WHERE category = ?
                                     AND id = ?''', [category_id, part_id])
        data = query.fetchone()
        query.close()

        if data:
            uid = data[0]

        else:
            uid = None

        return uid

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief build list of parts from a query result
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __build_part_list(self, query):
        part = Part()
        part.category = None
        part.id = None

        part_list = []

        for row in query:
            category_id, part_id, attr_id, attr_value = row

            if part_id != part.id:
                if part.id:
                    part_list.append(part)  # append last part

                part = Part()  # populate part
                part.category = category_id
                part.id = part_id

            if attr_id != None and attr_value != None:
                part.attributes[attr_id] = attr_value

        if part.id:
            part_list.append(part)

        return part_list


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Extension data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
gdata.db = None
gdata.transaction_lock = threading.Lock()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief return database object
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_database():
    if not gdata.db:
        app = mobius.core.application()
        path = app.get_config_path('part-catalogue.sqlite')
        gdata.db = Database(path)
        gdata.db.create_tables()
    return gdata.db


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.get> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_get(category_id, part_id):
    db = get_database()

    return db.get_part(category_id, part_id)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.get_all> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_get_all():
    db = get_database()

    return db.select_part_all()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.get_by_category> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_get_by_category(category_id):
    db = get_database()

    return db.select_part_by_category(category_id)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.new> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_new(category_id, part_id):
    db = get_database()
    part = None

    try:
        gdata.transaction_lock.acquire()
        part = db.new_part(category_id, part_id)
        db.commit()

    except Exception as e:
        mobius.core.logf('WRN ' + str(e))
        db.rollback()

    finally:
        gdata.transaction_lock.release()

    return part


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.remove> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_remove(part):
    db = get_database()

    try:
        gdata.transaction_lock.acquire()
        db.remove_part(part)
        db.commit()

    except Exception as e:
        mobius.core.logf('WRN ' + str(e))
        db.rollback()

    finally:
        gdata.transaction_lock.release()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.set> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_set(part):
    db = get_database()

    try:
        gdata.transaction_lock.acquire()
        db.set_part(part)
        db.commit()

    except Exception as e:
        mobius.core.logf('WRN ' + str(e))
        db.rollback()

    finally:
        gdata.transaction_lock.release()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# service <part.set_all> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_part_set_all(part_list):
    db = get_database()

    try:
        gdata.transaction_lock.acquire()

        for new_part in part_list:
            part = db.get_part(new_part.category, new_part.id)

            if not part:
                db.set_part(new_part)

        db.commit()

    except Exception as e:
        mobius.core.logf('WRN ' + str(e))
        db.rollback()

    finally:
        gdata.transaction_lock.release()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# API initialization
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start_api():
    gdata.mediator.advertise('part.get', svc_part_get)
    gdata.mediator.advertise('part.get_all', svc_part_get_all)
    gdata.mediator.advertise('part.get_by_category', svc_part_get_by_category)
    gdata.mediator.advertise('part.new', svc_part_new)
    gdata.mediator.advertise('part.remove', svc_part_remove)
    gdata.mediator.advertise('part.set', svc_part_set)
    gdata.mediator.advertise('part.set_all', svc_part_set_all)
