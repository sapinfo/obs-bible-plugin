#include "BibleResultHandler.h"

QString BibleResultHandler::formatVerse(int verseNumber, const QString &content, const BiblePluginSettings &settings) const {
    return QString("%1: %2").arg(verseNumber).arg(content);
}
