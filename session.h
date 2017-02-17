#pragma once

#include "common/abstractsession.h"

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/graphicsnodescene.hpp"
#include "qt5-node-editor/src/graphicsbezieredge.hpp"
#include "qt5-node-editor/src/graphicsnodesocket.hpp"

#include <QtCore/QDebug>
#include <QtCore/QAbstractItemModel>

#include <QtGui/QIcon>

#include <functional>

#include "common/abstractnode.h"

class QNodeWidget;
class QAbstractItemModel;
class GraphicsNodeScene;
class QIODevice;

class InterfaceSerializer;
class SessionPrivate;

class Session : public AbstractSession
{
    friend class NodeMimeData;
    Q_OBJECT
public:
    typedef QPair<GraphicsNode*, AbstractNode*> NodePair;

    explicit Session(QObject* parent, QNodeWidget* w);
    virtual ~Session();

    template<typename T>
    void registerType(const QString& name, const QString& id, const QString& cat, const QIcon& icon = {});

    QUrl fileName() const;
    void setFileName(const QUrl& url);

    void registerInterfaceSerializer(InterfaceSerializer* ser);

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& idx) const override;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const override;
    virtual QStringList mimeTypes() const override;

    QByteArray serializeSelection() const;

    void serialize(QIODevice *dev) const;
    void load(QIODevice *dev);
    void load(const QByteArray& data);

    AbstractNode* fromGraphicsNode(GraphicsNode* n) const;
    GraphicsNode* fromAbstractNode(AbstractNode* n) const;

    bool addNodesFromData(const QByteArray& data, const QPoint& point = {});

    void forEachSelected(const std::function<void(GraphicsNode* gn, AbstractNode* an)>& f) const;

    virtual PageManager* pages() const override;

    QNodeWidget* nodeWidget() const;

public Q_SLOTS:
    NodePair addToScene(const QModelIndex& idx);

private Q_SLOTS:
    void remove(const QObject* o, const QString& id);
    void renameN(const QString& n, const QString& name);
    void notifyForward(
        const QString& message,
        bool system,
        AbstractNode::NotifyPriority p
    );

Q_SIGNALS:
    void notify(
        const QString& message,
        AbstractNode* node,
        bool system = false,
        AbstractNode::NotifyPriority p = AbstractNode::NotifyPriority::NORMAL
    );
    void renamed(const QString& name);

private:
    NodePair addToSceneFromMetaObject(const QMetaObject& o, const QString& uid = {});

    QNodeWidget* m_pNodeW {nullptr};
    struct MetaInfo {
        int m_Index;
        const QMetaObject m_spMetaObj;
        QString m_Name;
        QIcon m_Icon;
        QVector< NodePair > m_lInstances;
    };

    struct Category {
        QString m_Name;
        QVector<MetaInfo*> m_lTypes;
    };

    QVector<Category*> m_slCategory;
    QHash<QString, MetaInfo*> m_hIdToType;
    QHash<QString, NodePair>  m_hIdToNode;
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
        beginInsertRows({}, m_slCategory.size(), m_slCategory.size());
        m_slCategory << cat;
        endInsertRows();
    }

    auto mi = new Session::MetaInfo {
        cat->m_lTypes.size(), T::staticMetaObject, name, icon, {}
    };

//     const auto idx = index(m_slCategory.size()-1, 0);

//     beginInsertRows(idx, cat->m_lTypes.size(), cat->m_lTypes.size());
    cat->m_lTypes << mi;
//     endInsertRows();

    m_hIdToType[id] = mi;
}
