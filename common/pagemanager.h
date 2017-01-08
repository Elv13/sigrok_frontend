#pragma once

#include <QtCore/QAbstractListModel>

class QWidget;

class PageManagerPrivate;
class AbstractNode;

class Q_DECL_EXPORT PageManager : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        REMOTE_MODEL_NAME = Qt::UserRole,
        REMOTE_OBJECT_NAME,
        REMOTE_WIDGET_TYPE,
        REMOTE_OBJECT_UID,
    };

    static PageManager* instance();

    void addPage(AbstractNode* n, QWidget* w, const QString& title = QString(), const QString& uid = QString());
    void removePage(AbstractNode* n);
//     void removePage(const QString& uid);

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;

Q_SIGNALS:
    void pageAdded(QWidget* page, const QString& title, const QString& uid);

private:
    explicit PageManager();
    virtual ~PageManager();

    PageManagerPrivate* d_ptr;
    Q_DECLARE_PRIVATE(PageManager)

};
