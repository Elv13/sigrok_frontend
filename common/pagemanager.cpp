#include "pagemanager.h"

#include <QtWidgets/QWidget>

PageManager* PageManager::instance()
{
    static auto i = new PageManager;

    return i;
}

static QList<QWidget*>& getPages()
{
    static QList<QWidget*> pages;

    return pages;
}

void PageManager::addPage(QWidget* w, const QString& title)
{
    getPages() << w;
    Q_EMIT pageAdded(w, title);
}

PageManager::PageManager()
{
    
}

PageManager::~PageManager()
{
    
}
