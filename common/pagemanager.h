#pragma once

#include <QtCore/QObject>

class QWidget;

class PageManager : public QObject
{
    Q_OBJECT

public:

    static PageManager* instance();

    void addPage(QWidget* w, const QString& title = QString());

Q_SIGNALS:
    void pageAdded(QWidget* page, const QString& title);

private:
    explicit PageManager();
    virtual ~PageManager();

};
