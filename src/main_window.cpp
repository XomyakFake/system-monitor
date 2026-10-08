#include "main_window.hpp"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QScrollBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <csignal>
#include <cerrno>
#include <sys/types.h>
#include <QMenu>
#include <QMessageBox>

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
    process_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    process_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    process_table_->setAlternatingRowColors(true);
    process_table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    process_table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    process_table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    process_table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    process_table_->setSortingEnabled(true);

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

    cpu_chart_ = new ChartWidget("CPU History", this);
    ram_chart_ = new ChartWidget("RAM History", this);
    layout->addWidget(cpu_chart_);
    layout->addWidget(ram_chart_);

    search = new QLineEdit(this);
    search->setPlaceholderText("Search process...");
    layout->addWidget(search);

    process_table_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(process_table_, &QTableWidget::customContextMenuRequested, this, &MainWindow::showProcessMenu);

}


void MainWindow::showProcessMenu(const QPoint& pos) {
    QTableWidgetItem* clicked = process_table_->itemAt(pos);
    if (clicked == nullptr) {
        return;
    }

    QTableWidgetItem* pidItem = process_table_->item(clicked->row(), 0);
    if (pidItem == nullptr) {
        return;
    }
    const int pid = pidItem->text().toInt();

    QMenu menu(this);
    QAction* terminateAction = menu.addAction(tr("Terminate"));
    QAction* killAction = menu.addAction(tr("Kill (force)"));

    QAction* chosen = menu.exec(process_table_->viewport()->mapToGlobal(pos));

    if (chosen == terminateAction) {
        sendSignalToProcess(pid, SIGTERM);
    } else if (chosen == killAction) {
        sendSignalToProcess(pid, SIGKILL);
    }
}

void MainWindow::sendSignalToProcess(int pid, int signal) {
    if (pid <= 1) {
        return;
    }

    if (::kill(static_cast<pid_t>(pid), signal) == -1) {
        if (errno == EPERM) {
            QMessageBox::warning(this, tr("Error"), tr("Not enough permissions"));
        } else if (errno == ESRCH) {
            QMessageBox::warning(this, tr("Error"), tr("Process not found"));
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to send signal"));
        }
    }
}

void MainWindow::updateUI() {
    const SystemSnapshot snapshot = monitor_.snapshot();
    if (!snapshot.valid) {
        return;
    }

    cpu_history.add(snapshot.cpu_usage_percent);


    status_label_->setText(tr("System data updated"));
    cpu_bar_->setValue(static_cast<int>(std::clamp(snapshot.cpu_usage_percent, 0.0, 100.0)));

    const double ram_percent = snapshot.memory.total == 0 
        ? 0.0
        : 100.0 * static_cast<double>(snapshot.memory.used) / static_cast<double>(snapshot.memory.total);

    ram_history.add(ram_percent);
    ram_bar_->setValue(static_cast<int>(std::clamp(ram_percent, 0.0, 100.0)));

    network_label_->setText(tr("Network: received %1 MiB/s, transmitted %2 MiB/s")
        .arg(snapshot.network.speedMbRe, 0, 'f', 2)
        .arg(snapshot.network.speedMbTr, 0, 'f', 2));



    const QString filter = search->text().trimmed().toLower();
    std::vector<ProcessInfo> filtered;
    filtered.reserve(snapshot.processes.size());
    for (const auto& process : snapshot.processes) {
        if (filter.isEmpty() || QString::fromStdString(process.name).toLower().contains(filter)) {
            filtered.push_back(process);
        }
    }

    const int current_row = process_table_->currentRow();
    const int scroll_value = process_table_->verticalScrollBar()->value();

    process_table_->setSortingEnabled(false);
    process_table_->setRowCount(static_cast<int>(filtered.size()));
    for (std::size_t row = 0; row < filtered.size(); ++row) {
        const ProcessInfo& process = filtered[row];
        process_table_->setItem(static_cast<int>(row), 0, new QTableWidgetItem(QString::number(process.pid)));
        process_table_->setItem(static_cast<int>(row), 1, new QTableWidgetItem(QString::fromStdString(process.name)));
        process_table_->setItem(static_cast<int>(row), 2, new QTableWidgetItem(QString::number(process.cpu_percent, 'f', 1)));
        process_table_->setItem(static_cast<int>(row), 3, new QTableWidgetItem(QString::number(process.vm_rss_kb / 1024.0, 'f', 1)));
    }
    process_table_->setSortingEnabled(true);

    if (current_row >= 0 && current_row < process_table_->rowCount()) {
        process_table_->selectRow(current_row);
    }
    process_table_->verticalScrollBar()->setValue(scroll_value);

    cpu_chart_->updateData(cpu_history.getData());
    ram_chart_->updateData(ram_history.getData());
}