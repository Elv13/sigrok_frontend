#pragma once

#include <QtWidgets/QWidget>

class ColumnProxy;
class DeduplicateNode;
class QAbstractItemModel;
class QComboBox;

class Deduplicate : public QWidget
{
    Q_OBJECT

public:
    explicit Deduplicate(const DeduplicateNode* n, QWidget* parent = nullptr);

    virtual ~Deduplicate();

    void setModel(QAbstractItemModel* m);

    QString selectedColumnName() const;

    void setPreferredColumn(const QString& name);

public Q_SLOTS:
    void slotEnableExtraColumn(bool);
    void slotChangeTreshold(double value);
    void slotIndexChanged(int idx);
    void slotColumnInserted();
    void slotIndexChanged();

Q_SIGNALS:
    void enableExtraColumn(bool);
    void thresholdChanged(const QVariant& v);
    void currentColumnChanged(int);

private:
    ColumnProxy* m_CProxy;
    QComboBox* m_pColumnCBB;
    QString m_PreferredColumn;
    bool m_HasManualSelection {false};
};
