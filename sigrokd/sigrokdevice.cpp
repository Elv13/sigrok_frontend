#include "sigrokdevice.h"
#include <libsigrokcxx/libsigrokcxx.hpp>

class SigrokDevicePrivate
{
public:
    bool m_IsActive {false};
    QString m_Name;
    std::shared_ptr<sigrok::HardwareDevice> m_Dev;
};

SigrokDevice::SigrokDevice(std::shared_ptr<sigrok::HardwareDevice> d, const QString& name, QObject* parent) : DeviceSource(parent),
    d_ptr(new SigrokDevicePrivate)
{
    d_ptr->m_Name = name;
    d_ptr->m_Dev = d;
}

SigrokDevice::~SigrokDevice()
{
    delete d_ptr;
}

QString SigrokDevice::name() const
{
    return d_ptr->m_Name;
}

bool SigrokDevice::isActive() const
{
    return d_ptr->m_IsActive;
}
void SigrokDevice::setIsActive(bool a)
{
    Q_UNUSED(a)
}

void SigrokDevice::start()
{
    d_ptr->m_IsActive = true;
    Q_EMIT isActiveChanged(true);
}

void SigrokDevice::stop()
{
    d_ptr->m_IsActive = false;
    Q_EMIT isActiveChanged(false);
}

void SigrokDevice::setName(QString n)
{
    d_ptr->m_Name = n;
    Q_EMIT nameChanged(n);
}

QByteArray SigrokDevice::type() const
{
    return "sigrok";
}

void SigrokDevice::setType(QByteArray type)
{
    Q_UNUSED(type);
}

std::shared_ptr<sigrok::HardwareDevice> SigrokDevice::native() const
{
    return d_ptr->m_Dev;
}
