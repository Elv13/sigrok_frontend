#include "headproxy.h"

class HeadProxyPrivate
{
public:
    int m_Maximum {-1};
};

HeadProxy::HeadProxy(QObject* parent) : QIdentityProxyModel(parent),
d_ptr(new HeadProxyPrivate)
{
    
}

HeadProxy::~HeadProxy()
{
    delete d_ptr;
}

int HeadProxy::rowCount(const QModelIndex& parent) const
{
    const int rc = QIdentityProxyModel::rowCount(parent);

    if (parent.isValid() || !isLimited())
        return rc;

    return rc > maximum()-1 ? maximum() : rc;
}

bool HeadProxy::isLimited() const
{
    return d_ptr->m_Maximum>-1;
}


int HeadProxy::maximum() const
{
    return d_ptr->m_Maximum;
}

void HeadProxy::setMaximum(int max)
{
    if (max < 0)
        return;

    d_ptr->m_Maximum = max;

    if (sourceModel() && sourceModel()->rowCount() > max)
        Q_EMIT layoutChanged();
}

void HeadProxy::setLimited(bool limit)
{
    const bool differ = isLimited() != limit;
    d_ptr->m_Maximum = limit ? d_ptr->m_Maximum : -1;

    if (differ)
        Q_EMIT layoutChanged();
}
