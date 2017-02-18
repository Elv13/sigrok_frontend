#pragma once

#include "liveacquisitionnode.h"

#include <memory>

#include <QtCore/QObject>

#include <QtCore/QAbstractItemModel>

class AcquisitionModel;

class ManualAcquisitionNodePrivate;

namespace sigrok {
    class HardwareDevice;
}

class Q_DECL_EXPORT ManualAcquisitionNode : public AcquisitionNode
{
    Q_OBJECT

public:
    Q_PROPERTY(bool acquire WRITE acquireSample USER true)
    Q_PROPERTY(QString acquireAs WRITE acquireSampleAs USER true)

    Q_INVOKABLE explicit ManualAcquisitionNode(AbstractSession* sess);

    bool dummy() const;

    virtual ~ManualAcquisitionNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QWidget* widget() const override;

    virtual void setDevice(SigrokDevice* dev) override;

public Q_SLOTS:
    void acquireSample(bool=false);
    void acquireSampleAs(const QString& name);

private:
    ManualAcquisitionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ManualAcquisitionNode)

};

Q_DECLARE_METATYPE(ManualAcquisitionNode*)
