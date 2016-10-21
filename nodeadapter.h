#pragma once

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/graphicsnodescene.hpp"
#include "qt5-node-editor/src/graphicsbezieredge.hpp"
#include "qt5-node-editor/src/qobjectnode.hpp"

#include <QtCore/QDebug>
#include <QtCore/QAbstractItemModel>

class QAbstractItemModel;
class GraphicsNodeScene;

template<typename T>
class NodeAdapter
{
public:
    explicit NodeAdapter(QAbstractItemModel* model, GraphicsNodeScene* scene, int column, int role);
};

template<typename T>
NodeAdapter<T>::NodeAdapter(QAbstractItemModel* model, GraphicsNodeScene* scene, int column, int role)
{
    auto l = [model, scene, role](const QModelIndex& idx) {

        auto t1 = qvariant_cast<T*>(idx.data(role));

        if (!t1)
            t1 = new T();

        QObjectnode* n2 = new QObjectnode(t1);

        n2->setTitle(t1->title());

        n2->add_sink("device");

        auto w = t1->widget();
        n2->setCentralWidget(w);

        scene->addItem(n2);
        n2->setPos(0,0);
    };

    for (int i=0; i < model->rowCount(); i++) {
        l(model->index(i,0));
    }

    QObject::connect(model, &QAbstractItemModel::rowsInserted, l);
}
