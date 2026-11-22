// src/ui/HomepagePage.h
#pragma once

#include "common/Types.h"
#include "services/RoomService.h"
#include <QDateEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

namespace aurel {

class HomepagePage : public QWidget {
    Q_OBJECT
public:
    explicit HomepagePage(RoomService& roomService, QWidget* parent = nullptr);
    void refreshFeaturedRooms();

signals:
    void searchRequested(const StayPeriod& period, int guests);
    void roomSelected(RoomId roomId, const std::optional<StayPeriod>& period);

private:
    RoomService& roomService_;
    QDateEdit* checkInEdit_{nullptr};
    QDateEdit* checkOutEdit_{nullptr};
    QSpinBox* guestsSpin_{nullptr};
    QTableWidget* featuredTable_{nullptr};
};

} // namespace aurel
