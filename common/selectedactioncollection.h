#pragma once

#include <QtCore/QObject>

class KXmlGuiWindow;

class GraphicsNode;
class Session;

class QAction;

class SelectedActionCollectionPrivate;

class SelectedActionCollection : public  QObject
{
    Q_OBJECT
public:
    explicit SelectedActionCollection(KXmlGuiWindow* parent = nullptr);
    virtual ~SelectedActionCollection();

    QAction* bg();
    QAction* fg();
    QAction* copy();
    QAction* cut();
    QAction* deleteNode();
    QAction* rename();

    GraphicsNode* currentNode() const;

public Q_SLOTS:
    void sessionChanged(Session* s);
    void currentChanged(GraphicsNode* n);

private:
    SelectedActionCollectionPrivate* d_ptr;
    Q_DECLARE_PRIVATE(SelectedActionCollection)
};
