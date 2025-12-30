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
#include "message_parser.hpp"
#include <mobius/core/io/bytearray_io.hpp>
#include <mobius/core/log.hpp>
#include <mobius/core/string_functions.hpp>
#include <format>
#include <unordered_map>
#include <unordered_set>

namespace
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Emoji representation as Unicode char
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, std::string> EMOJI_CHARS = {
    {"angel", "👼"},        {"anger", "🤯"},
    {"angry", "😠"},        {"bandit", "😈"},
    {"banned", "🚫"},       {"bear", "🧸"},
    {"bike", "🚴"},         {"bombed", "💣"},
    {"blush", "😳"},        {"bomb", "💣"},
    {"brokenheart", "💔"},  {"brb", "🕒"},
    {"call", "📞"},         {"camera", "📷"},
    {"catface", "🐱"},      {"champagne", "🍾"},
    {"chocolate", "🍫"},    {"clap", "👏"},
    {"clapping", "👏"},     {"clownface", "🤡"},
    {"confused", "😕"},     {"coolguy", "😎"},
    {"crying", "😢"},       {"danceparty", "🕺"},
    {"devilface", "😈"},    {"dogface", "🐶"},
    {"dollar", "💵"},       {"dollars", "💵"},
    {"dizzy", "😵"},        {"cake", "🎂"},
    {"cash", "💰"},         {"cat", "🐈"},
    {"claps", "👏"},        {"clown", "🤡"},
    {"coffee_cup", "☕"},   {"coffee", "☕"},
    {"cool", "😎"},         {"cry", "😢"},
    {"cwl", "😂"},          {"dance", "🕺"},
    {"devil", "😈"},        {"dog", "🐕"},
    {"drink", "🍸"},        {"drunk", "🥴"},
    {"dull", "🙄"},         {"emo", "🤯"},
    {"envy", "😒"},         {"explode", "💣"},
    {"explosion", "💣"},    {"facepalm", "🤦"},
    {"fear", "😨"},         {"fingerscrossed", "🤞"},
    {"ghost", "👻"},        {"gift", "🎁"},
    {"glasses", "🕶"},      {"giggle", "🤭"},
    {"handsinair", "🙌"},   {"happytears", "😂"},
    {"happy", "🙂"},        {"heart", "❤"},
    {"hearteyes", "😍"},    {"heidy", "🐿"},
    {"hi", "👋"},           {"highfive", "🖐"},
    {"hooray", "🎉"},       {"hug", "🧸"},
    {"idea", "💡"},         {"innocent", "😇"},
    {"inlove", "🥰"},       {"joy", "😂"},
    {"kissing", "😗"},      {"kiss", "😗"},
    {"ladyvamp", "🧛"},     {"ladyvampire", "🧛"},
    {"laughing", "😆"},     {"laugh", "😃"},
    {"lips", "💋"},         {"lipssealed", "🤐"},
    {"loudlycrying", "😭"}, {"makeup", "💄"},
    {"monocle", "🧐"},      {"mail", "✉"},
    {"meh", "😑"},          {"money", "💰"},
    {"mmm", "😋"},          {"monkey", "🐒"},
    {"mooning", "🌝"},      {"muscle", "💪"},
    {"muscleman", "💪"},    {"music", "🎶"},
    {"nerd", "🤓"},         {"nerdy", "🤓"},
    {"ninja", "🥷"},        {"no", "👎"},
    {"nod", "👍"},          {"notworthy", "🙄"},
    {"okeydokey", "👌"},    {"ok", "👌"},
    {"party", "🥳"},        {"phone", "📱"},
    {"pig", "🐖"},          {"poop", "💩"},
    {"pray", "🙏"},         {"pizza", "🍕"},
    {"praying", "🙏"},      {"puke", "🤮"},
    {"pumpkin", "🎃"},      {"rain", "🌧"},
    {"rofl", "🤣"},         {"rocket", "🚀"},
    {"rock", "🤘"},         {"sadface", "😞"},
    {"sadcry", "😭"},       {"sad", "😧"},
    {"sadness", "😢"},      {"satisfied", "😌"},
    {"sarcastic", "😒"},    {"scared", "😱"},
    {"scream", "😱"},       {"shocked", "😲"},
    {"shake", "🤝"},        {"shy", "😊"},
    {"sick", "🤢"},         {"sleeping", "😴"},
    {"skype", "💬"},        {"skull", "💀"},
    {"sleepy", "😪"},       {"smile", "😄"},
    {"smirk", "😏"},        {"speechless", "😐"},
    {"squirrel", "🐿"},     {"star", "⭐"},
    {"stareyes", "🤩"},     {"sun", "🌞"},
    {"surprised", "😲"},    {"swear", "🤬"},
    {"stop", "✋"},         {"sweat", "😓"},
    {"think", "🤔"},        {"time", "⏲"},
    {"tongueout", "😛"},    {"tmi", "🤭"},
    {"toothygrin", "😁"},   {"tongue", "😛"},
    {"thumbsdown", "👎"},   {"thumbsup", "👍"},
    {"tired", "😫"},        {"unhappy", "☹"},
    {"unamused", "😒"},     {"vampire", "🧛"},
    {"victory", "✌"},       {"waiting", "⏳"},
    {"wavehand", "👋"},     {"wave", "👋"},
    {"weary", "😩"},        {"whistle", "😗"},
    {"winkey", "😉"},       {"wink", "😉"},
    {"woozy", "🥴"},        {"wasntme", "🙄"},
    {"wave", "🌊"},         {"whew", "😮‍💨"},
    {"worry", "😟"},        {"wonder", "🤔"},
    {"xd", "😆"},           {"yawn", "🥱"},
    {"yes", "👍"},          {"yoga", "🧘"},
    {"zombie", "🧟"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Flags representation as Unicode char
// @see ISO-3166
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, std::string> FLAG_CHARS = {
    {"ad", "🇦🇩"}, {"ae", "🇦🇪"}, {"af", "🇦🇫"}, {"ag", "🇦🇬"}, {"ai", "🇦🇮"},
    {"al", "🇦🇱"}, {"am", "🇦🇲"}, {"ao", "🇦🇴"}, {"aq", "🇦🇶"}, {"ar", "🇦🇷"},
    {"as", "🇦🇸"}, {"at", "🇦🇹"}, {"au", "🇦🇺"}, {"aw", "🇦🇼"}, {"ax", "🇦🇽"},
    {"az", "🇦🇿"}, {"ba", "🇧🇦"}, {"bb", "🇧🇧"}, {"bd", "🇧🇩"}, {"be", "🇧🇪"},
    {"bf", "🇧🇫"}, {"bg", "🇧🇬"}, {"bh", "🇧🇭"}, {"bi", "🇧🇮"}, {"bj", "🇧🇯"},
    {"bl", "🇧🇱"}, {"bm", "🇧🇲"}, {"bn", "🇧🇳"}, {"bo", "🇧🇴"}, {"bq", "🇧🇶"},
    {"br", "🇧🇷"}, {"bs", "🇧🇸"}, {"bt", "🇧🇹"}, {"bv", "🇧🇻"}, {"bw", "🇧🇼"},
    {"by", "🇧🇾"}, {"bz", "🇧🇿"}, {"ca", "🇨🇦"}, {"cc", "🇨🇨"}, {"cd", "🇨🇩"},
    {"cf", "🇨🇫"}, {"cg", "🇨🇬"}, {"ch", "🇨🇭"}, {"ci", "🇨🇮"}, {"ck", "🇨🇰"},
    {"cl", "🇨🇱"}, {"cm", "🇨🇲"}, {"cn", "🇨🇳"}, {"co", "🇨🇴"}, {"cr", "🇨🇷"},
    {"cu", "🇨🇺"}, {"cv", "🇨🇻"}, {"cw", "🇨🇼"}, {"cx", "🇨🇽"}, {"cy", "🇨🇾"},
    {"cz", "🇨🇿"}, {"de", "🇩🇪"}, {"dj", "🇩🇯"}, {"dk", "🇩🇰"}, {"dm", "🇩🇲"},
    {"do", "🇩🇴"}, {"dz", "🇩🇿"}, {"ec", "🇪🇨"}, {"ee", "🇪🇪"}, {"eg", "🇪🇬"},
    {"eh", "🇪🇭"}, {"er", "🇪🇷"}, {"es", "🇪🇸"}, {"et", "🇪🇹"}, {"fi", "🇫🇮"},
    {"fj", "🇫🇯"}, {"fk", "🇫🇰"}, {"fm", "🇫🇲"}, {"fo", "🇫🇴"}, {"fr", "🇫🇷"},
    {"ga", "🇬🇦"}, {"gb", "🇬🇧"}, {"gd", "🇬🇩"}, {"ge", "🇬🇪"}, {"gf", "🇬🇫"},
    {"gg", "🇬🇬"}, {"gh", "🇬🇭"}, {"gi", "🇬🇮"}, {"gl", "🇬🇱"}, {"gm", "🇬🇲"},
    {"gn", "🇬🇳"}, {"gp", "🇬🇵"}, {"gq", "🇬🇶"}, {"gr", "🇬🇷"}, {"gs", "🇬🇸"},
    {"gt", "🇬🇹"}, {"gu", "🇬🇺"}, {"gw", "🇬🇼"}, {"gy", "🇬🇾"}, {"hk", "🇭🇰"},
    {"hm", "🇭🇲"}, {"hn", "🇭🇳"}, {"hr", "🇭🇷"}, {"ht", "🇭🇹"}, {"hu", "🇭🇺"},
    {"id", "🇮🇩"}, {"ie", "🇮🇪"}, {"il", "🇮🇱"}, {"im", "🇮🇲"}, {"in", "🇮🇳"},
    {"io", "🇮🇴"}, {"iq", "🇮🇶"}, {"ir", "🇮🇷"}, {"is", "🇮🇸"}, {"it", "🇮🇹"},
    {"je", "🇯🇪"}, {"jm", "🇯🇲"}, {"jo", "🇯🇴"}, {"jp", "🇯🇵"}, {"ke", "🇰🇪"},
    {"kg", "🇰🇬"}, {"kh", "🇰🇭"}, {"ki", "🇰🇮"}, {"km", "🇰🇲"}, {"kn", "🇰🇳"},
    {"kp", "🇰🇵"}, {"kr", "🇰🇷"}, {"kw", "🇰🇼"}, {"ky", "🇰🇾"}, {"kz", "🇰🇿"},
    {"la", "🇱🇦"}, {"lb", "🇱🇧"}, {"lc", "🇱🇨"}, {"li", "🇱🇮"}, {"lk", "🇱🇰"},
    {"lr", "🇱🇷"}, {"ls", "🇱🇸"}, {"lt", "🇱🇹"}, {"lu", "🇱🇺"}, {"lv", "🇱🇻"},
    {"ly", "🇱🇾"}, {"ma", "🇲🇦"}, {"mc", "🇲🇨"}, {"md", "🇲🇩"}, {"me", "🇲🇪"},
    {"mf", "🇲🇫"}, {"mg", "🇲🇬"}, {"mh", "🇲🇭"}, {"mk", "🇲🇰"}, {"ml", "🇲🇱"},
    {"mm", "🇲🇲"}, {"mn", "🇲🇳"}, {"mo", "🇲🇴"}, {"mp", "🇲🇵"}, {"mq", "🇲🇶"},
    {"mr", "🇲🇷"}, {"ms", "🇲🇸"}, {"mt", "🇲🇹"}, {"mu", "🇲🇺"}, {"mv", "🇲🇻"},
    {"mw", "🇲🇼"}, {"mx", "🇲🇽"}, {"my", "🇲🇾"}, {"mz", "🇲🇿"}, {"na", "🇳🇦"},
    {"nc", "🇳🇨"}, {"ne", "🇳🇪"}, {"nf", "🇳🇫"}, {"ng", "🇳🇬"}, {"ni", "🇳🇮"},
    {"nl", "🇳🇱"}, {"no", "🇳🇴"}, {"np", "🇳🇵"}, {"nr", "🇳🇷"}, {"nu", "🇳🇺"},
    {"nz", "🇳🇿"}, {"om", "🇴🇲"}, {"pa", "🇵🇦"}, {"pe", "🇵🇪"}, {"pf", "🇵🇫"},
    {"pg", "🇵🇬"}, {"ph", "🇵🇭"}, {"pk", "🇵🇰"}, {"pl", "🇵🇱"}, {"pm", "🇵🇲"},
    {"pn", "🇵🇳"}, {"pr", "🇵🇷"}, {"ps", "🇵🇸"}, {"pt", "🇵🇹"}, {"pw", "🇵🇼"},
    {"py", "🇵🇾"}, {"qa", "🇶🇦"}, {"re", "🇷🇪"}, {"ro", "🇷🇴"}, {"rs", "🇷🇸"},
    {"ru", "🇷🇺"}, {"rw", "🇷🇼"}, {"sa", "🇸🇦"}, {"sb", "🇸🇧"}, {"sc", "🇸🇨"},
    {"sd", "🇸🇩"}, {"se", "🇸🇪"}, {"sg", "🇸🇬"}, {"sh", "🇸🇭"}, {"si", "🇸🇮"},
    {"sj", "🇸🇯"}, {"sk", "🇸🇰"}, {"sl", "🇸🇱"}, {"sm", "🇸🇲"}, {"sn", "🇸🇳"},
    {"so", "🇸🇴"}, {"sr", "🇸🇷"}, {"ss", "🇸🇸"}, {"st", "🇸🇹"}, {"sv", "🇸🇻"},
    {"sx", "🇸🇽"}, {"sy", "🇸🇾"}, {"sz", "🇸🇿"}, {"tc", "🇹🇨"}, {"td", "🇹🇩"},
    {"tf", "🇹🇫"}, {"tg", "🇹🇬"}, {"th", "🇹🇭"}, {"tj", "🇹🇯"}, {"tk", "🇹🇰"},
    {"tl", "🇹🇱"}, {"tm", "🇹🇲"}, {"tn", "🇹🇳"}, {"to", "🇹🇴"}, {"tr", "🇹🇷"},
    {"tt", "🇹🇹"}, {"tv", "🇹🇻"}, {"tw", "🇹🇼"}, {"tz", "🇹🇿"}, {"ua", "🇺🇦"},
    {"ug", "🇺🇬"}, {"um", "🇺🇲"}, {"us", "🇺🇸"}, {"uy", "🇺🇾"}, {"uz", "🇺🇿"},
    {"va", "🇻🇦"}, {"vc", "🇻🇨"}, {"ve", "🇻🇪"}, {"vg", "🇻🇬"}, {"vi", "🇻🇮"},
    {"vn", "🇻🇳"}, {"vu", "🇻🇺"}, {"wf", "🇼🇫"}, {"ws", "🇼🇸"}, {"ye", "🇾🇪"},
    {"yt", "🇾🇹"}, {"za", "🇿🇦"}, {"zm", "🇿🇲"}, {"zw", "🇿🇼"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unknown Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::unordered_set<std::string> UNKNOWN_EMOJIS;
std::unordered_set<std::string> UNKNOWN_FLAGS;

} // namespace

namespace mobius::extension::app::skype
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param message Message string
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
message_parser::message_parser (const std::string &message)
    : parser_ (mobius::core::io::new_bytearray_reader (message))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add element to content list
// @param element Element to add
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::add_element (const mobius::core::pod::map &element)
{
    const auto element_type = element.get<std::string> ("type");
    const auto element_text =
        mobius::core::string::strip (element.get<std::string> ("text"));

    // Check if text or system message is empty
    if ((element_type == "text" || element_type == "system") &&
        element_text.empty ())
        return;

    // Try to merge text or system message with previous element
    if (content_.size () > 0)
    {
        auto p_element = content_.back ();
        const auto p_type = p_element.get<std::string> ("type");
        const auto p_text = p_element.get<std::string> ("text");

        if (p_type == "text" && element_type == "text")
        {
            p_element.set ("text", p_text + element_text);
            return;
        }

        else if (p_type == "system" && element_type == "system")
        {
            p_element.set ("text", p_text + ". " + element_text);
            return;
        }
    }

    // Add new element
    content_.push_back (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::parse ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    using element_type = mobius::core::decoder::sgml::parser::element::type;
    auto e = parser_.get ();

    while (e.get_type () != element_type::end)
    {
        auto text = e.get_text ();

        switch (e.get_type ())
        {
            case element_type::text:
                _add_text_element (text);
                break;

            case element_type::start_tag:
                _parse_start_tag (text);
                break;

            case element_type::end_tag:
                _parse_end_tag (text);
                break;

            case element_type::empty_tag:
                _parse_empty_tag (text);
                break;

            case element_type::entity:
                _parse_entity (text);
                break;

            default:;
        };

        e = parser_.get ();
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse start tag
// @param tag Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_start_tag (const std::string &tag)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    if (tag == "a")
        _parse_a ();

    else if (tag == "b")
        add_element (mobius::core::pod::map {{"type", "start/b"}});

    else if (tag == "files")
        _parse_files ();

    else if (tag == "flag")
        _parse_flag ();

    else if (tag == "i")
        add_element (mobius::core::pod::map {{"type", "start/i"}});

    else if (tag == "legacyquote")
        _parse_legacyquote ();

    else if (tag == "partlist")
        _parse_partlist ();

    else if (tag == "quote")
        _parse_quote ();

    else if (tag == "s")
        add_element (mobius::core::pod::map {{"type", "start/s"}});

    else if (tag == "sms")
        _parse_sms ();

    else if (tag == "ss")
        _parse_ss ();

    else
        log.development (__LINE__, "Unhandled start tag: <" + tag + ">");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse end tag
// @param tag Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_end_tag (const std::string &tag)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    mobius::core::pod::map element;

    if (tag == "b")
        element = mobius::core::pod::map {{"type", "end/b"}};

    else if (tag == "i")
        element = mobius::core::pod::map {{"type", "end/i"}};

    else if (tag == "quote")
        element = mobius::core::pod::map {{"type", "end/quote"}};

    else if (tag == "s")
        element = mobius::core::pod::map {{"type", "end/s"}};

    else
        log.development (__LINE__, "Unhandled end tag: </" + tag + ">");

    if (element)
        add_element (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse empty tag
// @param tag Tag name
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_empty_tag (const std::string &tag)
{
    mobius::core::log log (__FILE__, __FUNCTION__);
    log.development (__LINE__, "Unhandled empty tag: " + tag);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse entity and add to content
// @param entity
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_entity (const std::string &entity)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    std::string text;

    // Handle predefined entities
    if (entity == "lt")
        text = "<";

    else if (entity == "gt")
        text = ">";

    else if (entity == "amp")
        text = "&";

    else if (entity == "apos")
        text = "'";

    else if (entity == "quot")
        text = "\"";

    // Unhandled entity
    else
    {
        log.development (__LINE__, "Unhandled entity: " + entity);
        return;
    }

    // Add text element
    _add_text_element (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse a tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_a ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <a> tag");
        return;
    }

    // Add href element
    auto href = tag.get_attribute<std::string> ("href");

    add_element (mobius::core::pod::map {{"type", "href"}, {"url", href}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <files> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_files ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <sms> tag");
        return;
    }

    // Get files
    std::size_t total_size = 0;
    std::size_t file_count = 0;
    std::string file_list;

    for (const auto &child : tag.get_children ())
    {
        if (child.get_name () == "file")
        {
            auto size = stoll (child.get_attribute<std::string> ("size", "0"));
            auto file_name = child.get_content ();

            if (!file_list.empty ())
                file_list += ", ";

            file_list += file_name;

            total_size += size;
            file_count++;
        }
    }

    // Add system message element
    if (file_count == 1)
        add_system_element (
            std::format (
                "File sent: {} (Size: {} bytes)", file_list, total_size
            )
        );

    else if (file_count > 1)
        add_system_element (
            std::format (
                "Files sent: {} (Total size: {} bytes)", file_list, total_size
            )
        );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <flag> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_flag ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <flag> tag");
        return;
    }

    // Create element
    auto element = mobius::core::pod::map {{"type", "flag"}};

    // Set emoji code
    auto country = tag.get_attribute<std::string> ("country");
    auto iter = FLAG_CHARS.find (country);

    if (iter != FLAG_CHARS.end ())
        element.set ("code", iter->second);

    else if (UNKNOWN_FLAGS.find (country) == UNKNOWN_FLAGS.end ())
    {
        log.development (__LINE__, "Unknown flag: " + country);
        UNKNOWN_FLAGS.insert (country);
    }

    // Set text
    auto text = tag.get_content ();

    if (!text.empty ())
        element.set ("text", text);

    // Add element
    add_element (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <legacyquote> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_legacyquote ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <flag> tag");
        return;
    }

    std::ignore = tag; // ignore data
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <partlist> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_partlist ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <partlist> tag");
        return;
    }

    // Get participants
    std::size_t participant_count = 0;
    std::string text;

    for (const auto &child : tag.get_children ())
    {
        if (child.get_name () == "part")
        {
            auto identity = child.get_attribute<std::string> ("identity");

            auto name_tag = child.get_child ("name");
            auto name = name_tag ? name_tag.get_content () : std::string ();

            if (participant_count == 0)
                text += "Participants: ";

            else
                text += ", ";

            text += name.empty () ? identity : name + " (" + identity + ")";
            participant_count++;
        }
    }

    if (participant_count == 0)
        text += "No participants.";

    // Add system message element
    add_system_element (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <quote> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_quote ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get last element
    auto e = parser_.get_last ();
    if (e.get_type () !=
            mobius::core::decoder::sgml::parser::element::type::start_tag ||
        e.get_text () != "quote")
    {
        log.warning (__LINE__, "Invalid <quote> tag");
        return;
    }

    // Get attributes
    auto timestamp = e.get_attribute<std::string> ("timestamp");
    auto author_id = e.get_attribute<std::string> ("author");
    auto author_name = e.get_attribute<std::string> ("authorname");

    std::string author = author_id;
    if (!author_name.empty ())
        author += " (" + author_name + ")";

    auto element = mobius::core::pod::map {
        {"type", "start/quote"},
        {"author", author},
    };

    if (!timestamp.empty ())
        element.set (
            "timestamp",
            mobius::core::datetime::new_datetime_from_unix_timestamp (
                std::stoul (timestamp)
            )
        );

    add_element (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <sms> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_sms ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <sms> tag");
        return;
    }

    // Get targets and timestamp
    std::string targets;
    std::string msg = tag.get_attribute<std::string> ("alt");
    mobius::core::datetime::datetime timestamp;

    for (const auto &child : tag.get_children ())
    {
        auto child_name = child.get_name ();

        if (child_name == "sendtimestamp")
        {
            auto timestamp_str = child.get_content ();
            timestamp =
                mobius::core::datetime::new_datetime_from_unix_timestamp (
                    std::stoll (timestamp_str)
                );
        }

        else if (child_name == "targets")
        {
            for (const auto &target : child.get_children ())
            {
                if (target.get_name () == "target")
                {
                    if (!targets.empty ())
                        targets += ", ";
                    targets += target.get_content ();
                }
            }
        }
    }

    // Format system message text
    std::string text = "SMS message sent";

    if (timestamp)
        text += " at " + to_string (timestamp) + " (UTC)";

    if (!targets.empty ())
        text += " to " + targets;

    if (!msg.empty ())
        text += ": " + msg;

    text += ".";

    // Add system message element
    add_system_element (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse <ss> tag
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
message_parser::_parse_ss ()
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    // Get minidom tag
    auto tag = parser_.get_minidom ();
    if (!tag)
    {
        log.warning (__LINE__, "Invalid <ss> tag");
        return;
    }

    // Create element
    auto element = mobius::core::pod::map {{"type", "emoji"}};

    // Set emoji code
    auto ss_type = tag.get_attribute<std::string> ("type");
    auto iter = EMOJI_CHARS.find (ss_type);

    if (iter != EMOJI_CHARS.end ())
        element.set ("code", iter->second);

    else if (UNKNOWN_EMOJIS.find (ss_type) == UNKNOWN_EMOJIS.end ())
    {
        log.development (__LINE__, "Unknown emoji type: " + ss_type);
        UNKNOWN_EMOJIS.insert (ss_type);
    }

    // Set text
    auto text = tag.get_content ();

    if (!text.empty ())
        element.set ("text", text);

    // Add element
    add_element (element);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Parse Skype message
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::map>
parse_message (const std::string &message)
{
    mobius::core::log log (__FILE__, __FUNCTION__);

    try
    {
        message_parser parser (message);
        parser.parse ();

        auto content = parser.get_content ();

        if (content.empty ())
            content = {
                mobius::core::pod::map {{"type", "text"}, {"text", message}}
            };

        return content;
    }
    catch (const std::exception &e)
    {
        log.warning (__LINE__, e.what ());
        return {};
    }
}

} // namespace mobius::extension::app::skype
