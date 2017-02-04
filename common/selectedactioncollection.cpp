#include "selectedactioncollection.h"

#include <QtCore/QDebug>
#include <QtWidgets/QAction>

#include <KXmlGuiWindow>
#include <KActionCollection>

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

    QAction* m_pBg;
    QAction* m_pFg;
    QAction* m_pCopy;
    QAction* m_pCut;
    QAction* m_pPaste;

public Q_SLOTS:
    void slotBg();
    void slotFg();
    void slotCopy();
    void slotCut();
    void slotPaste();
};

SelectedActionCollection::SelectedActionCollection(KXmlGuiWindow* parent) : QObject(parent),
    d_ptr(new SelectedActionCollectionPrivate)
{
#define SELF &QAction::triggered, d_ptr, &SelectedActionCollectionPrivate
    d_ptr->m_pParent = parent;

    //actionCollection()->setDefaultShortcut(d_ptr->m_pBg, Qt::CTRL + Qt::Key_W);

    d_ptr->m_pBg = new QAction(parent);
    d_ptr->m_pBg->setText(tr("&Background"));
    d_ptr->m_pBg->setIcon(QIcon::fromTheme("color-fill"));
    parent->actionCollection()->addAction("background", d_ptr->m_pBg);
    d_ptr->m_pBg->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pBg;
    connect(d_ptr->m_pBg, SELF::slotBg);

    d_ptr->m_pFg = new QAction(parent);
    d_ptr->m_pFg->setText(tr("&Foreground"));
    d_ptr->m_pFg->setIcon(QIcon::fromTheme("color-picker"));
    parent->actionCollection()->addAction("foreground", d_ptr->m_pFg);
    d_ptr->m_pFg->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pFg;
    connect(d_ptr->m_pBg, SELF::slotFg);

    d_ptr->m_pCopy = new QAction(parent);
    d_ptr->m_pCopy->setText(tr("Copy"));
    d_ptr->m_pCopy->setIcon(QIcon::fromTheme("edit-copy"));
    parent->actionCollection()->addAction("copy", d_ptr->m_pCopy);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pCopy, Qt::CTRL + Qt::Key_C);
    d_ptr->m_pCopy->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pCopy;
    connect(d_ptr->m_pBg, SELF::slotCopy);

    d_ptr->m_pCut = new QAction(parent);
    d_ptr->m_pCut->setText(tr("Cut"));
    d_ptr->m_pCut->setIcon(QIcon::fromTheme("edit-cut"));
    parent->actionCollection()->addAction("cut", d_ptr->m_pCut);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pCut, Qt::CTRL + Qt::Key_X);
    d_ptr->m_pCut->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pCut;
    connect(d_ptr->m_pBg, SELF::slotCut);

    d_ptr->m_pPaste = new QAction(parent);
    d_ptr->m_pPaste->setText(tr("Paste"));
    d_ptr->m_pPaste->setIcon(QIcon::fromTheme("edit-paste"));
    parent->actionCollection()->addAction("paste", d_ptr->m_pPaste);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_pPaste, Qt::CTRL + Qt::Key_V);
    d_ptr->m_pPaste->setDisabled(true);
    d_ptr->m_lActions << d_ptr->m_pPaste;
    connect(d_ptr->m_pBg, SELF::slotPaste);
#undef SELF
}

SelectedActionCollection::~SelectedActionCollection()
{
    delete d_ptr;
}

void SelectedActionCollection::currentChanged(GraphicsNode* n)
{
    for (auto a : qAsConst(d_ptr->m_lActions)) {
        a->setEnabled(!!n);
    }
}


void SelectedActionCollectionPrivate::slotBg()
{

}

void SelectedActionCollectionPrivate::slotFg()
{

}

void SelectedActionCollectionPrivate::slotCopy()
{

}

void SelectedActionCollectionPrivate::slotCut()
{

}

void SelectedActionCollectionPrivate::slotPaste()
{

}

#include "selectedactioncollection.moc"
