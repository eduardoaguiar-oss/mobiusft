#ifndef MOBIUS_CORE_IO_LOCAL_FOLDER_IMPL_HPP
#define MOBIUS_CORE_IO_LOCAL_FOLDER_IMPL_HPP

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C)
// 2008-2026
// Eduardo Aguiar
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
#include <mobius/core/io/folder_impl_base.hpp>
#include <vector>

namespace mobius::core::io::local
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief local folder implementation class
// @author Eduardo Aguiar
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class folder_impl : public folder_impl_base
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_impl (const std::string &);
    folder_impl (const folder_impl &) = delete;
    folder_impl (folder_impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    folder_impl &operator= (const folder_impl &) = delete;
    folder_impl &operator= (folder_impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool exists () const final;
    bool is_deleted () const final;
    bool is_reallocated () const final;
    bool is_hidden () const final;
    std::string get_name () const final;
    std::string get_short_name () const final;
    inode_type get_inode () const final;
    size_type get_size () const final;
    user_id_type get_user_id () const final;
    std::string get_user_name () const final;
    group_id_type get_group_id () const final;
    std::string get_group_name () const final;
    permission_type get_permissions () const final;
    mobius::core::datetime::datetime get_access_time () const final;
    mobius::core::datetime::datetime get_modification_time () const final;
    mobius::core::datetime::datetime get_metadata_time () const final;
    mobius::core::datetime::datetime get_creation_time () const final;
    mobius::core::datetime::datetime get_deletion_time () const final;
    mobius::core::datetime::datetime get_backup_time () const final;
    folder_type get_parent () const final;
    std::vector<entry> get_children () const final;
    void create () final;
    void clear () final;
    void reload () final;
    void remove () final;
    void rename (const std::string &) final;
    bool move (folder_type) final;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is valid
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept final
    {
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if object is browseable
    // @return true/false
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    bool
    is_browseable () const final
    {
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set name
    // @param name Name
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_name (const std::string &name) final
    {
        name_ = name;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get path
    // @return Path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::string
    get_path () const final
    {
        return given_path_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Set path
    // @param path Path
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    set_path (const std::string &path) final
    {
        given_path_ = path;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get streams
    // @return Streams
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<stream_type>
    get_streams () const final
    {
        return {}; // local folders have no streams
    }

  private:
    // @brief Path
    std::string path_;

    // @brief Given path. Path given by user and only used by set/get_path
    // functions
    std::string given_path_;

    // @brief Folder name
    std::string name_;

    // @brief Folder exists flag
    mutable bool exists_;

    // @brief I-node
    mutable inode_type inode_;

    // @brief Size in bytes
    mutable size_type size_;

    // @brief User id
    mutable user_id_type user_id_;

    // @brief User name
    mutable std::string user_name_;

    // @brief Group id
    mutable group_id_type group_id_;

    // @brief Group name
    mutable std::string group_name_;

    // @brief RWX permissions
    mutable permission_type permissions_;

    // @brief Last access date/time
    mutable mobius::core::datetime::datetime access_time_;

    // @brief Last modification date/time
    mutable mobius::core::datetime::datetime modification_time_;

    // @brief Last metadata modification date/time
    mutable mobius::core::datetime::datetime metadata_time_;

    // @brief Stat loaded flag
    mutable bool flag_stat_loaded_ = false;

    // @brief Children loaded flag
    mutable bool flag_children_loaded_ = false;

    // @brief Children
     mutable std::vector<entry> children_;

  private:
    void _load_stat () const;
    void _load_children () const;
};

} // namespace mobius::core::io::local

#endif
