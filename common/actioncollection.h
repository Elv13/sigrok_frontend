#pragma once

#include <QtCore/QObject>

class ActionCollectionPrivate;
class KXmlGuiWindow;

class ActionCollection : public  QObject
{
    Q_OBJECT
public:
    explicit ActionCollection(KXmlGuiWindow* parent = nullptr);
    virtual ~ActionCollection();

Q_SIGNALS:
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomReset();
    void paste();

private:
    ActionCollectionPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ActionCollection)
};
