// src/ui/RoomSearchPage.h
#pragma once

#include "common/Types.h"
#include "services/RoomService.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

namespace aurel {

class RoomSearchPage : public QWidget {
    Q_OBJECT
public:
    explicit RoomSearchPage(RoomService& roomService, QWidget* parent = nullptr);
    void applyExternalSearch(const StayPeriod& period, int guests);
    void reloadRoomTypes();

signals:
    void roomSelected(RoomId roomId, const std::optional<StayPeriod>& period);

private slots:
    void onSearchClicked();

private:
    RoomService& roomService_;
    QCheckBox* filterByDateCheck_{nullptr};
    QDateEdit* checkInEdit_{nullptr};
    QDateEdit* checkOutEdit_{nullptr};
    QSpinBox* guestsSpin_{nullptr};
    QComboBox* roomTypeCombo_{nullptr};
    QTableWidget* resultsTable_{nullptr};
};

} // namespace aurel
