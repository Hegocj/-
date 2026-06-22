#include "TimelineWidget.h"

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QtGlobal>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(180);
}

void TimelineWidget::setRecords(const std::vector<FollowRecord>& records)
{
    m_records = records;
    updateGeometry();
    update();
}

QSize TimelineWidget::minimumSizeHint() const
{
    return QSize(320, qMax(180, 72 * static_cast<int>(m_records.size()) + 32));
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().base());

    const int left = 28;
    const int top = 28;
    const int rowHeight = 72;

    if (m_records.empty()) {
        painter.setPen(palette().mid().color());
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("No follow records"));
        return;
    }

    painter.setPen(QPen(palette().mid().color(), 2));
    painter.drawLine(left, top, left, top + rowHeight * (static_cast<int>(m_records.size()) - 1));

    for (int i = 0; i < static_cast<int>(m_records.size()); ++i) {
        const auto& record = m_records.at(static_cast<size_t>(i));
        const int y = top + i * rowHeight;

        painter.setBrush(QColor(45, 125, 210));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(left, y), 6, 6);

        painter.setPen(palette().text().color());
        const QString title = record.getDate().toString(QStringLiteral("yyyy-MM-dd HH:mm")) +
                              QStringLiteral("  ") + record.getOperatorName();
        painter.drawText(left + 18, y - 8, width() - left - 30, 20, Qt::AlignLeft | Qt::AlignVCenter, title);

        painter.setPen(palette().mid().color());
        painter.drawText(left + 18, y + 14, width() - left - 30, 42,
                         Qt::AlignLeft | Qt::TextWordWrap, record.getContent());
    }
}
