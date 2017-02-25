#pragma once

#include <memory>

#include <QtCore/QIdentityProxyModel>

class VectorizingProxyPrivate;

/**
 * Take a source model and keep a vector copy of some columns.
 *
 * This is useful for interacting with external systems that takes std::array,
 * std::vector or QVector as inputs.
 *
 * In the future, it could be extended to support some SIMD topologies for
 * subclasses to take advantage of.
 *
 * Another useful feature would be to implement a virtual method for direct
 * data access using the internal pointer. If struct is known and "trusted",
 * then it would have much better performance.
 */
class VectorizingProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit VectorizingProxy(QObject* parent = Q_NULLPTR);
    virtual ~VectorizingProxy();

    void setColumns(const QList<int>& columns);

    const QVector<double> cowVectorData(int column) const;

    virtual void setSourceModel(QAbstractItemModel* model) override;

    std::shared_ptr< QVector<double> > sharedVectorData(int column) const;

    QVector<double> rowsVector() const;

private:
    VectorizingProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(VectorizingProxy)
};
