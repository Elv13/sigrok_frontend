#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TableNodePrivate;

class Q_DECL_EXPORT TableNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("table_node", "Table", "", /*Tags:*/
        "matrix",
        "spreadsheet",
        "widget"
    )

public:
    Q_INVOKABLE explicit TableNode(AbstractSession* sess);
    virtual ~TableNode();

    virtual void write(QJsonObject &parent) const override;

    virtual QWidget* widget() const override;

    virtual QString remoteModelName() const override;
    virtual QString remoteWidgetType() const override;

private:
    TableNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TableNode)

};
Q_DECLARE_METATYPE(TableNode*)
