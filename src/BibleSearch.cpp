#include "BibleSearch.h"

QString make_search_pattern(const QString &raw)
{
    QString s = raw.trimmed();
    if (s.isEmpty()) return {};

    bool has_digit = false;
    QString escaped;
    escaped.reserve(s.size() * 2);
    for (QChar c : s) {
        if (c.isDigit()) has_digit = true;
        if (c == '\\' || c == '%' || c == '_')
            escaped.append('\\');
        escaped.append(c);
    }
    escaped.append(has_digit ? ":%" : "%");
    return escaped;
}
