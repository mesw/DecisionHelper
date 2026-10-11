#include "Session.h"
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cctype>

// ── Minimal hand-rolled JSON helpers ─────────────────────────────────────────

static std::string escapeStr(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    out += '"';
    return out;
}

// ── Serialise ────────────────────────────────────────────────────────────────

std::string Session::toJson(const DecisionSession& s) {
    std::ostringstream o;
    o << "{";
    o << "\"version\":2,";
    o << "\"nextOptionId\":" << s.nextOptionId << ",";
    o << "\"nextCriterionId\":" << s.nextCriterionId << ",";
    o << "\"title\":" << escapeStr(s.title) << ",";

    // options
    o << "\"options\":[";
    for (size_t i = 0; i < s.options.size(); ++i) {
        if (i) o << ",";
        const auto& opt = s.options[i];
        o << "{\"id\":" << opt.id
          << ",\"label\":" << escapeStr(opt.label)
          << ",\"score\":" << opt.score
          << ",\"rankPosition\":" << opt.rankPosition << "}";
    }
    o << "],";

    // criteria
    o << "\"criteria\":[";
    for (size_t i = 0; i < s.criteria.size(); ++i) {
        if (i) o << ",";
        const auto& c = s.criteria[i];
        o << "{\"id\":" << c.id
          << ",\"label\":" << escapeStr(c.label)
          << ",\"weight\":" << c.weight << "}";
    }
    o << "],";

    // option comparisons: map of criterionId -> map of "lo_hi" -> int
    o << "\"optionComparisons\":{";
    bool firstCrit = true;
    for (const auto& [critId, pairMap] : s.optionComparisons) {
        if (!firstCrit) o << ",";
        firstCrit = false;
        o << "\"" << critId << "\":{";
        bool firstPair = true;
        for (const auto& [pairKey, val] : pairMap) {
            if (!firstPair) o << ",";
            firstPair = false;
            o << "\"" << pairKey.first << "_" << pairKey.second << "\":" << val;
        }
        o << "}";
    }
    o << "},";

    // criteria comparisons: map of "lo_hi" -> int
    o << "\"criteriaComparisons\":{";
    bool firstPair = true;
    for (const auto& [pairKey, val] : s.criteriaComparisons) {
        if (!firstPair) o << ",";
        firstPair = false;
        o << "\"" << pairKey.first << "_" << pairKey.second << "\":" << val;
    }
    o << "}";

    o << "}";
    return o.str();
}

// ── Minimal JSON parser helpers ───────────────────────────────────────────────

struct Parser {
    const std::string& src;
    size_t pos = 0;

    void skipWs() { while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos; }
    char peek() { skipWs(); return pos < src.size() ? src[pos] : '\0'; }
    char consume() { skipWs(); return pos < src.size() ? src[pos++] : '\0'; }
    void expect(char c) { if (consume() != c) throw std::runtime_error("JSON parse error"); }

    std::string parseStr() {
        expect('"');
        std::string out;
        while (pos < src.size()) {
            char c = src[pos++];
            if (c == '"') return out;
            if (c == '\\') {
                if (pos >= src.size()) break;
                c = src[pos++];
                if (c == 'n') out += '\n';
                else out += c;
            } else {
                out += c;
            }
        }
        throw std::runtime_error("unterminated string");
    }

    long long parseInt() {
        skipWs();
        long long v = 0;
        bool neg = false;
        if (pos < src.size() && src[pos] == '-') { neg = true; ++pos; }
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos])))
            v = v * 10 + (src[pos++] - '0');
        return neg ? -v : v;
    }

    double parseFloat() {
        skipWs();
        size_t start = pos;
        if (pos < src.size() && src[pos] == '-') ++pos;
        while (pos < src.size() && (std::isdigit(static_cast<unsigned char>(src[pos])) ||
               src[pos] == '.' || src[pos] == 'e' || src[pos] == 'E' || src[pos] == '+' || src[pos] == '-'))
            ++pos;
        return std::stod(src.substr(start, pos - start));
    }

    void skipValue() {
        char c = peek();
        if (c == '"') { parseStr(); return; }
        if (c == '{') {
            consume();
            while (peek() != '}' && peek() != '\0') {
                parseStr(); expect(':'); skipValue();
                if (peek() == ',') consume();
            }
            if (peek() == '}') consume();
            return;
        }
        if (c == '[') {
            consume();
            while (peek() != ']' && peek() != '\0') {
                skipValue();
                if (peek() == ',') consume();
            }
            if (peek() == ']') consume();
            return;
        }
        while (pos < src.size() && src[pos] != ',' && src[pos] != '}' && src[pos] != ']' &&
               !std::isspace(static_cast<unsigned char>(src[pos])))
            ++pos;
    }
};

