#ifndef BIBLE_WIDGET_H
#define BIBLE_WIDGET_H

#include <QWidget>
#include <QString>

class BiblePluginManager;
class QComboBox;
class QSpinBox;
class QListView;
class QStandardItemModel;
class QPushButton;

class BibleWidget : public QWidget {
    Q_OBJECT
public:
    explicit BibleWidget(BiblePluginManager &manager, QWidget *parent = nullptr);
    ~BibleWidget() override;

private slots:
    void onBookChanged(int index);
    void onChapterChanged(int chapter);
    void onVerseClicked(const QModelIndex &index);
    void onPreviousVerse();
    void onNextVerse();

private:
    void loadVerseList(const QString &bookName, int chapter);
    void updateVerseList(const std::vector<std::pair<int, QString>> &verses);
    void setCurrentVerseIndex(int index);
    int currentVerseIndex() const;

    BiblePluginManager &pluginManager;
    QComboBox *bookComboBox;
    QSpinBox *chapterSpinBox;
    QListView *verseListView;
    QStandardItemModel *verseListModel;
    QPushButton *prevButton;
    QPushButton *nextButton;

    QString currentBook;
    int currentChapter;
    std::vector<std::pair<int, QString>> currentVerses; // verse number, content
};

#endif // BIBLE_WIDGET_H