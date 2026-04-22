#ifndef BIBLE_RESULT_HANDLER_H
#define BIBLE_RESULT_HANDLER_H

#include <QString>
#include "BiblePluginSettings.h"

class BibleResultHandler {
public:
    BibleResultHandler() = default;
    ~BibleResultHandler() = default;

    // Format a single verse for display.
    // Parameters:
    //   verseNumber: the verse number (integer)
    //   content: the raw verse text from the database
    //   settings: the current plugin settings (for font size, etc. - though we might not use all here)
    // Returns: a formatted string ready to be displayed in the UI.
    QString formatVerse(int verseNumber, const QString &content, const BiblePluginSettings &settings) const;

    // Optionally, we could have a method to format a list of verses, but we can do that in the UI or in the manager.
};

#endif // BIBLE_RESULT_HANDLER_H
