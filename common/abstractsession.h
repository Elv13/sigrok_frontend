#pragma once

#include <QtCore/QAbstractItemModel>

class PageManager;

class AbstractSession : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AbstractSession(QObject* parent);

    virtual PageManager* pages() const = 0;
};
