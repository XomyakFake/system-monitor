#include "main_window.hpp"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupLayout();

    monitor_.start();
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::updateUI);
    timer_->start(500);
}

MainWindow::~MainWindow() {
    monitor_.stop();
}

void MainWindow::setupLayout() {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    status_label_ = new QLabel(tr("Collecting system data..."), central);
    cpu_bar_ = new QProgressBar(central);
    cpu_bar_->setRange(0, 100);
    cpu_bar_->setFormat(tr("%p%"));

    ram_bar_ = new QProgressBar(central);
    ram_bar_->setRange(0, 100);
    ram_bar_->setFormat(tr("%p%"));

    network_label_ = new QLabel(tr("Network: waiting for sample"), central);

    process_table_ = new QTableWidget(central);
    process_table_->setColumnCount(4);
    process_table_->setHorizontalHeaderLabels({tr("PID"), tr("Name"), tr("CPU %"), tr("RSS (MiB)")});
    process_table_->horizontalHeader()->setStretchLastSection(true);
    process_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    process_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    process_table_->setAlternatingRowColors(true);

    layout->addWidget(status_label_);
    layout->addWidget(new QLabel(tr("CPU usage"), central));
    layout->addWidget(cpu_bar_);
    layout->addWidget(new QLabel(tr("Memory usage"), central));
    layout->addWidget(ram_bar_);
    layout->addWidget(network_label_);
    layout->addWidget(process_table_);

    setCentralWidget(central);
    setWindowTitle(tr("System Monitor"));
    resize(680, 520);
}

void MainWindow::updateUI() {
    const SystemSnapshot snapshot = monitor_.snapshot();
    if (!snapshot.valid) {
        return;
    }

    status_label_->setText(tr("System data updated"));
    cpu_bar_->setValue(static_cast<int>(std::clamp(snapshot.cpu_usage_percent, 0.0, 100.0)));

    const double memory_percent = snapshot.memory.total == 0
        ? 0.0
        : 100.0 * static_cast<double>(snapshot.memory.used) / static_cast<double>(snapshot.memory.total);
    ram_bar_->setValue(static_cast<int>(std::clamp(memory_percent, 0.0, 100.0)));

    network_label_->setText(tr("Network: received %1 MiB/s, transmitted %2 MiB/s")
        .arg(snapshot.network.speedMbRe, 0, 'f', 2)
        .arg(snapshot.network.speedMbTr, 0, 'f', 2));

    process_table_->setRowCount(static_cast<int>(snapshot.processes.size()));
    for (std::size_t row = 0; row < snapshot.processes.size(); ++row) {
        const ProcessInfo& process = snapshot.processes[row];
        process_table_->setItem(static_cast<int>(row), 0, new QTableWidgetItem(QString::number(process.pid)));
        process_table_->setItem(static_cast<int>(row), 1, new QTableWidgetItem(QString::fromStdString(process.name)));
        process_table_->setItem(static_cast<int>(row), 2, new QTableWidgetItem(QString::number(process.cpu_percent, 'f', 1)));
        process_table_->setItem(static_cast<int>(row), 3, new QTableWidgetItem(QString::number(process.vm_rss_kb / 1024.0, 'f', 1)));
    }
}