// ── Deserialise ──────────────────────────────────────────────────────────────

DecisionSession Session::fromJson(const std::string& json) {
    DecisionSession s;
    if (json.empty()) return s;

    try {
        Parser p{json, 0};
        p.expect('{');

        while (p.peek() != '}' && p.peek() != '\0') {
            std::string key = p.parseStr();
            p.expect(':');

            if (key == "title") {
                s.title = p.parseStr();
            } else if (key == "nextOptionId") {
                s.nextOptionId = static_cast<uint32_t>(p.parseInt());
            } else if (key == "nextCriterionId") {
                s.nextCriterionId = static_cast<uint32_t>(p.parseInt());
            } else if (key == "options") {
                p.expect('[');
                while (p.peek() != ']' && p.peek() != '\0') {
                    p.expect('{');
                    Option opt;
                    while (p.peek() != '}' && p.peek() != '\0') {
                        std::string ok = p.parseStr();
                        p.expect(':');
                        if (ok == "id") opt.id = static_cast<OptionId>(p.parseInt());
                        else if (ok == "label") opt.label = p.parseStr();
                        else if (ok == "score") opt.score = static_cast<float>(p.parseFloat());
                        else if (ok == "rankPosition") opt.rankPosition = static_cast<int>(p.parseInt());
                        else p.skipValue();
                        if (p.peek() == ',') p.consume();
                    }
                    p.expect('}');
                    s.options.push_back(opt);
                    if (p.peek() == ',') p.consume();
                }
                p.expect(']');
            } else if (key == "criteria" || key == "properties") {
                p.expect('[');
                while (p.peek() != ']' && p.peek() != '\0') {
                    p.expect('{');
                    Criterion c;
                    while (p.peek() != '}' && p.peek() != '\0') {
                        std::string ck = p.parseStr();
                        p.expect(':');
                        if (ck == "id") c.id = static_cast<CriterionId>(p.parseInt());
                        else if (ck == "label") c.label = p.parseStr();
                        else if (ck == "weight") c.weight = static_cast<float>(p.parseFloat());
                        else p.skipValue();
                        if (p.peek() == ',') p.consume();
                    }
                    p.expect('}');
                    s.criteria.push_back(c);
                    if (p.peek() == ',') p.consume();
                }
                p.expect(']');
            } else if (key == "optionComparisons") {
                p.expect('{');
                while (p.peek() != '}' && p.peek() != '\0') {
                    std::string critIdStr = p.parseStr();
                    CriterionId critId = static_cast<CriterionId>(std::stoul(critIdStr));
                    p.expect(':');
                    p.expect('{');
                    while (p.peek() != '}' && p.peek() != '\0') {
                        std::string pkStr = p.parseStr();
                        p.expect(':');
                        int val = static_cast<int>(p.parseInt());
                        auto u = pkStr.find('_');
                        if (u != std::string::npos) {
                            uint32_t a = static_cast<uint32_t>(std::stoul(pkStr.substr(0, u)));
                            uint32_t b = static_cast<uint32_t>(std::stoul(pkStr.substr(u + 1)));
                            s.optionComparisons[critId][makePairKey(a, b)] = val;
                        }
                        if (p.peek() == ',') p.consume();
                    }
                    p.expect('}');
                    if (p.peek() == ',') p.consume();
                }
                p.expect('}');
            } else if (key == "criteriaComparisons") {
                p.expect('{');
                while (p.peek() != '}' && p.peek() != '\0') {
                    std::string pkStr = p.parseStr();
                    p.expect(':');
                    int val = static_cast<int>(p.parseInt());
                    auto u = pkStr.find('_');
                    if (u != std::string::npos) {
                        uint32_t a = static_cast<uint32_t>(std::stoul(pkStr.substr(0, u)));
                        uint32_t b = static_cast<uint32_t>(std::stoul(pkStr.substr(u + 1)));
                        s.criteriaComparisons[makePairKey(a, b)] = val;
                    }
                    if (p.peek() == ',') p.consume();
                }
                p.expect('}');
            } else {
                p.skipValue();
            }

            if (p.peek() == ',') p.consume();
        }
        p.expect('}');
    } catch (...) {
        // Fallback to whatever was parsed
    }

    return s;
}
