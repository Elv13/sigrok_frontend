#include "actioncollection.h"

#include <QtWidgets/QAction>

#include <KXmlGuiWindow>
#include <KActionCollection>

class ActionCollectionPrivate : public QObject
{
    Q_OBJECT
public:
    KXmlGuiWindow* m_pParent;
    ActionCollection* q_ptr;

    QAction* m_ZoomIn;
    QAction* m_ZoomOut;
    QAction* m_ZoomFit;
    QAction* m_ZoomReset;

public Q_SLOTS:
    void slotZoonIn();
    void slotZoonOut();
    void slotZoonFit();
    void slotZoonReset();
};

ActionCollection::ActionCollection(KXmlGuiWindow* parent) : QObject(parent),
    d_ptr(new ActionCollectionPrivate)
{
    d_ptr->m_pParent = parent;
    d_ptr->q_ptr     = this;

#define SELF &QAction::triggered, d_ptr, &ActionCollectionPrivate
    d_ptr->m_ZoomIn = new QAction(parent);
    d_ptr->m_ZoomIn->setText(tr("Zoom-In"));
    d_ptr->m_ZoomIn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
    parent->actionCollection()->addAction(QStringLiteral("zoom-in"), d_ptr->m_ZoomIn);
    parent->actionCollection()->setDefaultShortcuts(d_ptr->m_ZoomIn, {
        Qt::CTRL + Qt::Key_Plus,
        Qt::CTRL + Qt::Key_Equal
    });
    connect(d_ptr->m_ZoomIn, SELF::slotZoonIn);

    d_ptr->m_ZoomOut = new QAction(parent);
    d_ptr->m_ZoomOut->setText(tr("Zoom-Out"));
    d_ptr->m_ZoomOut->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    parent->actionCollection()->addAction(QStringLiteral("zoom-out"), d_ptr->m_ZoomOut);
    parent->actionCollection()->setDefaultShortcut(d_ptr->m_ZoomOut, Qt::CTRL + Qt::Key_Minus);
    connect(d_ptr->m_ZoomOut, SELF::slotZoonOut);

    d_ptr->m_ZoomFit = new QAction(parent);
    d_ptr->m_ZoomFit->setText(tr("Fit screen"));
    d_ptr->m_ZoomFit->setIcon(QIcon::fromTheme(QStringLiteral("zoom-fit-best")));
    parent->actionCollection()->addAction(QStringLiteral("zoom-fit"), d_ptr->m_ZoomFit);
    connect(d_ptr->m_ZoomFit, SELF::slotZoonFit);

    d_ptr->m_ZoomReset = new QAction(parent);
    d_ptr->m_ZoomReset->setText(tr("Reset zoom"));
    d_ptr->m_ZoomReset->setIcon(QIcon::fromTheme(QStringLiteral("zoom-page")));
    parent->actionCollection()->addAction(QStringLiteral("zoom-reset"), d_ptr->m_ZoomReset);
    connect(d_ptr->m_ZoomReset, SELF::slotZoonReset);
#undef SELF
}

ActionCollection::~ActionCollection()
{
    delete d_ptr;
}

void ActionCollectionPrivate::slotZoonIn()
{
    Q_EMIT q_ptr->zoomIn();
}

void ActionCollectionPrivate::slotZoonOut()
{
    Q_EMIT q_ptr->zoomOut();
}

void ActionCollectionPrivate::slotZoonFit()
{
    Q_EMIT q_ptr->zoomFit();
}

void ActionCollectionPrivate::slotZoonReset()
{
    Q_EMIT q_ptr->zoomReset();
}

#include "actioncollection.moc"
