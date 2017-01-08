#pragma once

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/graphicsnodescene.hpp"
#include "qt5-node-editor/src/graphicsbezieredge.hpp"
#include "qt5-node-editor/src/graphicsnodesocket.hpp"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QAbstractItemModel>

#include <QtGui/QIcon>

#include "common/abstractnode.h"

class QNodeWidget;
class QAbstractItemModel;
class GraphicsNodeScene;
class QIODevice;

class ProxyNodeFactoryAdapter : public QAbstractItemModel
{
public:
    explicit ProxyNodeFactoryAdapter(QNodeWidget* w);

    template<typename T>
    void registerType(const QString& name, const QString& id, const QString& cat, const QIcon& icon = {});


    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;

    void serialize(QIODevice *dev) const;
    void load(QIODevice *dev);
    void load(const QByteArray& data);
    QPair<GraphicsNode*, AbstractNode*> addToSceneFromMetaObject(const QMetaObject& o, const QString& uid = {});

public Q_SLOTS:
    QPair<GraphicsNode*, AbstractNode*> addToScene(const QModelIndex& idx);

private Q_SLOTS:
    void remove(const QObject* o, const QString& id);

private:
    QNodeWidget* m_pNodeW;
    struct MetaInfo {
        int m_Index;
        const QMetaObject m_spMetaObj;
        QString m_Name;
        QIcon m_Icon;
        QVector< QPair<GraphicsNode*, AbstractNode*> > m_lInstances;
    };

    struct Category {
        QString m_Name;
        QVector<MetaInfo*> m_lTypes;
    };

    QVector<Category*> m_slCategory;
    QHash<QString, MetaInfo*> m_hIdToType;
};

template<typename T>
void ProxyNodeFactoryAdapter::registerType(const QString& name, const QString& category, const QString& id, const QIcon& icon )
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

    auto mi = new ProxyNodeFactoryAdapter::MetaInfo {
        cat->m_lTypes.size(), T::staticMetaObject, name, icon, {}
    };

    cat->m_lTypes << mi;

    m_hIdToType[id] = mi;
}
