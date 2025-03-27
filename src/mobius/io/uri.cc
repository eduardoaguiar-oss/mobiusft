// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "uri.h"
#include "path.h"
#include <mobius/charset.h>
#include <mobius/string_functions.h>
#include <mobius/exception.inc>
#include <cstdlib>
#include <regex>
#include <stdexcept>

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Syntax:
//
// 2.3. unreserved
//   A-Z, a-z, 0-9, "-", ".", "_", "~"
//
// 3.1. scheme
//   case insensitive
//
// 3.2. authority
//   [ userinfo "@" ] host [ ":" port] (3.2.1)
//   userinfo = (unreserved | pct-encoded | sub-delims | ":")
//      sometimes "user:password" (deprecated)
//   host = IP-literal | IPv4address | reg-name.
//     If it can be an IPv4 address then it is, instead of a reg-name (3.2.2)
//     case insensitive (3.2.2)
//     host allows '[' and ']' (IP-literal)
//     reg-name = (unreserved | pct-encoded | sub-delims)
//   port = (digit)*
//
// 3.3 path
//   dot-segments = "." and ".."
//   pchar = unreserved | pct-encoded | sub-delims | ":" | "@"
//   (pchar | "/")
//
// 3.4. query
//   (pchar | "/" | "?")*
//
// 3.5. fragment
//   (pchar | "/" | "?")*
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
namespace
{
// @brief RFC 3986 - section 2.3
static constexpr const char *UNRESERVED_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

// @brief RFC 3986 - section 3.1
static constexpr const char *ALLOWED_SCHEME = UNRESERVED_CHARS;

// @brief RFC 3986 - section 3.2
static constexpr const char *ALLOWED_AUTHORITY = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=:@[]";

// @brief RFC 3986 - section 3.3
static constexpr const char *ALLOWED_PATH = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=:@/";

// @brief RFC 3986 - section 3.4
static constexpr const char *ALLOWED_QUERY = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=:@/?";

// @brief RFC 3986 - section 3.5
static constexpr const char *ALLOWED_FRAGMENT = ALLOWED_QUERY;

// @brief RFC 3986 - appendix B - URI regular expression
static constexpr const char *URI_PATTERN = "(([A-Z][A-Z0-9+.-]*):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?";
static std::regex URI_REGEX (URI_PATTERN, std::regex::icase | std::regex::extended);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief merge two URI paths
// @param base base URI
// @param rel relative URI
// @return merged path
// @see RFC 3986 - section 5.2.3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
merge_paths (const mobius::io::uri& base, const mobius::io::uri& rel)
{
  std::string path;

  if (!base.get_authority ().empty () && base.get_path ().empty ())
    path = '/' + rel.get_path ();

  else
    {
      std::string::size_type pos = base.get_path ().rfind ('/');

      if (pos == std::string::npos)
        path = rel.get_path ();

      else
        path = base.get_path ().substr (0, pos) + '/' + rel.get_path ();
    }

  return path;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief remove dot segments
// @param path URI path
// @return path without "." and ".." dot segments
// @see RFC 3986 - section 5.2.4
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
remove_dot_segments (const std::string& path)
{
  mobius::io::path p (path);
  return to_string (p);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Encode a string with %XX escape sequences
// @param s Encoded string
// @param allowed Allowed chars (not converted)
// @return Decoded string
// @see RFC 3986 - section 2.1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_encode_triplets (const std::string& s, const std::string& allowed)
{
  std::string tmp;
  char pct_encoded[4] = {'%', 0, 0, 0};

  for (auto c : s)
    {
      if (allowed.find (c) == std::string::npos)
        {
          sprintf (pct_encoded + 1, "%02X", c);
          tmp += pct_encoded;
        }
      else
        tmp += c;
    }

  return tmp;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Decode a string with %XX escape sequences
// @param s Encoded string
// @return Decoded string
// @see RFC 3986 - section 2.1
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::string
_decode_triplets (const std::string& s)
{
  std::string::size_type begin = 0;
  std::string::size_type pos = s.find ('%', begin);
  std::string::size_type length = s.length ();
  std::string tmp;

  while (pos != std::string::npos)
    {
      tmp += s.substr (begin, pos - begin);

      if (pos + 2 < length)
        {
          std::string v = s.substr (pos + 1, 2);
          tmp += char (strtol (v.c_str (), nullptr, 16));
          begin = pos + 3;
        }
      else
        {
          tmp += '%';
          begin = pos + 1;
        }

      pos = s.find ('%', begin);
    }

  tmp += s.substr (begin);

  return tmp;
}

} // namespace

namespace mobius::io
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Build URI from string
// @param value URI as string
// @see RFC 3986 - section 3
// 2. Parsing must be called before decoding the percent-encoded triplets.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri::uri (const std::string& value)
{
  // Parse URI
  std::smatch match;

  if (!std::regex_match (value, match, URI_REGEX))
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("invalid URI"));

  // Set attributes
  scheme_ = mobius::string::tolower (_decode_triplets (match[2].str ()));
  query_ = match[7].str ();
  fragment_ = match[9].str ();

  // Parse authority
  _set_authority (_decode_triplets (match[4].str ()));

  // If it is not a relative URI, remove dot segments - RFC 3986 - section 6.2.2.3
  std::string path = match[5].str ();

  if ((!scheme_.empty () || !host_.empty ()) && !path.empty ())
    path = remove_dot_segments (path);

  path_ = path;

  _normalize ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Build URI from parts
// @param scheme Scheme
// @param username User name
// @param password Password
// @param host Host
// @param port Port
// @param path Path
// @param query Query
// @param fragment Fragment
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri::uri (
  const std::string& scheme,
  const std::string& username,
  const std::string& password,
  const std::string& host,
  const std::string& port,
  const std::string& path,
  const std::string& query,
  const std::string& fragment
)
 : scheme_ (scheme),
   username_ (username),
   password_ (password),
   host_ (host),
   port_ (port),
   path_ (_encode_triplets (path, ALLOWED_PATH)),
   query_ (_encode_triplets (query, ALLOWED_QUERY)),
   fragment_ (_encode_triplets (fragment, ALLOWED_FRAGMENT))
{
  _normalize ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Rebuild URI from its parts
// @see RFC 3986 - section 5.3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
uri::_normalize ()
{
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Normalize authority
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  authority_.clear ();

  if (!username_.empty ())
    {
      authority_ += username_;

      if (!password_.empty ())
        {
          authority_ += ':';
          authority_ += password_;
        }

      authority_ += '@';
    }

  authority_ += host_;

  if (!port_.empty ())
    {
      authority_ += ':';
      authority_ += port_;
    }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Normalize value
  // @see RFC 3986 - section 5.3
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  value_.clear ();

  if (!scheme_.empty ())
    {
      value_ += _encode_triplets (scheme_, ALLOWED_SCHEME);
      value_ += ':';
    }

  if (!authority_.empty () || scheme_ == "file")
    {
      value_ += "//";
      value_ += _encode_triplets (authority_, ALLOWED_AUTHORITY);
    }

  value_ += path_;

  if (!query_.empty ())
    {
      value_ += "?";
      value_ += query_;
    }

  if (!fragment_.empty ())
    {
      value_ += "#";
      value_ += fragment_;
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Set authority
// @param authority authority
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
uri::_set_authority (const std::string& value)
{
  std::string username;
  std::string password;
  std::string host;
  std::string port;

  // [username[:password]@]
  std::string::size_type pos = value.find ('@');
  std::string::size_type pos2;

  if (pos == std::string::npos)
    pos = 0;

  else
    {
      pos2 = value.find (':');

      if (pos2 == std::string::npos)
        username = value.substr (0, pos);

      else
        {
          username = value.substr (0, pos2);
          password = value.substr (pos2 + 1, pos - pos2 - 1);
        }

      pos++;
    }

  // host[:port]
  if (pos < value.length ())
    {
      if (value[pos] == '[')    // IPv6 | IPfuture
        {
          pos2 = value.find (']');

          if (pos2 == std::string::npos)        // unmatched ']'
            pos = value.length ();

          else
            {
              host = value.substr (pos, pos2 - pos + 1);
              pos = pos2 + 1;

              if (pos < value.length () && value[pos] == ':')
                port = value.substr (pos + 1);
            }
        }

      else                      // IPv4 | hostname
        {
          pos2 = value.find (':', pos);

          if (pos2 == std::string::npos)
            host = value.substr (pos);

          else
            {
              host = value.substr (pos, pos2 - pos);
              port = value.substr (pos2 + 1);
            }
        }
    }

  // set values
  username_ = _decode_triplets (username);
  password_ = _decode_triplets (password);
  host_ = mobius::string::tolower (_decode_triplets (host));
  port_ = _decode_triplets (port);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get path
// @param encoding String encoding
// @return URI path
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
uri::get_path (const std::string& encoding) const
{
  if (encoding.empty ())
    return path_;

  return conv_charset_to_utf8 (_decode_triplets (path_), encoding);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get query string
// @param encoding String encoding
// @return URI query string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
uri::get_query (const std::string& encoding) const
{
  if (encoding.empty ())
    return query_;

  return conv_charset_to_utf8 (_decode_triplets (query_), encoding);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get fragment
// @param encoding String encoding
// @return URI fragment
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
uri::get_fragment (const std::string& encoding) const
{
  if (encoding.empty ())
    return fragment_;

  return conv_charset_to_utf8 (_decode_triplets (fragment_), encoding);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get parent URI
// @return Parent URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
uri::get_parent () const
{
  uri parent;

  auto pos = value_.rfind ('/');

  if (pos != std::string::npos)
    parent = uri (value_.substr (0, pos));

  return parent;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get sibling URI by name
// @param filename Sibling file name
// @return Sibling URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
uri::get_sibling_by_name (const std::string& filename) const
{
  path p (path_);
  path cpath = p.get_sibling_by_name (filename);

  return uri (scheme_, username_, password_, host_, port_, cpath.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get sibling URI by extension
// @param ext Sibling extension
// @return Sibling URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
uri::get_sibling_by_extension (const std::string& ext) const
{
  path p (path_);
  path cpath = p.get_sibling_by_extension (ext);

  return uri (scheme_, username_, password_, host_, port_, cpath.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child URI by name
// @param name Child name
// @return Child URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
uri::get_child_by_name (const std::string& name) const
{
  path p (path_);
  path cpath = p.get_child_by_name (name);

  return uri (scheme_, username_, password_, host_, port_, cpath.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get child by path
// @param subpath Sub path
// @return Child URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
uri::get_child_by_path (const std::string& subpath) const
{
  std::string path = path_;

  if (path.empty () || path[path.length () - 1] != '/')
    path += '/';

  return new_uri_from_path (path + subpath);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filename from URI
// @param encoding String encoding
// @return Filename
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
uri::get_filename (const std::string& encoding) const
{
  std::string::size_type pos = path_.rfind ('/');
  std::string value;

  if (pos != std::string::npos)
    value = path_.substr (pos + 1);

  if (encoding.empty ())
    return value;

  return conv_charset_to_utf8 (_decode_triplets (value), encoding);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get filename extension from URI
// @return extension, if any
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
uri::get_extension () const
{
  auto filename = get_filename ();
  std::string::size_type pos = filename.rfind ('.');
  std::string value;

  if (pos != std::string::npos)
    value = filename.substr (pos + 1);

  return value;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get port number
// @return Port number
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::uint16_t
uri::get_port_number () const
{
  std::uint16_t port_number = 0;

  if (!port_.empty ())
    port_number = strtol (port_.c_str (), nullptr, 10);

  return port_number;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Resolve an URI reference
// @param base base URI (not relative URI)
// @param rel relative URI
// @return target URI joining base and relative URIs
// @see RFC 3986 - section 5.2
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
join (const uri& base, const uri& rel)
{
  std::string scheme;
  std::string username;
  std::string password;
  std::string host;
  std::string port;
  std::string path;
  std::string query;
  std::string fragment;

  // section 5.2.1 - "base" must be an URI
  if (base.get_scheme ().empty ())
    throw std::invalid_argument (MOBIUS_EXCEPTION_MSG ("base URI has no scheme"));

  // join
  fragment = rel.get_fragment ();

  if (!rel.get_scheme ().empty () && rel.get_scheme () != base.get_scheme ())
    {
      scheme = rel.get_scheme ();
      username = rel.get_username ();
      password = rel.get_password ();
      host = rel.get_host ();
      port = rel.get_port ();
      path = remove_dot_segments (rel.get_path ());
      query = rel.get_query ();
    }

  else
    {
      scheme = base.get_scheme ();

      if (!rel.get_authority ().empty ())
        {
          username = rel.get_username ();
          password = rel.get_password ();
          host = rel.get_host ();
          port = rel.get_port ();
          path = rel.get_path ();
          query = rel.get_query ();
        }
      else
        {
          username = base.get_username ();
          password = base.get_password ();
          host = base.get_host ();
          port = base.get_port ();

          if (rel.get_path ().empty ())
            {
              path = base.get_path ();
              query = rel.get_query ().empty () ? base.get_query () : rel.get_query ();
            }
          else
            {
              query = rel.get_query ();

              if (!rel.get_path ().empty () && rel.get_path ()[0] == '/')
                path = remove_dot_segments (rel.get_path ());

              else
                path = remove_dot_segments (merge_paths (base, rel));
            }
        }
    }

  return uri (scheme, username, password, host, port, path, query, fragment);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if two URI objects are equal
// @param lhs URI object
// @param rhs URI object
// @return true if lhs == rhs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator== (const uri& lhs, const uri& rhs)
{
  return lhs.get_value () == rhs.get_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Test if two URI objects are different
// @param lhs URI object
// @param rhs URI object
// @return true if lhs != rhs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator!= (const uri& lhs, const uri& rhs)
{
  return ! (lhs.get_value () == rhs.get_value ());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check whether one URI object is less than another one
// @param lhs URI object
// @param rhs URI object
// @return true if lhs < rhs
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
operator< (const uri& lhs, const uri& rhs)
{
  return lhs.get_value () < rhs.get_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check whether two URI objects are equal
// @param lhs URI object
// @param rhs URI object
// @return true if lhs == rhs
// @see RFC 3986 - section 4.4
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool
is_same_document (const uri& lhs, const uri& rhs)
{
  return lhs.get_scheme () == rhs.get_scheme () &&
         lhs.get_authority () == rhs.get_authority () &&
         lhs.get_path () == rhs.get_path () &&
         lhs.get_query () == rhs.get_query ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert path into URI
// @param path path
// @return URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uri
new_uri_from_path (const std::string& path)
{
  return uri ("file", "", "", "", "", path);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert URI to string
// @param u URI object
// @return String representation of URI
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
to_string (const uri& u)
{
  return u.get_value ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Write URI representation to std::ostream
// @param stream ostream reference
// @param u URI object
// @return reference to ostream
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::ostream&
operator<< (std::ostream& stream, const uri& u)
{
  stream << u.get_value ();

  return stream;
}

} // namespace mobius::io


