// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <mobius/core/log.hpp>
#include <mobius/core/richtext.hpp>
#include <mobius/core/string_functions.hpp>
#include <format>
#include <unordered_map>
#include <unordered_set>

namespace
{
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
// @brief Emoji representation as Unicode char
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const std::unordered_map<std::string, std::string> EMOJI_CHARS = {
    {"angel", "👼"},
    {"anger", "🤯"},
    {"angry", "😠"},
    {"bandit", "😈"},
    {"banned", "🚫"},
    {"bear", "🧸"},
    {"bike", "🚴"},
    {"bombed", "💣"},
    {"blush", "😳"},
    {"bomb", "💣"},
    {"brokenheart", "💔"},
    {"brb", "🕒"},
    {"call", "📞"},
    {"camera", "📷"},
    {"catface", "🐱"},
    {"champagne", "🍾"},
    {"chocolate", "🍫"},
    {"clap", "👏"},
    {"clapping", "👏"},
    {"clownface", "🤡"},
    {"confused", "😕"},
    {"coolguy", "😎"},
    {"crying", "😢"},
    {"danceparty", "🕺"},
    {"devilface", "😈"},
    {"disgust", "🤢"},
    {"dogface", "🐶"},
    {"dollarbill", "💵"},
    {"dollar", "💵"},
    {"dollars", "💵"},
    {"dizzy", "😵"},
    {"cake", "🎂"},
    {"cash", "💰"},
    {"cat", "🐈"},
    {"claps", "👏"},
    {"clown", "🤡"},
    {"coffee_cup", "☕"},
    {"coffee", "☕"},
    {"cool", "😎"},
    {"cry", "😢"},
    {"cwl", "😂"},
    {"dance", "🕺"},
    {"devil", "😈"},
    {"dog", "🐕"},
    {"drink", "🍸"},
    {"drunk", "🥴"},
    {"dull", "🙄"},
    {"emo", "🤯"},
    {"envy", "😒"},
    {"explode", "💣"},
    {"explosion", "💣"},
    {"evilgrin", "😈"},
    {"facepalm", "🤦"},
    {"fear", "😨"},
    {"fingerscrossed", "🤞"},
    {"flower", "🌸"},
    {"flushed", "😳"},
    {"frown", "☹"},
    {"funny", "😂"},
    {"giftbox", "🎁"},
    {"ghost", "👻"},
    {"gift", "🎁"},
    {"glasses", "🕶"},
    {"giggle", "🤭"},
    {"handsinair", "🙌"},
    {"happytears", "😂"},
    {"happy", "🙂"},
    {"heart", "❤"},
    {"hearteyes", "😍"},
    {"hearthands", "🤲"},
    {"heidy", "🐿"},
    {"hi", "👋"},
    {"highfive", "🖐"},
    {"hooray", "🎉"},
    {"hug", "🧸"},
    {"idea", "💡"},
    {"iheartyou", "❤️"},
    {"iloveyou", "❤️"},
    {"innocent", "😇"},
    {"inlove", "🥰"},
    {"joy", "😂"},
    {"kissing", "😗"},
    {"kiss", "😗"},
    {"ladyvamp", "🧛"},
    {"ladyvampire", "🧛"},
    {"laughing", "😆"},
    {"laugh", "😃"},
    {"like", "👍"},
    {"lips", "💋"},
    {"lipssealed", "🤐"},
    {"loudlycrying", "😭"},
    {"mansignlove", ""},
    {"makeup", "💄"},
    {"monocle", "🧐"},
    {"mail", "✉"},
    {"meh", "😑"},
    {"money", "💰"},
    {"mmm", "😋"},
    {"monkey", "🐒"},
    {"mooning", "🌝"},
    {"muscle", "💪"},
    {"muscleman", "💪"},
    {"music", "🎶"},
    {"nerd", "🤓"},
    {"nerdy", "🤓"},
    {"ninja", "🥷"},
    {"no", "👎"},
    {"nod", "👍"},
    {"notworthy", "🙄"},
    {"okeydokey", "👌"},
    {"ok", "👌"},
    {"party", "🥳"},
    {"phone", "📱"},
    {"pig", "🐖"},
    {"poop", "💩"},
    {"pray", "🙏"},
    {"pizza", "🍕"},
    {"praying", "🙏"},
    {"puke", "🤮"},
    {"pumpkin", "🎃"},
    {"punch", "👊"},
    {"priidu", "🤠"},
    {"rolleyes", "🙄"},
    {"robot", "🤖"},
    {"rose", "🌹"},
    {"rain", "🌧"},
    {"roflmao", "🤣"},
    {"rofl", "🤣"},
    {"rocket", "🚀"},
    {"rock", "🤘"},
    {"sadface", "😞"},
    {"sadcat", "😿"},
    {"sadcry", "😭"},
    {"sad", "😧"},
    {"sadness", "😢"},
    {"satisfied", "😌"},
    {"sarcastic", "😒"},
    {"scared", "😱"},
    {"scream", "😱"},
    {"shocked", "😲"},
    {"shake", "🤝"},
    {"shy", "😊"},
    {"sick", "🤢"},
    {"sleeping", "😴"},
    {"skype", "💬"},
    {"skull", "💀"},
    {"sleepy", "😪"},
    {"smile", "😄"},
    {"smirk", "😏"},
    {"speechless", "😐"},
    {"squirrel", "🐿"},
    {"star", "⭐"},
    {"stareyes", "🤩"},
    {"sun", "🌞"},
    {"surprised", "😲"},
    {"swear", "🤬"},
    {"stop", "✋"},
    {"sweat", "😓"},
    {"think", "🤔"},
    {"time", "⏲"},
    {"tongueout", "😛"},
    {"tmi", "🤭"},
    {"toothygrin", "😁"},
    {"tongue", "😛"},
    {"thumbsdown", "👎"},
    {"thumbsup", "👍"},
    {"tired", "😫"},
    {"tumbleweed", "🌵"},
    {"unhappy", "☹"},
    {"unamused", "😒"},
    {"vampire", "🧛"},
    {"victory", "✌"},
    {"waiting", "⏳"},
    {"wavehand", "👋"},
    {"wave", "👋"},
    {"weary", "😩"},
    {"whistle", "😗"},
    {"winkey", "😉"},
    {"woozy", "🥴"},
    {"wasntme", "🙄"},
    {"wave", "🌊"},
    {"whew", "😮‍💨"},
    {"wink", "😉"},
    {"womanblowkiss", "💋"},
    {"womanfacepalm", "🤦‍♀️"},
    {"worry", "😟"},
    {"wonder", "🤔"},
    {"xd", "😆"},
    {"xmasheart", "💖"},
    {"xmasyes", "👍"},
    {"yawned", "🥱"},
    {"yawn", "🥱"},
    {"yes", "👍"},
    {"yoga", "🧘"},
    {"zombie", "🧟"},
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Unknown Data
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static std::unordered_set<std::string> UNKNOWN_EMOJIS;
static std::unordered_set<std::string> UNKNOWN_FLAGS;

} // namespace

namespace mobius::core
{
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief RichText implementation class
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class richtext::impl
{
  public:
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Constructors
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl () = default;
    explicit impl (const std::vector<mobius::core::pod::map> &);
    impl (const impl &) = delete;
    impl (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Operators
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    impl &operator= (const impl &) = delete;
    impl &operator= (impl &&) = delete;

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Check if richtext is not empty
    // @return true if richtext has content, false otherwise
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    operator bool () const noexcept
    {
        return !segments_.empty ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get number of segments
    // @return Number of segments
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::size_t
    size () const
    {
        return segments_.size ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Get segments
    // @return Vector of segments
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    std::vector<segment>
    get_segments () const
    {
        return segments_;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // @brief Clear richtext content
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void
    clear ()
    {
        segments_.clear ();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Prototypes
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    void begin_bold ();
    void begin_italic ();
    void begin_link (const std::string &url);
    void begin_message_bubble (const std::string &, const std::string &);
    void begin_strikethrough ();
    void begin_underline ();

    void end_bold ();
    void end_italic ();
    void end_link ();
    void end_message_bubble ();
    void end_strikethrough ();
    void end_underline ();

    void add_text (const std::string &);
    void add_system_text (const std::string &);
    void add_newline ();
    void add_emoji (const std::string &);
    void add_flag (const std::string &);

    std::string to_html () const;
    std::string to_markdown () const;
    std::string to_latex () const;
    std::string to_pango () const;
    std::vector<mobius::core::pod::map> to_pod () const;

  private:
    std::vector<segment> segments_;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Constructor
// @param segments Vector of segments to initialize richtext
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
richtext::impl::impl (const std::vector<mobius::core::pod::map> &segments)
{
    segments_.reserve (segments.size ());

    std::transform (
        segments.begin (), segments.end (), std::back_inserter (segments_),
        [] (const mobius::core::pod::map &seg) -> segment
        {
            mobius::core::pod::map cmap = seg.clone ();
            auto type = cmap.pop<std::string> ("type");
            return {type, cmap};
        }
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin bold
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::begin_bold ()
{
    segments_.push_back ({"begin/b", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin italic
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::begin_italic ()
{
    segments_.push_back ({"begin/i", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin hyperlink
// @param url URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::begin_link (const std::string &url)
{
    segments_.push_back ({"begin/link", {{"url", url}}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin message bubble
// @param timestamp Timestamp
// @param author Author
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::begin_message_bubble (
    const std::string &timestamp, const std::string &author
)
{
    segments_.push_back (
        {"begin/message_bubble", {{"timestamp", timestamp}, {"author", author}}}
    );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin strikethrough
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::begin_strikethrough ()
{
    segments_.push_back ({"begin/s", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin underline
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::begin_underline ()
{
    segments_.push_back ({"begin/u", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End bold text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::end_bold ()
{
    segments_.push_back ({"end/b", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End italic text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::end_italic ()
{
    segments_.push_back ({"end/i", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End hyperlink
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::end_link ()
{
    segments_.push_back ({"end/link", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End message bubble
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::end_message_bubble ()
{
    segments_.push_back ({"end/message_bubble", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End strikethrough
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::end_strikethrough ()
{
    segments_.push_back ({"end/s", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End underline
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::end_underline ()
{
    segments_.push_back ({"end/u", {}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add normal text
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::add_text (const std::string &text)
{
    // If text is empty, do not add segment
    if (text.empty ())
        return;

    // Try to merge text with previous segment if it is also text
    if (segments_.size () > 0)
    {
        auto &p = segments_.back ();

        if (p.type == "text")
        {
            auto p_text = p.metadata.get<std::string> ("content");
            p.metadata.set ("content", p_text + text);
            return;
        }
    }

    segments_.push_back ({"text", {{"content", text}}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add system text
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::add_system_text (const std::string &text)
{
    // If text is empty, do not add segment
    if (text.empty ())
        return;

    // Try to merge text with previous segment if it is also text
    if (segments_.size () > 0)
    {
        auto &p = segments_.back ();

        if (p.type == "system_text")
        {
            auto p_text = p.metadata.get<std::string> ("content");
            p.metadata.set ("content", p_text + text);
            return;
        }
    }

    segments_.push_back ({"system_text", {{"content", text}}});
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add newline
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::add_newline ()
{
    add_text ("\n");
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add emoji char
// @param id Emoji ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::add_emoji (const std::string &id)
{
    // Add segment
    segments_.push_back ({"emoji", {{"id", id}}});

    // Log unknown emoji only once
    if (EMOJI_CHARS.find (id) == EMOJI_CHARS.end () &&
        UNKNOWN_EMOJIS.find (id) == UNKNOWN_EMOJIS.end ())
    {
        mobius::core::log log (__FILE__, __FUNCTION__);
        log.development (__LINE__, "Unknown emoji: " + id);

        UNKNOWN_EMOJIS.insert (id);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add flag char
// @param id Flag ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::impl::add_flag (const std::string &id)
{
    // Add segment
    segments_.push_back ({"flag", {{"id", id}}});

    // Log unknown flag only once
    if (FLAG_CHARS.find (id) == FLAG_CHARS.end () &&
        UNKNOWN_FLAGS.find (id) == UNKNOWN_FLAGS.end ())
    {
        mobius::core::log log (__FILE__, __FUNCTION__);
        log.development (__LINE__, "Unknown flag: " + id);

        UNKNOWN_FLAGS.insert (id);
    }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to HTML
// @return HTML markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::impl::to_html () const
{
    std::string html_text;

    for (const auto &segment : segments_)
    {
        if (segment.type == "begin/b")
            html_text += "<b>";

        else if (segment.type == "end/b")
            html_text += "</b>";

        else if (segment.type == "begin/i")
            html_text += "<i>";

        else if (segment.type == "end/i")
            html_text += "</i>";

        else if (segment.type == "begin/link")
        {
            auto url = segment.metadata.get<std::string> ("url");
            html_text += "<a href=\"" + url + "\">";
        }

        else if (segment.type == "end/link")
            html_text += "</a>";

        else if (segment.type == "begin/s")
            html_text += "<s>";

        else if (segment.type == "end/s")
            html_text += "</s>";

        else if (segment.type == "begin/u")
            html_text += "<u>";

        else if (segment.type == "end/u")
            html_text += "</u>";

        else if (segment.type == "text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            html_text += mobius::core::string::html_escape (text);
        }

        else if (segment.type == "system_text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            html_text += "<i><font color=\"#77b\" face=\"courier\">" +
                         mobius::core::string::html_escape (text) +
                         "</font></i>";
        }

        else if (segment.type == "hyperlink")
        {
            auto text = segment.metadata.get<std::string> ("text");
            auto uri = segment.metadata.get<std::string> ("uri");
            html_text += "<a href=\"" + uri + "\">" +
                         mobius::core::string::html_escape (text) + "</a>";
        }

        else if (segment.type == "emoji")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto emoji_char_it = EMOJI_CHARS.find (id);

            if (emoji_char_it != EMOJI_CHARS.end ())
                html_text += std::format (
                    "<font size=\"x-large\">{}</font>", emoji_char_it->second
                );

            else
                html_text += std::format (
                    "<font color=\"#00d000\" weight=\"bold\">({})</font>", id
                ); // Unknown emoji
        }

        else if (segment.type == "flag")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto flag_char_it = FLAG_CHARS.find (id);

            html_text += "<font size=\"x-large\">";

            if (flag_char_it != FLAG_CHARS.end ())
                html_text += flag_char_it->second;
            else
                html_text += "🏳️"; // Unknown flag

            html_text += "</font>";
        }

        else if (segment.type == "begin/message_bubble")
        {
            auto timestamp = segment.metadata.get<std::string> ("timestamp");
            auto author = segment.metadata.get<std::string> ("author");

            html_text += std::format (
                "<font color=\"#0080b0\">[{}] {}:<br/><i>", timestamp,
                mobius::core::string::html_escape (author)
            );
        }

        else if (segment.type == "end/message_bubble")
        {
            html_text += "</i></font>";
        }
    }

    return html_text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to LaTeX
// @return LaTeX markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::impl::to_latex () const
{
    std::string latex_text;

    for (const auto &segment : segments_)
    {
        if (segment.type == "begin/b")
            latex_text += "\\textbf{";

        else if (segment.type == "end/b")
            latex_text += "}";

        else if (segment.type == "begin/i")
            latex_text += "\\textit{";

        else if (segment.type == "end/i")
            latex_text += "}";

        else if (segment.type == "begin/link")
        {
            auto url = segment.metadata.get<std::string> ("url");
            latex_text += "\\href{" + url + "}{";
        }

        else if (segment.type == "end/link")
            latex_text += "}";

        else if (segment.type == "begin/s")
            latex_text += "\\sout{";

        else if (segment.type == "end/s")
            latex_text += "}";

        else if (segment.type == "begin/u")
            latex_text += "\\underline{";

        else if (segment.type == "end/u")
            latex_text += "}";

        else if (segment.type == "text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            latex_text += mobius::core::string::latex_escape (text);
        }

        else if (segment.type == "system_text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            latex_text += "\\textcolor{gray}{" +
                          mobius::core::string::latex_escape (text) + "}";
        }

        else if (segment.type == "hyperlink")
        {
            auto text = segment.metadata.get<std::string> ("text");
            auto uri = segment.metadata.get<std::string> ("uri");
            latex_text += "\\href{" + uri + "}{" +
                          mobius::core::string::latex_escape (text) + "}";
        }

        else if (segment.type == "emoji")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto emoji_char_it = EMOJI_CHARS.find (id);

            if (emoji_char_it != EMOJI_CHARS.end ())
                latex_text += "\\big{" + emoji_char_it->second + "}";
            else
                latex_text += "\\texttt{(" + id + ")}";
        }

        else if (segment.type == "flag")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto flag_char_it = FLAG_CHARS.find (id);

            if (flag_char_it != FLAG_CHARS.end ())
                latex_text += flag_char_it->second;
            else
                latex_text += "\\texttt{:unknown_flag:}"; // Unknown flag
        }
    }

    return latex_text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to Markdown
// @return MarkDown markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::impl::to_markdown () const
{
    std::string markdown_text;
    std::string last_link;

    for (const auto &segment : segments_)
    {
        if (segment.type == "begin/b")
            markdown_text += "**";

        else if (segment.type == "end/b")
            markdown_text += "**";

        else if (segment.type == "begin/i")
            markdown_text += "*";

        else if (segment.type == "end/i")
            markdown_text += "*";

        else if (segment.type == "begin/link")
        {
            last_link = segment.metadata.get<std::string> ("url");
            markdown_text += "[";
        }

        else if (segment.type == "end/link")
            markdown_text += "](" + last_link + ")";

        else if (segment.type == "begin/s")
            markdown_text += "~~";

        else if (segment.type == "end/s")
            markdown_text += "~~";

        else if (segment.type == "begin/u")
            markdown_text += "<u>";

        else if (segment.type == "end/u")
            markdown_text += "</u>";

        else if (segment.type == "text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            markdown_text += text;
        }

        else if (segment.type == "system_text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            markdown_text += std::format ("`{}`", text);
        }

        else if (segment.type == "hyperlink")
        {
            auto text = segment.metadata.get<std::string> ("text");
            auto uri = segment.metadata.get<std::string> ("uri");
            markdown_text += "[" + text + "](" + uri + ")";
        }

        else if (segment.type == "emoji")
        {
            auto id = segment.metadata.get<std::string> ("id");
            markdown_text += ":" + id + ":";
        }

        else if (segment.type == "flag")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto flag_char_it = FLAG_CHARS.find (id);

            if (flag_char_it != FLAG_CHARS.end ())
                markdown_text += flag_char_it->second;
            else
                markdown_text += ":unknown_flag:"; // Unknown flag
        }
    }

    return markdown_text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to Pango
// @return Pango markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::impl::to_pango () const
{
    std::string pango_text;

    for (const auto &segment : segments_)
    {
        if (segment.type == "text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            pango_text += mobius::core::string::html_escape (text);
        }

        else if (segment.type == "system_text")
        {
            auto text = segment.metadata.get<std::string> ("content");
            pango_text += "<span color=\"#77b\">" +
                          mobius::core::string::html_escape (text) + "</span>";
        }

        else if (segment.type == "begin/b")
            pango_text += "<b>";

        else if (segment.type == "end/b")
            pango_text += "</b>";

        else if (segment.type == "begin/i")
            pango_text += "<i>";

        else if (segment.type == "end/i")
            pango_text += "</i>";

        else if (segment.type == "begin/link")
        {
            auto url = segment.metadata.get<std::string> ("url");
            pango_text +=
                "<span underline=\"single\" color=\"blue\">" + mobius::core::string::html_escape (url) + " [";
        }

        else if (segment.type == "end/link")
            pango_text += "]</span>";

        else if (segment.type == "begin/s")
            pango_text += "<s>";

        else if (segment.type == "end/s")
            pango_text += "</s>";

        else if (segment.type == "begin/u")
            pango_text += "<u>";

        else if (segment.type == "end/u")
            pango_text += "</u>";

        else if (segment.type == "begin/message_bubble")
        {
            auto timestamp = segment.metadata.get<std::string> ("timestamp");
            auto author = segment.metadata.get<std::string> ("author");

            pango_text += "<span>";
            pango_text += std::format (
                "\n  <span weight=\"bold\" foreground=\"#1B72E8\">{}</span>",
                author
            );
            pango_text += std::format (
                "\n  <span size=\"x-small\" foreground=\"#667781\">  {}</span>",
                timestamp
            );
            pango_text +=
                "\n  <span background=\"#E1FEC6\" foreground=\"#000000\">";
        }

        else if (segment.type == "end/message_bubble")
            pango_text += "</span>\n</span>";

        else if (segment.type == "emoji")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto emoji_char_it = EMOJI_CHARS.find (id);

            if (emoji_char_it != EMOJI_CHARS.end ())
                pango_text += "<span size=\"x-large\">" +
                              emoji_char_it->second + "</span>";
            else // unknown emoji
                pango_text += "<span color=\"#00d000\" weight=\"bold\">(" + id +
                              ")</span>";
        }

        else if (segment.type == "flag")
        {
            auto id = segment.metadata.get<std::string> ("id");
            auto flag_char_it = FLAG_CHARS.find (id);

            if (flag_char_it != FLAG_CHARS.end ())
                pango_text += std::format (
                    "<span size=\"x-large\">{}</span>", flag_char_it->second
                );
            else // unknown flag
                pango_text += "<span size=\"x-large\">🏳️</span>";
        }
    }

    return pango_text;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to POD
// @return Vector of POD maps representing segments
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::map>
richtext::impl::to_pod () const
{
    std::vector<mobius::core::pod::map> pod_segments (segments_.size ());

    std::transform (
        segments_.begin (), segments_.end (), pod_segments.begin (),
        [] (const segment &segment)
        {
            mobius::core::pod::map pod_segment;
            pod_segment.set ("type", segment.type);
            pod_segment.update (segment.metadata);

            return pod_segment;
        }
    );

    return pod_segments;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief RichText constructor
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
richtext::richtext ()
    : impl_ (std::make_shared<impl> ())
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief RichText constructor with segments
// @param segments Vector of segments to initialize richtext
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
richtext::richtext (const std::vector<mobius::core::pod::map> &segments)
    : impl_ (std::make_shared<impl> (segments))
{
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Check if richtext is not empty
// @return true if richtext has content, false otherwise
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
richtext::
operator bool () const noexcept
{
    return impl_->operator bool ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Clear richtext content
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::clear ()
{
    impl_->clear ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get number of segments
// @return Number of segments
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::size_t
richtext::size () const
{
    return impl_->size ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Get segments
// @return Vector of segments
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<richtext::segment>
richtext::get_segments () const
{
    return impl_->get_segments ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin bold
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::begin_bold ()
{
    impl_->begin_bold ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin italic
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::begin_italic ()
{
    impl_->begin_italic ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin hyperlink
// @param url URL
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::begin_link (const std::string &url)
{
    impl_->begin_link (url);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add reply
// @param timestamp Timestamp
// @param author Author
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::begin_message_bubble (
    const std::string &timestamp, const std::string &author
)
{
    impl_->begin_message_bubble (timestamp, author);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin strikethrough
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::begin_strikethrough ()
{
    impl_->begin_strikethrough ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Begin underline
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::begin_underline ()
{
    impl_->begin_underline ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End bold text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::end_bold ()
{
    impl_->end_bold ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End italic text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::end_italic ()
{
    impl_->end_italic ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End hyperlink
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::end_link ()
{
    impl_->end_link ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End message bubble
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::end_message_bubble ()
{
    impl_->end_message_bubble ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End strikethrough
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::end_strikethrough ()
{
    impl_->end_strikethrough ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief End underline
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::end_underline ()
{
    impl_->end_underline ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add normal text
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::add_text (const std::string &text)
{
    impl_->add_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add system text
// @param text Text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::add_system_text (const std::string &text)
{
    impl_->add_system_text (text);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add newline
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::add_newline ()
{
    impl_->add_newline ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add emoji char
// @param id Emoji ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::add_emoji (const std::string &id)
{
    impl_->add_emoji (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Add flag char
// @param id Flag ID
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void
richtext::add_flag (const std::string &id)
{
    impl_->add_flag (id);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to HTML
// @return HTML markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::to_html () const
{
    return impl_->to_html ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to Markdown
// @return MarkDown markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::to_markdown () const
{
    return impl_->to_markdown ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to LaTeX
// @return LaTeX markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::to_latex () const
{
    return impl_->to_latex ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to Pango
// @return Pango markup text
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string
richtext::to_pango () const
{
    return impl_->to_pango ();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// @brief Convert richtext to POD
// @return Vector of POD maps representing segments
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::vector<mobius::core::pod::map>
richtext::to_pod () const
{
    return impl_->to_pod ();
}

} // namespace mobius::core
