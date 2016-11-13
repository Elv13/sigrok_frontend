#include "nodeadapter.h"


#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/graphicsnodescene.hpp"
#include "qt5-node-editor/src/graphicsbezieredge.hpp"
#include "nodes/devicenode.h"
#include <QtCore/QAbstractItemModel>

NodeAdapter::NodeAdapter(QAbstractItemModel* model, GraphicsNodeScene* scene, int column, int role)
{
    Q_UNUSED(model)
    Q_UNUSED(scene)
    Q_UNUSED(column)
    Q_UNUSED(role)
    /*auto l = [model, scene, role](const QModelIndex& idx) {

        auto t1 = qvariant_cast<DeviceNode*>(idx.data(role));

        if (!t1)
            t1 = new DeviceNode();

        QObjectnode* n2 = new QObjectnode(t1);

        n2->setTitle(t1->title());

        auto w = t1->widget();
        n2->setCentralWidget(w);

        scene->addItem(n2);
        n2->setPos(0,0);

        QObject::connect(t1, &DeviceNode::titleChanged, n2, &GraphicsNode::setTitle);
    };

    for (int i=0; i < model->rowCount(); i++) {
        l(model->index(i,0));
    }

    QObject::connect(model, &QAbstractItemModel::rowsInserted, l);*/ //FIXME dead code
}
