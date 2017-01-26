#pragma once

#include "common/abstractsession.h"

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

class InterfaceSerializer;
class SessionPrivate;

class Session : public AbstractSession
{
    Q_OBJECT
public:
    explicit Session(QNodeWidget* w);

    template<typename T>
    void registerType(const QString& name, const QString& id, const QString& cat, const QIcon& icon = {});

    void registerInterfaceSerializer(InterfaceSerializer* ser);

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& idx) const override;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const override;
    virtual QStringList mimeTypes() const override;

    void serialize(QIODevice *dev) const;
    void load(QIODevice *dev);
    void load(const QByteArray& data);

    virtual PageManager* pages() const override;

public Q_SLOTS:
    QPair<GraphicsNode*, AbstractNode*> addToScene(const QModelIndex& idx);

private Q_SLOTS:
    void remove(const QObject* o, const QString& id);
    void renameN(const QString& n, const QString& name);

private:
    QPair<GraphicsNode*, AbstractNode*> addToSceneFromMetaObject(const QMetaObject& o, const QString& uid = {});

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
    SessionPrivate* d_ptr;
};

Q_DECLARE_METATYPE(Session*)

template<typename T>
void Session::registerType(const QString& name, const QString& category, const QString& id, const QIcon& icon )
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

    auto mi = new Session::MetaInfo {
        cat->m_lTypes.size(), T::staticMetaObject, name, icon, {}
    };

    cat->m_lTypes << mi;

    m_hIdToType[id] = mi;
}
