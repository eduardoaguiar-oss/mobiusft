// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mobius Forensic Toolkit
// Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025 Eduardo Aguiar
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
#include "element.h"
#include <mobius/exception.inc>
#include <mobius/string_functions.h>
#include <algorithm>
#include <stdexcept>

#include <iostream>

namespace mobius::decoder::xml
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief element implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class element::impl
{
public:

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Constructors
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  explicit impl (xmlNode *);
  impl (const impl&) = delete;
  impl (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Operators
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  impl& operator= (const impl&) = delete;
  impl& operator= (impl&&) = delete;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get element name
  // @return Name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_name () const
  {
    return name_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set element name
  // @param name Name
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_name (const std::string& name)
  {
    name_ = name;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set element path
  // @param path Path
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_path (const std::string& path)
  {
    path_ = path;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get content
  // @return Content
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::string
  get_content () const
  {
    return content_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set content
  // @param content Content
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_content (const std::string& content)
  {
    content_ = content;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set property
  // @param name Name
  // @param value Value
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_property (const std::string& name, const std::string& value)
  {
    properties_[name] = value;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get properties
  // @return Properties
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::map <std::string,std::string>
  get_properties () const
  {
    return properties_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get parent element
  // @return Parent element, if any
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  element
  get_parent () const
  {
    return parent_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Set parent
  // @param parent Parent
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  set_parent (const element& parent)
  {
    parent_ = parent;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Add child
  // @param child Child element
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  void
  add_child (const element& child)
  {
    children_.push_back (child);
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // @brief Get children
  // @return Children elements
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  std::vector <element>
  get_children () const
  {
    return children_;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Prototypes
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  bool has_property (const std::string&) const;
  std::string get_property (const std::string&) const;
  std::string get_path () const;

private:
  // @brief Name
  std::string name_;

  // @brief Full path
  mutable std::string path_;

  // @brief Content (text + CDATA)
  std::string content_;

  // @brief Properties
  std::map <std::string, std::string> properties_;

  // @brief Parent element
  element parent_;

  // @brief Child elements
  std::vector <element> children_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param node libxml2 node structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
element::impl::impl (xmlNode *node)
{
  if (node->name)
    set_name (reinterpret_cast <const char *> (node->name));

  // get properties
  auto attr = node->properties;

  while (attr)
    {
      xmlChar* value = xmlNodeListGetString (node->doc, attr->children, 1);
      if (value)
        {
          set_property (reinterpret_cast <const char *> (attr->name), reinterpret_cast <const char *> (value));
          xmlFree (value);
        }

      attr = attr->next;
    }

  // process child nodes
  std::string content;

  auto child_node = node->children;

  while (child_node)
    {
      if (child_node->type == XML_ELEMENT_NODE)
          add_child (element (child_node));

      else if (child_node->type == XML_TEXT_NODE)
        content += reinterpret_cast <const char *> (XML_GET_CONTENT (child_node));

      else if (child_node->type == XML_CDATA_SECTION_NODE)
        content += reinterpret_cast <const char *> (XML_GET_CONTENT (child_node));

      child_node = child_node->next;
    }

  // set content
  set_content (content);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if element has a given property
// @param name Property name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
element::impl::has_property (const std::string& name) const
{
  return properties_.find (name) != properties_.end ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get property value
// @param name Property name
// @return Property value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::impl::get_property (const std::string& name) const
{
  std::string value;

  auto iter = properties_.find (name);

  if (iter != properties_.end ())
    value = iter->second;

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get element full path
// @return Full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::impl::get_path () const
{
  if (path_.empty ())
    {
      if (parent_)
        path_ = parent_.get_path ();

      path_ += '/' + name_;
    }

  return path_;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param node libxml2 node structure
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
element::element (xmlNode *node)
  : impl_ (std::make_shared <impl> (node))
{
  // set children parent
  for (auto& child : get_children ())
    child.impl_->set_parent (*this);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get element name
// @return Name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::get_name () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_name ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get element full path
// @return Full path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::get_path () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_path ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get element content
// @return Content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::get_content () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_content ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if element has a given property
// @param name Property name
// @return true/false
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
element::has_property (const std::string& name) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->has_property (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get property value
// @param name Property name
// @return Property value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::get_property (const std::string& name) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_property (name);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get property by relative path
// @param path Relatvie path
// @return Property value
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
element::get_property_by_path (const std::string& path) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  std::string value;
  std::string::size_type pos = path.rfind ('/');

  if (pos == std::string::npos)
    value = impl_->get_property (path);

  else
    {
      auto child = get_child_by_path (path.substr (0, pos));

      if (child)
        value = child.get_property (path.substr (pos + 1));
    }

  return value;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get properties
// @return Properties
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::map <std::string,std::string>
element::get_properties () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_properties ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent element
// @return Parent element, if any
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
element
element::get_parent () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_parent ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get first child element by relative path
// @param path Relative path
// @return First element that matches a given path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
element
element::get_child_by_path (const std::string& path) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  element e;

  auto children = get_children_by_path (path);

  if (!children.empty ())
    e = children[0];

  return e;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children
// @return Children elements
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <element>
element::get_children () const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  return impl_->get_children ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children elements by relative path
// @param path Relative path
// @return Children elements
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <element>
element::get_children_by_path (const std::string& path) const
{
  if (!impl_)
    throw std::runtime_error (MOBIUS_EXCEPTION_MSG ("element object is null"));

  std::string::size_type pos = 0;

  // seek each pattern segment, limited by '/'
  std::vector <element> elements = {*this};
  std::string::size_type end = path.find ('/', pos);

  while (pos != std::string::npos && !elements.empty ())
    {
      // get next segment from path
      std::string segment;

      if (end == std::string::npos)
        {
          segment = path.substr (pos);
          pos = std::string::npos;
        }

      else
        {
          segment = path.substr (pos, end - pos);
          pos = end + 1;
          end = path.find ('/', pos);
        }

      // create new vector with all elements whose name matches the sub path
      std::vector <element> tmp_elements;

      for (const auto& e : elements)
        {
          auto children = e.get_children ();

          std::copy_if (
            children.begin (),
            children.end (),
            std::back_inserter (tmp_elements),
            [&segment](const auto& c){ return mobius::string::fnmatch (segment, c.get_name ()); }
          );
        }

      // overwrite the candidate vector
      elements = tmp_elements;
    }

  // return elements found
  return elements;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get children by name
// @param name Child name
// @return Children elements
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector <element>
element::get_children_by_name (const std::string& name) const
{
  std::vector <element> children = get_children ();
  std::vector <element> selected_children;

  std::copy_if (
    children.begin (),
    children.end (),
    std::back_inserter (selected_children),
    [name](const element& e){ return e.get_name () == name; }
  );

  return selected_children;
}

} // namespace mobius::decoder::xml


