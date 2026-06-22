#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "FollowRecord.h"

#include <QSize>
#include <QWidget>
#include <vector>

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);

    void setRecords(const std::vector<FollowRecord>& records);
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<FollowRecord> m_records;
};

#endif // TIMELINEWIDGET_H
