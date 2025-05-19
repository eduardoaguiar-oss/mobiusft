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
import os
import os.path
import re
import shutil

import mobius
import mobius.core.io
import pymobius.xml

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
EXTENSION_ID = 'report-model'
EXTENSION_NAME = 'Report Model'
EXTENSION_AUTHOR = 'Eduardo Aguiar'
EXTENSION_VERSION = '1.5'
EXTENSION_DESCRIPTION = 'Report model and services'

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Constants
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
VALUE_EXP = re.compile(r'\${(.*?)}')
STATEMENT_VARS = {'text': {'text': '', 'newline': True},
                  'verbatim': {'text': '', 'newline': True, 'use_exp_value': True},
                  'for': {'vars': '', 'values': '', 'code': '@BLOCK@'},
                  'while': {'condition': '', 'code': '@BLOCK@'},
                  'if': {'condition': '', 'if_code': '@BLOCK@', 'else_code': '@BLOCK@', 'elifs': '@LIST@'},
                  'elif': {'condition': '', 'code': '@BLOCK@'},
                  'assign': {'var': '', 'value': ''},
                  'call': {'report': '', 'args': ''},
                  'exec': {'cmd': ''},
                  'output': {'filename': ''},
                  'block': {'statements': '@LIST@'},
                  'report': {'name': '', 'code': '@BLOCK@', 'args': ''}}

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Extension data
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
g_cachedir = None
g_reportdir = None


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generic generator for python source
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class PySource(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief create sourcefile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self, path):
        self.pos = 0
        f = mobius.core.io.new_file_by_path(path)
        self.__fp = mobius.core.io.text_writer(f.new_writer())

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief close sourcefile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def close(self):
        self.__fp = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief indent one level
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def indent(self):
        self.pos += 1

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief dedent one level
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def dedent(self):
        self.pos -= 1

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief write string to sourcefile
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def write(self, text='', indentation=True, newline=True, escaped=False):
        if indentation:
            self.__fp.write('  ' * self.pos)

        if escaped:
            text = text.replace('\\', '\\\\')
            text = text.replace("'", "\\'")
            # text = text.replace (r"%", r"%%")
            text = text.replace('\n', '\\n')
            text = text.replace('\t', '\\t')

        self.__fp.write(text or '')

        if newline:
            self.__fp.write('\n')


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief generate python code from report template
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Generator(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate code
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate(self, report_id, pyfile, mediator):
        self.mediator = mediator

        # create cache subdir, if necessary
        pydir = os.path.dirname(pyfile)
        if not os.path.exists(pydir):
            os.makedirs(pydir)

        # create .dep file
        deppath = os.path.join(pydir, '%s.dep' % report_id)
        depfile = mobius.core.io.new_file_by_path(deppath)
        depwriter = mobius.core.io.text_writer(depfile.new_writer())

        # generate source
        try:
            source = PySource(pyfile)
            self.generate_py_header(source)
            self.generate_py_write_latex(source)
            self.generate_py_write_verbatim(source)
            self.generate_py_main_method(source, report_id, depwriter)
            self.generate_py_report_methods(source, depwriter)
            self.generate_py_footer(source)
        except Exception:
            os.remove(deppath)
            raise

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate source header
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_py_header(self, source):
        source.write('#%s' % ('-=' * 39))
        source.write('# Generated by Mobius Forensic Toolkit. Do not edit this file')
        source.write('#%s' % ('-=' * 39))
        source.write('class Generator (object):')

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate source write_latex method
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_py_write_latex(self, source):
        source.write()
        source.write('  def write_latex (self, output, s):')
        source.write("    s = s or ''")
        source.write("    s = s.replace ('\\\\', '\\\\textbackslash{}')")
        source.write("    s = s.replace ('_', '\\\\_')")
        source.write("    s = s.replace ('{', '\\\\{')")
        source.write("    s = s.replace ('}', '\\\\}')")
        source.write("    s = s.replace ('#', '\\\\#')")
        source.write("    s = s.replace ('&', '\\\\&')")
        source.write("    s = s.replace ('%', '\\\\%')")
        source.write("    s = s.replace ('$', '\\\\$')")
        source.write("    s = s.replace ('^', '\\\\textasciicircum{}')")
        source.write("    s = s.replace ('°C', '\\\\textcelsius{}')")
        source.write("    s = s.replace ('\\\\textbackslash\\\\{\\\\}', '\\\\textbackslash{}')")
        source.write("    output.write (s.encode ('utf-8'))")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief Generate source write_verbatim method
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_py_write_verbatim(self, source):
        source.write()
        source.write('  def write_verbatim (self, output, s):')
        source.write("    s = s or ''")
        source.write("    output.write (s.encode ('utf-8'))")

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate main method
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_py_main_method(self, source, report_id, depfile):
        self.references = []
        self.generated = set()

        report = self.mediator.call('report.load', report_id)
        args = 'self'
        if report.args:
            args = args + ', ' + report.args

        source.write()
        source.write('  def generate (%s):' % args)
        source.write('    output = None')
        source.indent()
        self.generate_statement(source, report.code)
        source.dedent()

        depfile.write('%s\n' % report_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate nested report
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_py_report_methods(self, source, depfile):

        # generate nested reports methods
        while self.references:
            reference_id = self.references[0]

            if reference_id not in self.generated:
                self.generated.add(reference_id)
                report = self.mediator.call('report.load', reference_id)
                if not report:
                    raise Exception('unknown report reference: ' + reference_id)

                args = 'self, output'
                if report.args:
                    args = args + ', ' + report.args

                source.write()
                source.write('  def generate_%s (%s):' % (self.get_underscore_id(reference_id), args))
                source.indent()
                self.generate_statement(source, report.code)
                source.dedent()

            self.references = self.references[1:]
            depfile.write('%s\n' % reference_id)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate source footer
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_py_footer(self, source):
        source.write()
        source.write('generator = Generator ()')
        source.write('generator.generate (*args)')
        source.close()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate statement
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_statement(self, source, obj):
        method_name = 'generate_%s' % obj.statement.replace('-', '_')
        statement_method = getattr(self, method_name, None)

        if statement_method:
            statement_method(source, obj)

        else:
            raise Exception('unknown statement: ' + obj.statement)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate block
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_block(self, source, obj):
        source.indent()

        if not obj.statements:
            source.write('pass')
        else:
            for statement in obj.statements:
                self.generate_statement(source, statement)

        source.dedent()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate verbatim
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_verbatim(self, source, obj):
        source.write("self.write_verbatim (output, '", newline=False)

        if obj.newline:
            newline_str = '\\n'
        else:
            newline_str = ''

        if hasattr(obj, 'use_exp_value') and obj.use_exp_value:
            fmt, args = self.get_expand_vars(obj.text)
            if args:
                source.write("%s%s' %% (%s))" % (fmt, newline_str, ', '.join(args)), indentation=False)

            else:
                source.write(obj.text, indentation=False, newline=False, escaped=True)
                source.write("%s')" % newline_str, indentation=False)

        else:
            source.write(obj.text, indentation=False, newline=False, escaped=True)
            source.write("%s')" % newline_str, indentation=False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate text
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_text(self, source, obj):
        fmt, args = self.get_expand_vars(obj.text)
        source.write("self.write_latex (output, '%s" % fmt, newline=False)

        if obj.newline:
            source.write('\\n', indentation=False, newline=False)
        source.write("'", indentation=False, newline=False)

        if args:
            source.write(" %% (%s)" % ', '.join(args), newline=False, indentation=False)

        source.write(")", indentation=False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate output
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_output(self, source, obj):
        frmt, args = self.get_expand_vars(obj.filename)
        source.write("output = open ('%s'" % frmt, newline=False)

        if args:
            source.write(" %% (%s)" % ', '.join(args), newline=False, indentation=False)

        source.write(", 'wb')", indentation=False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate assign
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_assign(self, source, obj):
        source.write('%s = %s' % (obj.var, obj.value))

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate call
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_call(self, source, obj):
        if obj.args:
            args = 'output, ' + obj.args
        else:
            args = 'output'

        source.write('self.generate_%s (%s)' % (self.get_underscore_id(obj.report), args))

        if obj.report not in self.generated:
            self.references.append(obj.report)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate exec
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_exec(self, source, obj):
        source.write('%s' % obj.cmd)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate for
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_for(self, source, obj):
        source.write('for %s in %s:' % (obj.vars, obj.values))
        self.generate_statement(source, obj.code)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate while
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_while(self, source, obj):
        source.write('while %s:' % obj.condition)
        self.generate_statement(source, obj.code)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate if
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_if(self, source, obj):
        source.write('if %s:' % obj.condition)
        self.generate_statement(source, obj.if_code)

        for item in obj.elifs:
            source.write('elif %s:' % item.condition)
            self.generate_statement(source, item.code)

        source.write('else:')
        self.generate_statement(source, obj.else_code)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief generate switch
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def generate_switch(self, source, obj):
        first = True
        for case in obj.cases:
            if first:
                stmt = 'if'
                first = False
            else:
                stmt = 'elif'
            source.write('%s %s == %s:' % (stmt, obj.expression, case.value))
            source.indent()
            self.generate_statement(source, case.code)
            source.dedent()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return underscored report.id string
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_underscore_id(self, report_id):
        report_id = report_id.replace('-', '_')
        report_id = report_id.replace('.', '_')
        return report_id

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief return format string and args for ${} strings
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_expand_vars(self, text):
        pos = 0
        frmt = ''
        args = []

        for exp in VALUE_EXP.finditer(text):
            frmt += self.py_escape(text[pos:exp.start(0)]) + r'%s'
            args.append(exp.group(1))
            pos = exp.end(0)

        frmt += self.py_escape(text[pos:])

        return frmt, args

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief escape python controle chars
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def py_escape(self, s):
        s = s.replace('\\', '\\\\')
        s = s.replace("'", "\\'")
        s = s.replace(r"%", r"%%")
        s = s.replace('\n', '\\n')
        s = s.replace('\t', '\\t')
        return s


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Report statement
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class Statement(object):
    def __init__(self):
        self.statement = ''


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief create new report statement
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def new_statement(stmt_id):
    stmt = Statement()
    stmt.statement = stmt_id

    # fill statement vars
    stmt_vars = STATEMENT_VARS.get(stmt_id, {})

    for name, value in stmt_vars.items():
        if value == '@BLOCK@':
            value = new_statement('block')

        elif value == '@LIST@':
            value = []

        setattr(stmt, name, value)

    return stmt


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief return report path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_report_path(report_id):
    global g_reportdir

    if not g_reportdir:
        app = mobius.core.application()
        g_reportdir = app.get_config_path('report')
    return os.path.join(g_reportdir, '%s.report' % report_id.replace('.', '/'))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Return report folder path
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_report_folder_path(folder_id):
    global g_reportdir

    if not g_reportdir:
        app = mobius.core.application()
        g_reportdir = app.get_config_path('report')
    return os.path.join(g_reportdir, folder_id.replace('.', '/'))


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Return path to report .py code
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_report_py_path(report_id):
    global g_cachedir

    if not g_cachedir:
        app = mobius.core.application()
        g_cachedir = app.get_cache_path('report')

    pyfile = os.path.join(g_cachedir, '%s.py' % report_id)
    depfile = os.path.join(g_cachedir, '%s.dep' % report_id)

    # check if cached code is still valid
    cache_is_valid = False

    if os.path.exists(depfile):
        st_depfile = os.stat(depfile)  # depfile modification time
        cache_is_valid = True

        # compare with references' modification time
        for line in open(depfile, encoding='utf-8'):
            reference_file = get_report_path(line.rstrip())

            if not os.path.exists(reference_file):
                cache_is_valid = False

            else:
                st_ref = os.stat(reference_file)

                if st_ref.st_mtime > st_depfile.st_mtime:
                    cache_is_valid = False

    # generate report code if necessary
    if not cache_is_valid:
        generator = Generator()
        generator.generate(report_id, pyfile, pymobius.mediator)

    # return path to generated code
    return pyfile


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.iter> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_iter():
    global g_reportdir

    if not g_reportdir:
        app = mobius.core.application()
        g_reportdir = app.get_config_path('report')

    pos = len(g_reportdir) + 1

    for root, dirs, files in os.walk(g_reportdir, topdown=True):
        for name in dirs:
            abs_path = os.path.join(root, name)
            rel_path = abs_path[pos:]
            name = os.path.splitext(rel_path)[0].replace('/', '.')
            if name:
                yield 'FOLDER', name

        for name in files:
            abs_path = os.path.join(root, name)
            rel_path = abs_path[pos:]
            name = os.path.splitext(rel_path)[0].replace('/', '.')
            yield 'REPORT', name


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.load> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_load(report_id):
    path = get_report_path(report_id)

    if os.path.exists(path):
        pickle = pymobius.xml.Pickle()
        report = pickle.load(path)
    else:
        report = None

    return report


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.new> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_new():
    report = new_statement('report')
    return report


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.new-folder> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_new_folder(folder_id):
    path = get_report_folder_path(folder_id)
    os.makedirs(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.new-statement> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_new_statement(stmt_id):
    stmt = new_statement(stmt_id)
    return stmt


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.remove> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_remove(report_id):
    path = get_report_path(report_id)

    if os.path.exists(path):
        os.remove(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.remove-folder> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_remove_folder(folder_id):
    path = get_report_folder_path(folder_id)

    if os.path.isdir(path):
        shutil.rmtree(path)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.run> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_run(report_id, *args):
    # check if report exists
    template_path = get_report_path(report_id)

    if not os.path.exists(template_path):
        raise Exception('Report "%s" does not exist' % report_id)

    # get report's python code
    pyfile = get_report_py_path(report_id)

    # generic data holder class
    class Data(object):
        pass

    # execute report
    g_data = globals().copy()
    g_data['Data'] = Data
    l_data = {'args': args}

    fp = open(pyfile, 'rb')
    data = fp.read()
    fp.close()

    exec(data, g_data, l_data)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Service <report.save> implementation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def svc_report_save(report, report_id):
    path = get_report_path(report_id)

    dirname = os.path.dirname(path)
    if not os.path.exists(dirname):
        os.makedirs(dirname)

    pickle = pymobius.xml.Pickle()
    pickle.save(path, report)


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief API initialization
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def pvt_start_api():
    pymobius.mediator.advertise('report.iter', svc_report_iter)
    pymobius.mediator.advertise('report.load', svc_report_load)
    pymobius.mediator.advertise('report.new', svc_report_new)
    pymobius.mediator.advertise('report.new-folder', svc_report_new_folder)
    pymobius.mediator.advertise('report.new-statement', svc_report_new_statement)
    pymobius.mediator.advertise('report.remove', svc_report_remove)
    pymobius.mediator.advertise('report.remove-folder', svc_report_remove_folder)
    pymobius.mediator.advertise('report.run', svc_report_run)
    pymobius.mediator.advertise('report.save', svc_report_save)
