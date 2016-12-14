#pragma once

#include "aquisitionnode.h"

#include <memory>

#include <QtCore/QObject>

#include <QtCore/QAbstractItemModel>

class AquisitionModel;

class ManualAquisitionNodePrivate;

namespace sigrok {
    class HardwareDevice;
}

class Q_DECL_EXPORT ManualAquisitionNode : public AquisitionNode
{
    Q_OBJECT

public:
    Q_PROPERTY(bool aquire WRITE aquireSample USER true)
    Q_PROPERTY(QString aquireAs WRITE aquireSampleAs USER true)

    Q_INVOKABLE explicit ManualAquisitionNode(QObject* parent = nullptr);

    bool dummy() const;

    virtual ~ManualAquisitionNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QWidget* widget() const override;

    virtual void setDevice(SigrokDevice* dev) override;

public Q_SLOTS:
    void aquireSample(bool=false);
    void aquireSampleAs(const QString& name);

private:
    ManualAquisitionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ManualAquisitionNode)

};

Q_DECLARE_METATYPE(ManualAquisitionNode*)
