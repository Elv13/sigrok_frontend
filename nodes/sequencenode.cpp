#include "sequencenode.h"

#include <QtCore/QStringListModel>
#include <QtCore/QJsonArray>
#include <QtCore/QDebug>

#include "widgets/sequence.h"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class SequenceNodePrivate : public QObject
{
    Q_OBJECT
public:
    bool        m_Looping {false};
    int         m_Current {0};
    Sequence*   m_pWidget {nullptr};
    QStringListModel m_Model {this};

    // Helpers
    QString first() const;
    QString last() const;
    QString at(int i) const;

    SequenceNode* q_ptr;
public Q_SLOTS:
    void slotCurrentChanged(const QModelIndex& idx);
};

SequenceNode::SequenceNode(AbstractSession* sess)
    : AbstractNode(sess), d_ptr(new SequenceNodePrivate)
{
    d_ptr->q_ptr = this;
}

SequenceNode::~SequenceNode()
{
    delete d_ptr;
}

QWidget* SequenceNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Sequence();
        d_ptr->m_pWidget->setModel(&d_ptr->m_Model);
        connect(d_ptr->m_pWidget, &Sequence::currentChanged,
            d_ptr, &SequenceNodePrivate::slotCurrentChanged);
    }

    return d_ptr->m_pWidget;
}

void SequenceNode::forward(bool)
{
    if (!d_ptr->m_Model.rowCount())
        return;

    if (d_ptr->m_Looping && d_ptr->m_Current == d_ptr->m_Model.rowCount()-1) {
        setCurrent(0);
        return;
    }

    if (d_ptr->m_Current == d_ptr->m_Model.rowCount()-1)
        return;

    setCurrent(d_ptr->m_Current+1);
}

void SequenceNode::backward(bool)
{
    if (!d_ptr->m_Model.rowCount())
        return;


    if (d_ptr->m_Looping && !d_ptr->m_Current)
        return setCurrent(d_ptr->m_Model.rowCount()-1);

    if (!d_ptr->m_Current)
        return;

    setCurrent(d_ptr->m_Current-1);
}

void SequenceNode::reset(bool)
{
    if (!d_ptr->m_Model.rowCount())
        return;

    setCurrent(0);
}

QString SequenceNode::next() const
{
    if (!d_ptr->m_Model.rowCount())
        return QString();

    if (d_ptr->m_Looping && d_ptr->m_Current == d_ptr->m_Model.rowCount()-1)
        return d_ptr->first();

    if (d_ptr->m_Current == d_ptr->m_Model.rowCount()-1)
        return QString();

    return d_ptr->at(d_ptr->m_Current+1);
}

QString SequenceNode::previous() const
{
    if (!d_ptr->m_Model.rowCount())
        return QString();

    if (d_ptr->m_Looping && !d_ptr->m_Current)
        return d_ptr->last();

    if (!d_ptr->m_Current)
        return QString();

    return d_ptr->at(d_ptr->m_Current-1);
}

void SequenceNode::add(const QString& name)
{
    const int r = d_ptr->m_Model.rowCount();
    d_ptr->m_Model.insertRows(r, 1, {});
    d_ptr->m_Model.setData(d_ptr->m_Model.index(r,0), name, Qt::EditRole);
}

bool SequenceNode::isLooping() const
{
    return d_ptr->m_Looping;
}

void SequenceNode::setLooping(bool value)
{
    d_ptr->m_Looping = value;
}

bool SequenceNode::hasEnded() const
{
    return !((d_ptr->m_Looping) || d_ptr->m_Current < d_ptr->m_Model.rowCount());
}

int SequenceNode::currentIndex() const
{
    return d_ptr->m_Current;
}

void SequenceNode::setCurrent(int v)
{
    if (v == d_ptr->m_Current || v < 0 || v >= d_ptr->m_Model.rowCount())
        return;

    const auto old = d_ptr->m_Current;

    d_ptr->m_Current = v;

    if (d_ptr->m_pWidget)
        d_ptr->m_pWidget->setCurrent(d_ptr->m_Model.index(v, 0));

    Q_EMIT currentIndexChanged(v);
    Q_EMIT previousChanged(previous());
    Q_EMIT nextChanged(next());

    if (v == d_ptr->m_Model.rowCount() - 1)
        Q_EMIT endedChanged(true);
    else if (old == d_ptr->m_Model.rowCount() -1)
        Q_EMIT endedChanged(false);
}

void SequenceNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    QJsonArray elements();

    const auto t = d_ptr->m_Model.stringList();
//    for (const auto& v : qAsConst(t))
//        elements << v;

    parent[QStringLiteral("elements")] = elements;
    parent[QStringLiteral("looping") ] = d_ptr->m_Looping;
}

void SequenceNode::read(const QJsonObject &parent)
{
    AbstractNode::read(parent);

    const auto elems = parent[QStringLiteral("elements")].toArray();

    QStringList elements;

    for (int i = 0; i < elems.size(); ++i)
        elements << elems[i].toString();

    d_ptr->m_Model.setStringList(elements);
    d_ptr->m_Looping = true;
}

QString SequenceNodePrivate::first() const
{
    return m_Model.index(0,0).data().toString();
}

QString SequenceNodePrivate::last() const
{
    return m_Model.index(m_Model.rowCount()-1,0).data().toString();
}

QString SequenceNodePrivate::at(int i) const
{
    return m_Model.index(i,0).data().toString();
}

void SequenceNodePrivate::slotCurrentChanged(const QModelIndex& idx)
{
    if (idx.row() != m_Current)
       q_ptr->setCurrent(idx.row());
}

#include <sequencenode.moc>
