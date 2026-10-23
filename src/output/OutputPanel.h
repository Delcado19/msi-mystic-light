#pragma once
#include "OutputScheduler.h"
#include "editor/LightingWindow.h"
#include <QWidget>
#include <vector>
class QLabel;
class QPushButton;
class QTreeWidget;
struct OutputConnection {
    OutputFrame initial;
    OutputScheduler::SessionFactory factory;
    std::function<bool()> present;
};
// Authorization is an injected host boundary; tests supply fake connections and never open hardware.
class OutputPanel : public QWidget {
    Q_OBJECT
public:
    using Authorize = std::function<QVector<OutputConnection>(QWidget*, const QVector<OutputFrame>&)>;
    OutputPanel(LightingWindow& window, Authorize authorize, QWidget* parent = nullptr);
    void stopOutput();
signals:
    void outputStateChanged(const QString& state);
private:
    void start();
    void stop(const QString& reason);
    void dispatch(const QImage& frame);
    void refresh();
    bool active() const;
    struct Connection {
        OutputFrame approved;
        std::unique_ptr<OutputScheduler> scheduler;
        quint64 generation = 0;
    };
    LightingWindow& window_;
    Authorize authorize_;
    std::vector<Connection> connections_;
    bool authorizing_ = false, stopping_ = false;
    QPushButton *start_, *stop_;
    QLabel* status_;
    QTreeWidget* endpoints_;
    QString message_ = "Stopped. Start reviews reports before opening hardware.";
};
