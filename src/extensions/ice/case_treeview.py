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
import mobius
import pymobius
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import Gtk

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief Null handler
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class NullHandler(object):

    def __init__(self, ret):
        self.__ret = ret

    def __call__(self, *args):
        return self.__ret


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief get control's handler for event
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
def get_handler(event_id, control, wid=None):
    event_full_id = wid + '_' + event_id if wid else event_id
    handler = getattr(control, event_full_id, None)

    return handler

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# @brief widget: treeview
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
(TREEITEM_ICON, TREEITEM_NAME, TREEITEM_OBJ) = range(3)
TREEITEM_MIMETYPE = 'application/x-mobius-treeitem'


class CaseTreeView(object):

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief initialize widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __init__(self):
        self.__mediator = pymobius.mediator.copy()
        self.__widget = Gtk.Frame()

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        sw.show()
        self.__widget.add(sw)

        datastore = Gtk.TreeStore.new([GdkPixbuf.Pixbuf, str, object])

        self.__treeview = Gtk.TreeView.new_with_model(datastore)
        self.__treeview.connect('test-expand-row', self.__on_test_expand_row)
        self.__treeview.show()
        sw.add(self.__treeview)

        self.__treeview.set_headers_visible(False)
        self.__treeview.set_enable_tree_lines(True)

        tvcolumn = Gtk.TreeViewColumn()
        tvcolumn.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        self.__treeview.append_column(tvcolumn)

        renderer = Gtk.CellRendererPixbuf()
        tvcolumn.pack_start(renderer, False)
        tvcolumn.add_attribute(renderer, 'pixbuf', TREEITEM_ICON)

        renderer = Gtk.CellRendererText()
        tvcolumn.pack_start(renderer, True)
        tvcolumn.add_attribute(renderer, 'text', TREEITEM_NAME)

        # set data
        self.__dnd_source_targets = []
        self.__dnd_source_next_id = 0
        self.__dnd_dest_targets = []
        self.__dnd_dest_next_id = 0
        self.__category_icons = {}

        # set handlers
        self.__on_retrieve_icon_handler = None
        self.__on_retrieve_name_handler = None
        self.__on_retrieve_children_handler = None
        self.__on_reordering_item_handler = None
        self.__on_selection_changed_handler = None
        self.__on_file_dropped_handler = None

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set control object
    # @param control control object
    # @param wid widget id to be appended to the handler functions' names
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_control(self, control, wid=None):

        # event: on_retrieve_icon
        self.__on_retrieve_icon_handler = get_handler('on_retrieve_icon', control, wid) or \
                                          NullHandler(None)

        # event: on_retrieve_name
        self.__on_retrieve_name_handler = get_handler('on_retrieve_name', control, wid) or NullHandler(None)

        # event: on_retrieve_is_expandable
        self.__on_retrieve_is_expandable_handler = (get_handler('on_retrieve_is_expandable', control, wid) or
                                                    NullHandler(None))

        # event: on_retrieve_children
        self.__on_retrieve_children_handler = (get_handler('on_retrieve_children', control, wid) or
                                               NullHandler([]))

        # event: on_reordering_item
        self.__on_reordering_item_handler = get_handler('on_reordering_item', control, wid)

        if self.__on_reordering_item_handler:
            self.__add_source_dnd_target(TREEITEM_MIMETYPE, 0, Gdk.DragAction.MOVE)
            self.__add_dest_dnd_target(TREEITEM_MIMETYPE, Gtk.TargetFlags.SAME_WIDGET, Gdk.DragAction.MOVE)
            self.__treeview.connect('drag-data-received', self.__on_drag_data_received)

        # event: on_selection_changed
        self.__on_selection_changed_handler = get_handler('on_selection_changed', control, wid)

        if self.__on_selection_changed_handler:
            selection = self.__treeview.get_selection()
            selection.connect('changed', self.__on_selection_changed)

        # event: on_remove_item
        self.__on_remove_item_handler = get_handler('on_remove_item', control, wid) or NullHandler(None)

        # event: on_file_dropped
        self.__on_file_dropped_handler = get_handler('on_file_dropped', control, wid)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get ui widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_ui_widget(self):
        return self.__widget

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief show widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def show(self):
        return self.__widget.show()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief hide widget
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def hide(self):
        return self.__widget.hide()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief clear treeview
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def clear(self):
        model = self.__treeview.get_model()
        model.clear()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set treeview root items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_root_items(self, itemlist):
        model = self.__treeview.get_model()
        model.clear()

        # load items
        for item in itemlist:
            icon = self.__retrieve_icon(item)
            name = self.__retrieve_name(item)
            is_expandable = self.__retrieve_is_expandable(item)
            treeiter = model.append(None, (icon, name, item))

            # insert place holder if item is expandable
            if is_expandable:
                model.append(treeiter)

            # expand item at iter
            path = model.get_path(treeiter)
            self.__treeview.expand_row(path, False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set widget sensitive status
    # @param sensitive True/False
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_sensitive(self, sensitive):
        return self.__widget.set_sensitive(sensitive)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief set multiple selection mode
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def set_multiple_selection(self, flag):
        selection = self.__treeview.get_selection()
        selection.set_mode(Gtk.SelectionMode.MULTIPLE if flag else Gtk.SelectionMode.SINGLE)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add mimetype to the list of allowed drop file types
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def add_dnd_dest_mimetype(self, mimetype):
        self.__add_dest_dnd_target(mimetype, 0, Gdk.DragAction.COPY)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief count selected items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def count_selected_items(self):
        selection = self.__treeview.get_selection()
        return selection.count_selected_rows()

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief get selected items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def get_selected_items(self):
        selection = self.__treeview.get_selection()
        model, pathlist = selection.get_selected_rows()
        item_list = []

        for path in pathlist:
            treeiter = model.get_iter(path)
            item = model.get_value(treeiter, TREEITEM_OBJ)
            item_list.append(item)

        return item_list

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief reload selected items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def reload_selected_items(self, reload_children=True):
        selection = self.__treeview.get_selection()
        model, pathlist = selection.get_selected_rows()

        for path in pathlist:
            treeiter = model.get_iter(path)

            item = model.get_value(treeiter, TREEITEM_OBJ)
            model.set_value(treeiter, TREEITEM_ICON, self.__retrieve_icon(item))
            model.set_value(treeiter, TREEITEM_NAME, self.__retrieve_name(item))

            if reload_children:
                self.__reload_children(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief expand selected items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def expand_selected_items(self):
        selection = self.__treeview.get_selection()
        model, pathlist = selection.get_selected_rows()

        for path in pathlist:
            self.__treeview.expand_row(path, False)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief remove selected items
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def remove_selected_items(self):

        # get selected items
        selection = self.__treeview.get_selection()
        model, pathlist = selection.get_selected_rows()

        # remove items
        treeiterlist = [model.get_iter(path) for path in pathlist]
        has_next = False

        for treeiter in treeiterlist:
            if model.iter_is_valid(treeiter):
                item = model.get_value(treeiter, TREEITEM_OBJ)
                self.__on_remove_item_handler(item)
                has_next = model.remove(treeiter)
            else:
                has_next = False

        # select next item
        if has_next:
            selection.select_iter(treeiter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle treeview->on-selection-changed
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_selection_changed(self, selection, *args):
        model, pathlist = selection.get_selected_rows()
        itemlist = []

        for path in pathlist:
            treeiter = model.get_iter(path)
            item = model.get_value(treeiter, TREEITEM_OBJ)
            itemlist.append(item)

        self.__on_selection_changed_handler(itemlist)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle treeview->test-expand-row event
    # @detail This function implements lazy evaluation. Every item inserted
    # receive a null child (obj = None). When row is expanded, three situations
    # arise:
    # 1. row has no child: object already loaded but no subitems were found
    # 2. row has null child: just a markplace. Object must be loaded
    # 3. row has valid child: object already loaded
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_test_expand_row(self, treeview, parent_iter, path, *args):
        model = treeview.get_model()
        parent = model.get_value(parent_iter, TREEITEM_OBJ)

        # check if item has placeholder
        has_placeholder = False
        child_iter = model.iter_children(parent_iter)

        if child_iter and model.get_value(child_iter, TREEITEM_OBJ) is None:
            has_placeholder = True

        # if item had a placeholder, load children
        if has_placeholder:
            self.__reload_children(parent_iter)

        return False

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief handle treeview->drag-data-received
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __on_drag_data_received(self, treeview, context, x, y, selection, info, utime, *args):
        model = treeview.get_model()
        accept_dnd = False

        # get mimetype
        target = selection.get_target()
        mimetype = target.name()

        # get dest_path and dest_pos (DND position)
        row_data = treeview.get_dest_row_at_pos(x, y)

        if row_data:
            dest_path, dest_pos = row_data

        else:
            dest_path, dest_pos = (len(model) - 1,), Gtk.TreeViewDropPosition.AFTER

        dest_iter = model.get_iter(dest_path)

        # get target_iter and target_pos (parent iter and child pos)
        if dest_pos == Gtk.TreeViewDropPosition.BEFORE:
            target_iter = model.iter_parent(dest_iter)
            target_pos = dest_path[-1]

        elif dest_pos in (Gtk.TreeViewDropPosition.INTO_OR_BEFORE, Gtk.TreeViewDropPosition.INTO_OR_AFTER):
            target_iter = dest_iter
            target_pos = model.iter_n_children(target_iter)

        elif dest_pos == Gtk.TreeViewDropPosition.AFTER:
            target_iter = model.iter_parent(dest_iter)
            target_pos = dest_path[-1] + 1

        else:
            mobius.core.logf('ERR invalid DND position: %s' % dest_pos)
            context.drop_reply(False, utime)  # reject DND
            return

        # handle reordering
        if mimetype == TREEITEM_MIMETYPE:
            if self.__reorder_treeitems(target_iter, target_pos):
                accept_dnd = True

        # handle file DND into treeview
        elif self.__on_file_dropped_handler:
            parent = None

            if target_iter:
                parent = model.get_value(target_iter, TREEITEM_OBJ)

            accept_dnd = self.__on_file_dropped_handler(parent, target_pos, mimetype, selection.get_data())
            if accept_dnd:
                self.__reload_children(target_iter)

        # accept or reject mimetype
        if accept_dnd:
            context.finish(True, False, utime)
            target_path = model.get_path(target_iter)
            self.__treeview.expand_row(target_path, False)

        else:
            Gdk.drop_reply(context, False, utime)  # reject DND

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief reorder treeitems after a DND
    # @param target_iter target iter
    # @param target_pos item position
    # @return True done/False otherwise
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __reorder_treeitems(self, target_iter, target_pos):
        selection = self.__treeview.get_selection()
        model, pathlist = selection.get_selected_rows()

        # get new parent item
        if target_iter:
            new_parent = model.get_value(target_iter, TREEITEM_OBJ)

        else:
            new_parent = None

        # reorder items
        for path in pathlist:
            treeiter = model.get_iter(path)
            item = model.get_value(treeiter, TREEITEM_OBJ)

            # get old parent item
            old_parent_iter = model.iter_parent(treeiter)

            if old_parent_iter:
                old_parent = model.get_value(old_parent_iter, TREEITEM_OBJ)

            else:
                old_parent = None

            # sanity check: cannot move item neither downwards descendant tree nor to itself
            if target_iter and (model.is_ancestor(treeiter, target_iter) or model.get_path(treeiter) == model.get_path(
                    target_iter)):
                return False

            # call "on-reordering-item" handler
            if not self.__on_reordering_item_handler(item, old_parent, new_parent, target_pos):
                return False

            # move item
            new_iter = model.insert(target_iter, target_pos)
            self.__copy_iter(model, treeiter, new_iter)
            model.remove(treeiter)

        return True

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve icon for item
    # @param item treeitem
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_icon(self, item):
        icon = self.__category_icons.get(item.category)

        if not icon:
            icon_data = self.__on_retrieve_icon_handler(item)

            if icon_data:
                image = mobius.ui.new_icon_from_data(icon_data, mobius.ui.icon.size_dnd)
                icon = image.get_ui_widget().get_pixbuf()
                self.__category_icons[item.category] = icon

        return icon

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve name for item
    # @param item treeitem
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_name(self, item):
        name = self.__on_retrieve_name_handler(item) or item.name
        return name

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve if item is expandable
    # @param item treeitem
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_is_expandable(self, item):
        is_expandable = self.__on_retrieve_is_expandable_handler(item)

        if is_expandable is None:
            if hasattr(item, 'get_child_count') and callable(item.get_child_count):
                is_expandable = item.get_child_count() > 0

            elif hasattr(item, 'children'):
                is_expandable = len(item.children) > 0

        return is_expandable

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief retrieve item children
    # @param item treeitem
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __retrieve_children(self, item):
        children = self.__on_retrieve_children_handler(item) or item.get_children()
        return children

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief reload treeitem
    # @param treeiter tree iterator
    # @description E,A,G,H,B -> A,B,C,D,E,F (e.g)
    #     1. G,H deleted (E,A,B)
    #     2. C,D,F appended (E,A,B,C,D,F)
    #     3. new_order = [1,2,3,4,0,5]
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __reload_children(self, parent_iter):
        model = self.__treeview.get_model()

        # get new children
        parent = model.get_value(parent_iter, TREEITEM_OBJ)
        new_children = self.__retrieve_children(parent)

        pos_dict = {}
        for pos, child in enumerate(new_children):
            pos_dict[id(child)] = pos

        # build reorder list
        ordered_list = list(a for a in range(len(pos_dict)))
        new_order = [-1] * len(pos_dict)
        child_iter = model.iter_children(parent_iter)
        counter = 0

        while child_iter:
            item = model.get_value(child_iter, TREEITEM_OBJ)
            item_id = id(item)

            pos = pos_dict.get(item_id, -1)

            if pos == -1:  # delete item
                has_next = model.remove(child_iter)

                if not has_next:
                    child_iter = None

            else:
                new_order[pos] = counter
                pos_dict[item_id] = -1
                counter += 1
                child_iter = model.iter_next(child_iter)

        # add remaining items
        for child in new_children:
            pos = pos_dict.get(id(child))

            if pos != -1:
                icon = self.__retrieve_icon(child)
                name = self.__retrieve_name(child)
                is_expandable = self.__retrieve_is_expandable(child)
                treeiter = model.append(parent_iter, (icon, name, child))

                # insert place holder if child is expandable
                if is_expandable:
                    model.append(treeiter)

                new_order[pos] = counter
                counter += 1

        # reorder entries
        if new_order != ordered_list:
            model.reorder(parent_iter, new_order)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief copy tree branch
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __copy_iter(self, model, source_iter, dest_iter):
        values = list(model[source_iter])
        model.set_row(dest_iter, values)

        for i in range(model.iter_n_children(source_iter)):
            child_source_iter = model.iter_nth_child(source_iter, i)
            child_dest_iter = model.append(parent=dest_iter)
            self.__copy_iter(model, child_source_iter, child_dest_iter)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add source mimetype to DND
    # @param mimetype
    # @param flags GTK flags
    # @param action GTK DND actions
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __add_source_dnd_target(self, mimetype, flags, action):
        self.__dnd_source_targets.append(Gtk.TargetEntry.new(mimetype, flags, self.__dnd_source_next_id))
        self.__dnd_source_next_id += 1
        self.__treeview.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, self.__dnd_source_targets, action)

    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    # @brief add dest mimetype to DND
    # @param mimetype
    # @param flags GTK flags
    # @param actions GTK DND actions
    # =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    def __add_dest_dnd_target(self, mimetype, flags, actions):
        self.__dnd_dest_targets.append(Gtk.TargetEntry.new(mimetype, flags, self.__dnd_dest_next_id))
        self.__dnd_dest_next_id += 1
        self.__treeview.enable_model_drag_dest(self.__dnd_dest_targets, actions)
