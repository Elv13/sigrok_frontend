#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class XmlNodePrivate;

class Q_DECL_EXPORT XmlNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("xml_node", "Import XML", "", /*Tags:*/
        "hardware", "file"
    )
public:
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY modelChanged USER true);

    Q_INVOKABLE explicit XmlNode(AbstractSession* sess);
    virtual ~XmlNode();

    virtual QWidget* widget() const override;

    QAbstractItemModel* model() const;

    virtual void write(QJsonObject &parent) const override;

Q_SIGNALS:
    void modelChanged(QAbstractItemModel*);
    void activated(bool);
    void tick(bool);

private:
    XmlNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(XmlNode)

};
Q_DECLARE_METATYPE(XmlNode*)
