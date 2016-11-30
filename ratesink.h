#pragma once

#include <QtCore/QObject>

class AquisitionModel;

class RateSinkPrivate;

class RateSink : public QObject
{
    Q_OBJECT

public:
    explicit RateSink(const AquisitionModel* source);
    virtual ~RateSink();

    int sampleSize() const;
    void setSampleSize(int size);

    int threshold() const;
    void setThreshold(int value);

    float rate() const;

private:
    RateSinkPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RateSink)

Q_SIGNALS:
    void rateChanged(float rate);
    void rateChangedAsString(QString);
};
