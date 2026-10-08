#pragma once

#include "system_monitor.hpp"
#include "history.hpp"
#include "chart_widget.hpp"

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QProgressBar;
class QTableWidget;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void updateUI();
    void showProcessMenu(const QPoint& pos);

private:
    void setupLayout();
    void sendSignalToProcess(int pid, int signal);

    History<double> cpu_history{60};
    History<double> ram_history{60};

    ChartWidget* cpu_chart_ = nullptr;
    ChartWidget* ram_chart_ = nullptr;

    QLineEdit* search = nullptr;

    SystemMonitor monitor_;
    QTimer* timer_ = nullptr;
    QProgressBar* cpu_bar_ = nullptr;
    QProgressBar* ram_bar_ = nullptr;
    QLabel* network_label_ = nullptr;
    QLabel* status_label_ = nullptr;
    QTableWidget* process_table_ = nullptr;
};