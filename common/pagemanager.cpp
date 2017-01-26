#include "pagemanager.h"

#include <QtWidgets/QWidget>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include "remotemanager.h"

#include "../common/abstractnode.h"

struct PageNode
{
    QWidget*      widget;
    QString       title;
    QString       remoteObjectName;
    QString       remoteModelName;
    AbstractNode* n;
};

class PageManagerPrivate
{
public:
    QList<PageNode*> pages;
};

void PageManager::addPage(AbstractNode* n, QWidget* w, const QString& title, const QString& uid)
{
    beginInsertRows({}, d_ptr->pages.size(), d_ptr->pages.size());
    d_ptr->pages << new PageNode { w, title, "SS", "FF", n};
    endInsertRows();

    Q_EMIT pageAdded(w, title, uid);
}

void PageManager::removePage(AbstractNode* n)
{
    for (int i = 0; i < d_ptr->pages.size(); i++) {
        auto element = d_ptr->pages[i];
        if (element->n == n) {
            beginRemoveRows({}, i, i);
            d_ptr->pages.removeAt(i);
            delete element;
            endRemoveRows();
            break;
        }
    }
}

// void PageManager::removePage(const QString& uid)
// {
//     for (int i = 0; i < d_ptr->pages.size(); i++) {
//         auto element = d_ptr->pages[i];
//         if (element->n->uid() == uid) {
//             beginRemoveRows({}, i, i);
//             d_ptr->pages.removeAt(i);
//             delete element;
//             endRemoveRows();
//             break;
//         }
//     }
// }

PageManager::PageManager() : d_ptr(new PageManagerPrivate)
{
    RemoteManager::instance()->addModel(this, {
        Qt::DisplayRole,
        Qt::EditRole,
        Role::REMOTE_WIDGET_TYPE,
        Role::REMOTE_MODEL_NAME,
        Role::REMOTE_OBJECT_NAME,
        Role::REMOTE_OBJECT_UID
    }, QStringLiteral("PageManager"));
}

PageManager::~PageManager()
{
    
}

QVariant PageManager::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto i = d_ptr->pages[idx.row()];

    Q_ASSERT(i);
    Q_ASSERT(i->n);

    switch(role) {
        case Qt::DisplayRole:
            return i->title;
        case Qt::EditRole:
        case Role::REMOTE_MODEL_NAME:
            return i->n->remoteModelName();
        case Role::REMOTE_OBJECT_NAME:
            return i->n->remoteObjectName();
        case Role::REMOTE_WIDGET_TYPE:
            return i->n->remoteWidgetType();
        case Role::REMOTE_OBJECT_UID:
            Q_ASSERT(!i->n->uid().isEmpty());
            return i->n->uid();
    }

    return {};
}

int PageManager::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->pages.size();
}

bool PageManager::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    Q_UNUSED(idx);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}
