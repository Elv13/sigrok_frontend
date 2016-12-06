#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QModelIndex>
#include <functional>

class QTreeView;
class QComboBox;
class RangeProxy;
class FilterTopLevelProxy;
class QVBoxLayout;
class QDialogButtonBox;

namespace color_widgets {
    class ColorWheel;
};

class Range : public QWidget
{
    Q_OBJECT

public:
    explicit Range(QWidget* parent = nullptr);
    virtual ~Range();

    RangeProxy* rangeProxy() const;
    void setRangeProxy(RangeProxy* p);

    void setColumnWidgetFactory(
        int col,
        std::function<QWidget*(const QPersistentModelIndex& idx)> w
    );

private:
    RangeProxy* m_pProxy {nullptr};
    FilterTopLevelProxy* m_pFiltered {nullptr};
    QVector< std::function<QWidget*(const QPersistentModelIndex&)> > m_lWidgetFactories; //FIXME deadcode
    QVector< std::function<QWidget*(const QPersistentModelIndex&)> > m_lWidgetFactoriesChild;
    QTreeView* m_pTree;
    QComboBox* m_pColumn;
    bool m_Mutex {false};
    QWidget* m_pMainWidget;
    QVBoxLayout* m_pLayout;
    color_widgets::ColorWheel* m_pColorWheel {nullptr};
    QDialogButtonBox* m_pButtonBox {nullptr};
    QPersistentModelIndex m_CurrentIdx;

    // Helpers
    void applyWidget(const QModelIndex& root, QVector< std::function<QWidget*(const QPersistentModelIndex&)> >& f);

private Q_SLOTS:
    void slotAjustColumns();
    void slotAddClicked();
    void slotAllColumns(bool);
    void okClicked();
    void cancelClicked();
};
