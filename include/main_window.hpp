#pragma once

#include "system_monitor.hpp"

#include <QMainWindow>

class QLabel;
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

private:
    void setupLayout();

    SystemMonitor monitor_;
    QTimer* timer_ = nullptr;
    QProgressBar* cpu_bar_ = nullptr;
    QProgressBar* ram_bar_ = nullptr;
    QLabel* network_label_ = nullptr;
    QLabel* status_label_ = nullptr;
    QTableWidget* process_table_ = nullptr;
};