// src/ui/RoomDetailsPage.h
#pragma once

#include "common/Types.h"
#include "services/RoomService.h"
#include <QDateEdit>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

namespace aurel {

class RoomDetailsPage : public QWidget {
    Q_OBJECT
public:
    explicit RoomDetailsPage(RoomService& roomService, QWidget* parent = nullptr);
    void loadRoom(RoomId roomId, const std::optional<StayPeriod>& stayPeriod);

signals:
    void proceedToBookingRequested(RoomId roomId, const StayPeriod& stayPeriod);

private slots:
    void onCheckAvailabilityClicked();

private:
    RoomService& roomService_;
    RoomId currentRoomId_{0};

    QLabel* titleLabel_{nullptr};
    QLabel* metaLabel_{nullptr};
    QLabel* availabilityLabel_{nullptr};
    QTextEdit* descEdit_{nullptr};
    QListWidget* facilitiesList_{nullptr};
    QListWidget* imagesList_{nullptr};
    QListWidget* reviewsList_{nullptr};
    QDateEdit* checkInEdit_{nullptr};
    QDateEdit* checkOutEdit_{nullptr};
    QPushButton* bookNowBtn_{nullptr};
};

} // namespace aurel
