#ifndef __QOBJECTNODE_HPP__FF345507_3840_47A5_BE60_D24FFF0BEE7F
#define __QOBJECTNODE_HPP__FF345507_3840_47A5_BE60_D24FFF0BEE7F

#include "graphicsnode.hpp"

class QObjectnodePrivate;

class PropertyConnection final : public QObject //TODO make private
{
    Q_OBJECT
public:
    explicit PropertyConnection(QObject* parent);
    ~PropertyConnection();

    const QMetaObject* _source_mo;
    QMetaObject::Connection _conn;
    int _prop_id;
    QObjectnodePrivate* d_ptr;
};
Q_DECLARE_METATYPE(PropertyConnection*)


class QObjectnode : public GraphicsNode
{
public:
	QObjectnode(QObject* data, QGraphicsItem *parent = nullptr);

private:
	QObjectnodePrivate* d_ptr;
	Q_DECLARE_PRIVATE(QObjectnode)
};

#endif /* __QOBJECTNODE_HPP__FF345507_3840_47A5_BE60_D24FFF0BEE7F */

