#include "BibleWidget.h"
#include "../BiblePluginManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <obs-module.h>

BibleWidget::BibleWidget(QWidget *parent)
    : QWidget(parent),
      searchEdit(new QLineEdit),
      searchBtn(new QPushButton),
      verseView(new QListView),
      verseModel(new QStandardItemModel(this)),
      toggleBtn(new QPushButton),
      bookView(new QListView),
      bookModel(new QStandardItemModel(this)) {
    searchEdit->setPlaceholderText(obs_module_text("BibleSearchPlaceholder"));
    searchBtn->setText(obs_module_text("BibleSearchButton"));
    toggleBtn->setText(obs_module_text("BibleToggleButton"));

    verseView->setModel(verseModel);
    verseView->setSelectionMode(QAbstractItemView::SingleSelection);
    verseView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    bookView->setModel(bookModel);
    bookView->setSelectionMode(QAbstractItemView::SingleSelection);
    bookView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 좌측 컨테이너
    auto *left = new QWidget(this);
    auto *leftLayout = new QVBoxLayout(left);
    auto *searchRow  = new QHBoxLayout;
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(searchBtn);
    leftLayout->addLayout(searchRow);
    leftLayout->addWidget(verseView, 1);
    leftLayout->addWidget(toggleBtn);

    // 우측 컨테이너 (책 리스트만)
    auto *right = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->addWidget(bookView, 1);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto *main = new QVBoxLayout(this);
    main->addWidget(splitter, 1);

    // 시그널 연결
    connect(searchEdit, &QLineEdit::returnPressed, this, &BibleWidget::onSearchTriggered);
    connect(searchBtn,  &QPushButton::clicked,     this, &BibleWidget::onSearchTriggered);
    connect(bookView,   &QListView::clicked,       this, &BibleWidget::onBookClicked);
    connect(toggleBtn,  &QPushButton::clicked,     this, &BibleWidget::onToggleVisibleClicked);
    connect(verseView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &BibleWidget::onVerseCurrentChanged);

    auto *mgr = BiblePluginManager::instance();
    connect(mgr, &BiblePluginManager::list_changed,
            this, &BibleWidget::onListChanged);
    connect(mgr, &BiblePluginManager::verse_changed,
            this, &BibleWidget::onVerseChanged);

    populateBookList();
    searchEdit->setText(mgr->current_query());
    refreshVerseList();
    syncSelectionFromManager();
}

BibleWidget::~BibleWidget() = default;

void BibleWidget::populateBookList() {
    bookModel->clear();
    for (const auto &b : BiblePluginManager::instance()->get_books()) {
        auto *it = new QStandardItem(b.name + ":" + b.abbr);
        it->setData(b.abbr, Qt::UserRole + 1);
        bookModel->appendRow(it);
    }
}

void BibleWidget::onSearchTriggered() {
    BiblePluginManager::instance()->apply_search(searchEdit->text());
}

void BibleWidget::onBookClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    QString abbr = index.data(Qt::UserRole + 1).toString();
    searchEdit->setText(abbr);
    BiblePluginManager::instance()->apply_search(abbr);
}

void BibleWidget::onVerseCurrentChanged(const QModelIndex &current, const QModelIndex &) {
    if (!current.isValid()) return;
    suppress_selection_feedback = true;
    BiblePluginManager::instance()->select_index(current.row());
    suppress_selection_feedback = false;
}

void BibleWidget::onToggleVisibleClicked() {
    auto *m = BiblePluginManager::instance();
    m->set_visible(!m->visible());
}

void BibleWidget::onListChanged() {
    refreshVerseList();
}

void BibleWidget::onVerseChanged(QString, QString, bool) {
    if (suppress_selection_feedback) return;
    syncSelectionFromManager();
}

void BibleWidget::refreshVerseList() {
    verseModel->clear();
    const auto &list = BiblePluginManager::instance()->filtered_list();
    for (const auto &r : list) {
        QString line = r.verse_ref + "  " + r.content;
        verseModel->appendRow(new QStandardItem(line));
    }
}

void BibleWidget::syncSelectionFromManager() {
    int i = BiblePluginManager::instance()->current_index();
    if (i < 0 || i >= verseModel->rowCount()) {
        verseView->clearSelection();
        return;
    }
    QModelIndex idx = verseModel->index(i, 0);
    verseView->setCurrentIndex(idx);
    verseView->scrollTo(idx);
}
