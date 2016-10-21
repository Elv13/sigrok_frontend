#include "ratesink.h"

#include <QtCore/QVector>
#include <QtCore/QDebug>

#include "aquisitionmodel.h"

class RateSinkPrivate : public QObject
{
public:
    int m_SampleSize {10};
    int m_Threshold  {1 };
    float m_Rate {0};
    RateSink* q_ptr;

    QVector<unsigned long long> m_lBuffer {0,0,0,0,0,0,0,0,0,0};

    void slotCurrentChanged();
};

void RateSinkPrivate::slotCurrentChanged()
{
    //FIXME use a circular buffer, not a popbuffer.
    m_lBuffer.pop_front();
    m_lBuffer << 0;

    int delaySum = 0;

    for (int i=1; i < m_SampleSize; i++)
        delaySum += m_lBuffer[i] - m_lBuffer[i-1];

    m_Rate = delaySum/(m_SampleSize-1);

    Q_EMIT q_ptr->rateChanged(m_Rate); //TODO implement m_Threshold
    Q_EMIT q_ptr->rateChangedAsString(QString("%1 sample/s").arg(m_Rate));
}

RateSink::RateSink(const AquisitionModel* source) :
    QObject((QObject*)source), d_ptr(new RateSinkPrivate())
{
    d_ptr->q_ptr = this;

    QObject::connect(
        source, &AquisitionModel::currentValuesChanged,
        d_ptr, &RateSinkPrivate::slotCurrentChanged
    );
}

RateSink::~RateSink()
{
    
}

int RateSink::sampleSize() const
{
    return d_ptr->m_SampleSize;
}

void RateSink::setSampleSize(int size)
{
    d_ptr->m_SampleSize = size;
}

int RateSink::threshold() const
{
    return d_ptr->m_Threshold;
}

void RateSink::setThreshold(int value)
{
    d_ptr->m_Threshold = value;
}

float RateSink::rate() const
{
    return d_ptr->m_Rate;
}
