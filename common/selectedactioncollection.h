#pragma once

#include <QtCore/QObject>

#ifdef WITH_XMLGUI
 class KXmlGuiWindow;
 class Session;
#else
 #include <QtWidgets/QMainWindow>
 typedef QMainWindow KXmlGuiWindow;
#endif

class GraphicsNode;

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

#ifdef WITH_XMLGUI
    GraphicsNode* currentNode() const;

public Q_SLOTS:
    void sessionChanged(Session* s);
    void currentChanged(GraphicsNode* n);
#endif

private:
    SelectedActionCollectionPrivate* d_ptr;
    Q_DECLARE_PRIVATE(SelectedActionCollection)
};
