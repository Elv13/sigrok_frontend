#include "selectedactioncollection.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QAction>

#ifdef WITH_XMLGUI
 #include <KXmlGuiWindow>
 #include <KActionCollection>
 #include "../session.h" //FIXME
#else
 class Session;
#endif


#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class SelectedActionCollectionPrivate : public QObject
{
    Q_OBJECT
public:
    QList<QAction*> m_lActions;
    KXmlGuiWindow* m_pParent;

    GraphicsNode* m_pCurrentNode {nullptr};
    Session* m_pSession {nullptr};

    QAction* m_pBg;
    QAction* m_pFg;
    QAction* m_pCopy;
    QAction* m_pCut;
    QAction* m_pDelete;
    QAction* m_pRename;

public Q_SLOTS:
    void slotCopy();
    void slotCut();
    void slotDelete();
    void slotRename();
};

SelectedActionCollection::SelectedActionCollection(KXmlGuiWindow* parent) : QObject(parent),
    d_ptr(new SelectedActionCollectionPrivate)
{
#define SELF &QAction::triggered, d_ptr, &SelectedActionCollectionPrivate
    d_ptr->m_pParent = parent;

    d_ptr->m_pBg = new QAction(parent);
    d_ptr->m_pBg->setText(tr("&Background"));
    d_ptr->m_pBg->setIcon(QIcon::fromTheme(QStringLiteral("color-fill")));
#ifdef WITH_XMLGUI
    parent->actionCollection()->addAction(QStringLiteral("background"), d_ptr->m_pBg);
#endif
    d_ptr->m_pBg->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pBg;

    d_ptr->m_pFg = new QAction(parent);
    d_ptr->m_pFg->setText(tr("&Foreground"));
    d_ptr->m_pFg->setIcon(QIcon::fromTheme(QStringLiteral("color-picker")));
#ifdef WITH_XMLGUI
    parent->actionCollection()->addAction(QStringLiteral("foreground"), d_ptr->m_pFg);
#endif
    d_ptr->m_pFg->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pFg;

    d_ptr->m_pCopy = new QAction(parent);
    d_ptr->m_pCopy->setText(tr("Copy"));
    d_ptr->m_pCopy->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
#ifdef WITH_XMLGUI
    parent->actionCollection()->addAction(QStringLiteral("copy"), d_ptr->m_pCopy);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pCopy, Qt::CTRL + Qt::Key_C);
#endif
    d_ptr->m_pCopy->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pCopy;
    connect(d_ptr->m_pCopy, SELF::slotCopy);

    d_ptr->m_pCut = new QAction(parent);
    d_ptr->m_pCut->setText(tr("Cut"));
    d_ptr->m_pCut->setIcon(QIcon::fromTheme(QStringLiteral("edit-cut")));
#ifdef WITH_XMLGUI
    parent->actionCollection()->addAction(QStringLiteral("cut"), d_ptr->m_pCut);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pCut, Qt::CTRL + Qt::Key_X);
#endif
    d_ptr->m_pCut->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pCut;
    connect(d_ptr->m_pCut, SELF::slotCut);

    d_ptr->m_pDelete = new QAction(parent);
    d_ptr->m_pDelete->setText(tr("Delete"));
    d_ptr->m_pDelete->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
#ifdef WITH_XMLGUI
    parent->actionCollection()->addAction(QStringLiteral("delete"), d_ptr->m_pDelete);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pDelete, Qt::SHIFT + Qt::Key_Delete);
#endif
    d_ptr->m_pDelete->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pDelete;
    connect(d_ptr->m_pDelete, SELF::slotDelete);

    d_ptr->m_pRename = new QAction(parent);
    d_ptr->m_pRename->setText(tr("Rename"));
    d_ptr->m_pRename->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
#ifdef WITH_XMLGUI
    parent->actionCollection()->addAction(QStringLiteral("rename"), d_ptr->m_pRename);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pRename, Qt::CTRL + Qt::Key_R);
#endif
    d_ptr->m_pRename->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pRename;
    connect(d_ptr->m_pRename, SELF::slotRename);
#undef SELF
}

SelectedActionCollection::~SelectedActionCollection()
{
    delete d_ptr;
}


#ifdef WITH_XMLGUI
GraphicsNode* SelectedActionCollection::currentNode() const
{
    return d_ptr->m_pCurrentNode;
}

void SelectedActionCollection::currentChanged(GraphicsNode* n)
{
    for (auto a : qAsConst(d_ptr->m_lActions)) {
        a->setEnabled(!!n);
    }

    d_ptr->m_pCurrentNode = n;
}
void SelectedActionCollection::sessionChanged(Session* s)
{
    d_ptr->m_pSession = s;
}
#endif

void SelectedActionCollectionPrivate::slotCopy()
{
#ifdef WITH_XMLGUI
    if ((!m_pCurrentNode) || (!m_pSession)) return;

    auto an = m_pSession->fromGraphicsNode(m_pCurrentNode);

    if (!an)
        return;

    // Serialize
    const QByteArray a = m_pSession->serializeSelection();

    static QClipboard* c = QGuiApplication::clipboard();

    QMimeData* md = new QMimeData();
    md->setData(QStringLiteral("x-tutorial4/x-nodes-content"), a);

    c->setMimeData(md);
#endif

}

void SelectedActionCollectionPrivate::slotCut()
{
    if ((!m_pCurrentNode) || (!m_pSession)) return;

    slotCopy();
    slotDelete();

}

void SelectedActionCollectionPrivate::slotDelete()
{
    if ((!m_pCurrentNode) || (!m_pSession)) return;

#ifdef WITH_XMLGUI
    m_pSession->forEachSelected([](auto gn, auto) {
        gn->remove();
    });
#endif
}

void SelectedActionCollectionPrivate::slotRename()
{
    if ((!m_pCurrentNode) || (!m_pSession)) return;

#ifdef WITH_XMLGUI
    m_pCurrentNode->openTitleEditor();
#endif
}

QAction* SelectedActionCollection::bg()         { return d_ptr->m_pBg;     }
QAction* SelectedActionCollection::fg()         { return d_ptr->m_pFg;     }
QAction* SelectedActionCollection::copy()       { return d_ptr->m_pCopy;   }
QAction* SelectedActionCollection::cut()        { return d_ptr->m_pCut;    }
QAction* SelectedActionCollection::deleteNode() { return d_ptr->m_pDelete; }
QAction* SelectedActionCollection::rename()     { return d_ptr->m_pRename; }

#include "selectedactioncollection.moc"
