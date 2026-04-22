#include "BibleWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>

BibleWidget::BibleWidget(BiblePluginManager &manager, QWidget *parent)
    : QWidget(parent), pluginManager(manager),
      bookComboBox(new QComboBox),
      chapterSpinBox(new QSpinBox),
      verseListView(new QListView),
      verseListModel(new QStandardItemModel(this)),
      prevButton(new QPushButton("◀︎")),
      nextButton(new QPushButton("▶︎")),
      currentBook(QString()),
      currentChapter(0)
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *controlsLayout = new QHBoxLayout;

    controlsLayout->addWidget(new QLabel("책:"));
    controlsLayout->addWidget(bookComboBox, 1);
    controlsLayout->addWidget(new QLabel("장:"));
    controlsLayout->addWidget(chapterSpinBox);

    mainLayout->addLayout(controlsLayout);

    verseListView->setModel(verseListModel);
    mainLayout->addWidget(verseListView, 1);

    auto *navLayout = new QHBoxLayout;
    navLayout->addStretch();
    navLayout->addWidget(prevButton);
    navLayout->addWidget(nextButton);
    navLayout->addStretch();
    mainLayout->addLayout(navLayout);

    connect(bookComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BibleWidget::onBookChanged);
    connect(chapterSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BibleWidget::onChapterChanged);
    connect(verseListView, &QListView::clicked,
            this, &BibleWidget::onVerseClicked);
    connect(prevButton, &QPushButton::clicked,
            this, &BibleWidget::onPreviousVerse);
    connect(nextButton, &QPushButton::clicked,
            this, &BibleWidget::onNextVerse);

    auto bookList = pluginManager.get_book_list();
    for (const auto &book : bookList) {
        bookComboBox->addItem(book);
    }

    if (!bookList.empty()) {
        bookComboBox->setCurrentIndex(0);
        onBookChanged(0);
    }

    verseListModel->clear();
}

BibleWidget::~BibleWidget() = default;

void BibleWidget::onBookChanged(int index)
{
    if (index < 0 || index >= bookComboBox->count())
        return;

    currentBook = bookComboBox->currentText();
    int maxChapter = pluginManager.get_chapter_count(currentBook);
    chapterSpinBox->setMaximum(maxChapter);
    chapterSpinBox->setValue(1);
}

void BibleWidget::onChapterChanged(int chapter)
{
    if (chapter < 1)
        return;

    currentChapter = chapter;
    loadVerseList(currentBook, currentChapter);
}

void BibleWidget::loadVerseList(const QString &bookName, int chapter)
{
    const int maxVersesPerChapter = 10000;
    auto verses = pluginManager.get_verses(bookName, chapter, 0, maxVersesPerChapter);
    currentVerses = verses;
    updateVerseList(verses);
    setCurrentVerseIndex(0);
}

void BibleWidget::updateVerseList(const std::vector<std::pair<int, QString>> &verses)
{
    verseListModel->clear();
    for (const auto &[verseNum, content] : verses) {
        QString displayText = QString("%1: %2").arg(verseNum).arg(content);
        auto *item = new QStandardItem(displayText);
        verseListModel->appendRow(item);
    }
}

void BibleWidget::setCurrentVerseIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(currentVerses.size()))
        return;

    verseListView->clearSelection();
    QModelIndex modelIndex = verseListModel->index(index, 0);
    verseListView->setCurrentIndex(modelIndex);
    verseListView->scrollTo(modelIndex);
}

int BibleWidget::currentVerseIndex() const
{
    QModelIndex current = verseListView->currentIndex();
    if (current.isValid())
        return current.row();
    return -1;
}

void BibleWidget::onVerseClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        setCurrentVerseIndex(index.row());
    }
}

void BibleWidget::onPreviousVerse()
{
    int current = currentVerseIndex();
    if (current > 0) {
        setCurrentVerseIndex(current - 1);
    }
}

void BibleWidget::onNextVerse()
{
    int current = currentVerseIndex();
    if (current >= 0 && current < static_cast<int>(currentVerses.size()) - 1) {
        setCurrentVerseIndex(current + 1);
    }
}