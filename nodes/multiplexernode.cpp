#include "multiplexernode.h"


class MultiplexerNodePrivate final : public QObject
{
public:

    MultiplexerNode* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

MultiplexerNode::MultiplexerNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new MultiplexerNodePrivate())
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &MultiplexerNodePrivate::slotModelChanged);
}

MultiplexerNode::~MultiplexerNode()
{
    delete d_ptr;
}

QString MultiplexerNode::title() const
{
    return "Multiplexer";
}

QString MultiplexerNode::id() const
{
    return QStringLiteral("multiplexer_node");
}

void MultiplexerNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* MultiplexerNode::widget() const
{
    return nullptr;
}

void MultiplexerNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    Q_EMIT q_ptr->modelChangedCopy(newModel, old);
}
