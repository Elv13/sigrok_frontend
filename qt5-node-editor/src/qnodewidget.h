#pragma once

#include "qnodeview.h"

#include <QtCore/QVariant>

class QNodeWidgetPrivate;

class QAbstractItemModel;
class GraphicsNode;

class Q_DECL_EXPORT QNodeWidget : public QNodeView
{
    Q_OBJECT
public:
    enum class ObjectFlags {
        NONE                  = 0,
        USER_PROPERTIES       = 1 << 0,
        DESIGNABLE_PROPERTIES = 1 << 1,
        CLASS_PROPERTIES      = 1 << 2,
        INHERITED_PROPERTIES  = 1 << 3,
        SIGNALS               = 1 << 4,
        SLOTS                 = 1 << 5,
        CLASS_METHODS         = 1 << 6,
        INHERITED_METHODS     = 1 << 7
    };

    explicit QNodeWidget(QWidget* parent = Q_NULLPTR);
    virtual ~QNodeWidget();

    GraphicsNode* addObject(
        QObject*       o,
        const QString& title = QString(),
        ObjectFlags    f     = ObjectFlags::NONE,
        const QVariant& uid  = {}
    );

    GraphicsNode* addModel(
        QAbstractItemModel* m,
        const QString&      title = QString(),
        const QVariant&     uid   = {}
    );

    GraphicsNode* currentNode() const;
    QList<GraphicsNode*> currentNodes() const;

    void setCurrentNode(GraphicsNode* n);

    bool removeNode(GraphicsNode* n);

Q_SIGNALS:
    // Removing
    void objectRemoved(QObject* o);
    void modelRemoved(QAbstractItemModel* m);

    // Renaming
    void objectRenamed(QObject* o, const QString& nameName, const QString& oldName);
    void modelRenamed(QAbstractItemModel* m, const QString& nameName, const QString& oldName);
    void nodeRenamed(GraphicsNode* n, const QString& nameName, const QString& oldName);
    void nodeRenamed(const QString& uid, const QString& nameName, const QString& oldName);

    // Selection
    void currentNodeChanged(GraphicsNode* current, GraphicsNode* previous);

private:
    QNodeWidgetPrivate* d_ptr;
    Q_DECLARE_PRIVATE(QNodeWidget)
};
