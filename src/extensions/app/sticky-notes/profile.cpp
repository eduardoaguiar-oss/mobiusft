// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008-2026 Eduardo Aguiar
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "profile.hpp"
#include <mobius/core/io/line_reader.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/mediator.hpp>
#include <mobius/core/string_functions.hpp>
#include <mobius/framework/utils.hpp>
#include "file_plum_sqlite.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// References:
// - https://medium.com/@two06/reading-windows-sticky-notes-5468985eff4d
// - https://github.com/iamhunggy/StickyParser
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace mobius::extension::app::sticky_notes
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse message line
// @param line Line to parse
// @return Parsed elements
//
// Raw text escape sequences:
// - \b Start bold text
// - \b0 End bold text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::map>
parse_line (const std::string &text)
{
    std::vector<mobius::core::pod::map> elements;

    mobius::core::log log (__FILE__, __FUNCTION__);
    size_t i = 0;

    while (i < text.size ())
    {
        // Search for next escape sequence
        size_t pos = text.find ('\\', i);

        // Generate text element, if any
        size_t end_pos = (pos == std::string::npos) ? text.size () : pos;

        if (i < end_pos)
        {
            elements.emplace_back (
                mobius::core::pod::map {
                    {"type", "text"},
                    {"text", text.substr (i, end_pos - i)}
                }
            );
            i = end_pos;
        }

        // Handle escape command
        if (pos != std::string::npos)
        {
            // Parse command
            size_t cmd_start = pos + 1;
            size_t cmd_end = text.find_first_of (" \\", cmd_start);
            std::string command = text.substr (
                cmd_start, cmd_end == std::string::npos ? std::string::npos
                                                        : cmd_end - cmd_start
            );

            if (command == "\\b")
                elements.emplace_back (
                    mobius::core::pod::map {{"type", "start/b"}}
                );

            else if (command == "\\b0")
                elements.emplace_back (
                    mobius::core::pod::map {{"type", "end/b"}}
                );

            else
                log.development (__LINE__, "Unhandled command: \\" + command);

            i = (cmd_end == std::string::npos) ? text.size () : cmd_end;
        }
    }

    return elements;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse raw text into blocks structures
// @param raw_text Raw text
// @return Vector of elements
// Each line starting with "\id=" indicates a new block.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::map>
parse_blocks (const std::string &raw_text)
{
    std::vector<mobius::core::pod::map> elements;

    mobius::core::io::line_reader lr (raw_text);
    std::string line;

    while (lr.read (line))
    {
        if (line.starts_with (R"(\id=)"))
        {
            // If line has more than just the ID, parse it
            if (line.size () > 41)
            {
                auto line_elements = parse_line (line.substr (41));

                std::copy (
                    line_elements.begin (), line_elements.end (),
                    std::back_inserter (elements)
                );
            }

            // Add newline element
            elements.emplace_back (
                mobius::core::pod::map {{"type", "text"}, {"text", "\n"}}
            );
        }
    }

    return elements;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set folder
// @param f Folder
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_set_folder (const mobius::core::io::folder &f)
{
    if (folder_ || !f)
        return;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Set data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_ = f;
    last_modified_time_ = f.get_modification_time ();
    creation_time_ = f.get_creation_time ();
    username_ = mobius::framework::get_username_from_path (f.get_path ());

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Emit sampling_folder event
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    mobius::core::emit (
        "sampling_folder", std::string ("app.sticky_notes.profiles"), f
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Update last modified time based on file
// @param f File
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::_update_mtime (const mobius::core::io::file &f)
{
    if (!f)
        return;

    if (!last_modified_time_ ||
        f.get_modification_time () > last_modified_time_)
        last_modified_time_ = f.get_modification_time ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add plum.sqlite file to profile
// @param f plum.sqlite file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_plum_sqlite_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        file_plum_sqlite fp (f.new_reader ());

        if (!fp)
            return;

        log.info (__LINE__, "File decoded [plum.sqlite]: " + f.get_path ());

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Add notes
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        for (const auto &nt : fp.get_notes ())
        {
            note n;
            n.creation_time = nt.created_at;
            n.last_modification_time = nt.updated_at;
            n.body = parse_blocks (nt.text);

            // Metadata
            n.metadata.set ("record_idx", nt.idx);
            n.metadata.set ("change_key", nt.change_key);
            n.metadata.set (
                "creation_note_id_anchor", nt.creation_note_id_anchor
            );
            n.metadata.set ("deleted_at", nt.deleted_at);
            n.metadata.set ("id", nt.id);
            n.metadata.set ("is_always_on_top", nt.is_always_on_top);
            n.metadata.set ("is_future_note", nt.is_future_note);
            n.metadata.set ("is_open", nt.is_open);
            n.metadata.set (
                "is_remote_data_invalid", nt.is_remote_data_invalid
            );
            n.metadata.set ("last_server_version", nt.last_server_version);
            n.metadata.set ("parent_id", nt.parent_id);
            n.metadata.set ("pending_insights_scan", nt.pending_insights_scan);
            n.metadata.set ("remote_id", nt.remote_id);
            n.metadata.set ("remote_schema_version", nt.remote_schema_version);
            n.metadata.set ("theme", nt.theme);
            n.metadata.set ("type", nt.type);
            n.metadata.set ("window_position", nt.window_position);

            // Add note to the list
            notes_.emplace_back (std::move (n));
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.sticky_notes.plum_sqlite"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add Sticky Notes .snt file to profile
// @param f Sticky Notes .snt file
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
profile::add_sticky_notes_snt_file (const mobius::core::io::file &f)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Decode file
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        /*file_sticky_notes_snt fp (f.new_reader ());

        if (!fp)
            return;

        log.info (__LINE__, "File decoded [StickyNotes.snt]: " + f.get_path ());*/

        _set_folder (f.get_parent ());
        _update_mtime (f);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Emit sampling_file event
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        mobius::core::emit (
            "sampling_file", std::string ("app.sticky_notes.stickynotes_snt"),
            f.new_reader ()
        );
    }
    catch (const std::exception &e)
    {
        log.warning (
            __LINE__, std::string (e.what ()) + " (file: " + f.get_path () + ")"
        );
    }
}

} // namespace mobius::extension::app::sticky_notes
