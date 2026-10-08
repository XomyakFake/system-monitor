#pragma once

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QPainter>
#include <QValueAxis>

#include <deque>

class ChartWidget : public QChartView {
public:
    explicit ChartWidget(const QString& title, QWidget* parent = nullptr)
        : QChartView(parent) {
        series_ = new QLineSeries(this);
        chart_ = new QChart();
        chart_->addSeries(series_);
        chart_->setTitle(title);
        chart_->legend()->hide();

        auto* yAxis = new QValueAxis(this);
        yAxis->setRange(0, 100);
        yAxis->setLabelFormat("%.0f");
        chart_->addAxis(yAxis, Qt::AlignLeft);
        series_->attachAxis(yAxis);

        setChart(chart_);
        setRenderHint(QPainter::Antialiasing);
    }

    void updateData(const std::deque<double>& history) {
        series_->clear();
        for (std::size_t i = 0; i < history.size(); ++i) {
            series_->append(static_cast<double>(i), history[i]);
        }
    }

private:
    QLineSeries* series_ = nullptr;
    QChart* chart_ = nullptr;
};