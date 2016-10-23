#pragma once

#include <QtCore/QDebug>


class QAbstractItemModel;
class GraphicsNodeScene;

class NodeAdapter
{
public:
    explicit NodeAdapter(QAbstractItemModel* model, GraphicsNodeScene* scene, int column, int role);
};
