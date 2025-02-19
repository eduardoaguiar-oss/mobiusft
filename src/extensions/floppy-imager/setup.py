# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Mobius Forensic Toolkit
# Copyright (C) 2008,2009,2010 Eduardo Aguiar
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
import os
import re
import subprocess
import tempfile
import mobius


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Installation class
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Installer(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Initialize object
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.fdgetprm = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Install
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def run(self):
        self.get_fdgetprm()
        self.get_fdgetdrvprm()
        self.install_in('main.py')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get FDGETPRM
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_fdgetprm(self):

        self.fdgetprm = self.__run_c_code('''
#include <linux/fd.h>
#include <stdio.h>

int
main ()
{
  printf ("0x%08x", FDGETPRM);
  return 0;
}
''')

        if self.fdgetprm:
            mobius.core.logf(f"INF FDGETPRM = {self.fdgetprm}")
        else:
            mobius.core.logf("WRN could not get FDGETPRM (Floppy Imager will not work)")
            self.fdgetprm = -1

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Get FDGETDRVPRM
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_fdgetdrvprm(self):

        self.fdgetdrvprm = self.__run_c_code('''
#include <linux/fd.h>
#include <stdio.h>

int
main ()
{
  printf ("0x%08x", FDGETDRVPRM);
  return 0;
}
''')

        if self.fdgetdrvprm:
            mobius.core.logf(f"INF FDGETDRVPRM = {self.fdgetdrvprm}")
        else:
            mobius.core.logf("WRN could not get FDGETDRVPRM (Floppy Imager will not work)")
            self.fdgetdrvprm = -1

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Compile and run C code
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __run_c_code(self, code):

        # create code
        fd, tmpfile = tempfile.mkstemp('.c')
        os.write(fd, code.encode('utf-8'))
        os.close(fd)
        exefile = os.path.splitext(tmpfile)[0]

        # compile
        rc = os.system(f'cc -s {tmpfile} -o {exefile} >/dev/null 2>&1')
        if rc != 0:
            return None

        # run
        proc = subprocess.Popen([exefile], stdout=subprocess.PIPE)
        return proc.communicate()[0].decode('utf-8')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Substitute configure vars (${var})
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def install_in(self, path):

        # read file
        fp = open(path, 'r', encoding='utf-8')
        lines = fp.readlines()
        fp.close()

        # replace function
        def repl(matchobj):
            name = matchobj.group(1)
            return getattr(self, name, None)

        # replace vars
        fp = open(path, 'w', encoding='utf-8')
        for line in lines:
            fp.write(re.sub(r'\$\{([a-z0-9_]*?)}', repl, line))
        fp.close()


installer = Installer()
installer.run()
