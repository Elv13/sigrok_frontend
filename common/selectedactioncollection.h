#pragma once

#include <QtCore/QObject>

class KXmlGuiWindow;

class GraphicsNode;
class Session;

class SelectedActionCollectionPrivate;

class SelectedActionCollection : public  QObject
{
    Q_OBJECT
public:
    explicit SelectedActionCollection(KXmlGuiWindow* parent = nullptr);
    virtual ~SelectedActionCollection();

public Q_SLOTS:
    void sessionChanged(Session* s);
    void currentChanged(GraphicsNode* n);

private:
    SelectedActionCollectionPrivate* d_ptr;
    Q_DECLARE_PRIVATE(SelectedActionCollection)
};
