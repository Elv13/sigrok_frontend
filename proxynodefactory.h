#pragma once

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/graphicsnodescene.hpp"
#include "qt5-node-editor/src/graphicsbezieredge.hpp"
#include "qt5-node-editor/src/qobjectnode.hpp"
#include "qt5-node-editor/src/graphicsnodesocket.hpp"

#include <QtCore/QDebug>
#include <QtCore/QAbstractItemModel>

#include <QtGui/QIcon>

#include "nodes/abstractnode.h"

class QAbstractItemModel;
class GraphicsNodeScene;

class ProxyNodeFactoryAdapter : public QAbstractItemModel
{
public:
    explicit ProxyNodeFactoryAdapter(GraphicsNodeScene* scene);
    void registerNode(AbstractNode* o);

    template<typename T>
    void registerType(const QString& name, const QString& cat, const QIcon& icon = {});


    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;

public Q_SLOTS:
    QPair<QObjectnode*, AbstractNode*> addToScene(const QModelIndex& idx);

private:
    GraphicsNodeScene* m_pScene;
    typedef struct MetaInfo {
        const QMetaObject m_spMetaObj;
        QString m_Name;
        QIcon m_Icon;
        QVector< QPair<QObjectnode*, AbstractNode*> > m_lInstances;
    } MetaInfo;

    typedef struct Category {
        QString m_Name;
        QVector<MetaInfo*> m_lTypes;
    };

    QVector<Category*> m_slCategory;
};

template<typename T>
void ProxyNodeFactoryAdapter::registerType(const QString& name, const QString& category, const QIcon& icon )
{
    // Look for the category
    Category* cat = nullptr;
    Q_FOREACH(Category* c, m_slCategory) {
        if (c->m_Name == category) {
            cat = c;
            break;
        }
    }

    if (!cat) {
        cat = new Category {
            category,
            {}
        };
        m_slCategory << cat;
    }

    cat->m_lTypes << new ProxyNodeFactoryAdapter::MetaInfo {
        T::staticMetaObject, name, icon
    };
}
