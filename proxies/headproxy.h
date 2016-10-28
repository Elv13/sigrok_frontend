#pragma once

#include <QtCore/QIdentityProxyModel>

class HeadProxyPrivate;

class HeadProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged);

    explicit HeadProxy(QObject* parent = nullptr);
    virtual ~HeadProxy();

    virtual int rowCount(const QModelIndex& parent = {}) const override;

    bool isLimited() const;

    int maximum() const;

public Q_SLOTS:
    void setMaximum(int max);
    void setLimited(bool limit);

Q_SIGNALS:
    void maximumChanged(int);

private:
    HeadProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(HeadProxy)
};
