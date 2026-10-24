#pragma once
#include "OutputScheduler.h"
#include "editor/LightingWindow.h"
#include <QWidget>
#include <vector>
#include <QSet>
class QTimer;
class QLabel;
class QPushButton;
class QTreeWidget;
struct OutputConnection {
    OutputFrame initial;
    OutputScheduler::SessionFactory factory;
    std::function<bool()> present;
};
// Connection preparation is injected; tests supply fake transports and never open hardware.
class OutputPanel : public QWidget {
    Q_OBJECT
public:
    using Authorize = std::function<QVector<OutputConnection>(QWidget*, const QVector<OutputFrame>&)>;
    OutputPanel(LightingWindow& window, Authorize authorize, QWidget* parent = nullptr);
    void stopOutput();
    void enableAutomaticOutput();
signals:
    void outputStateChanged(const QString& state);
private:
    void start(bool includeMissing = false);
    void stop(const QString& reason);
    void dispatch(const QImage& frame);
    void refresh();
    void updateControls();
    QStringList selectedChannels() const;
    OutputFrame enabledFrame(OutputFrame frame) const;
    QSet<QString> pausedChannels_;
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
    QTimer* automatic_;
    QVector<OutputFrame> configured_;
    QPushButton *start_, *stop_;
    QLabel* status_;
    QTreeWidget* endpoints_;
    QString message_ = "Waiting for configured hardware.";
};
