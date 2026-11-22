// src/ui/RoomSearchPage.cpp
#include "ui/RoomSearchPage.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace aurel {

RoomSearchPage::RoomSearchPage(RoomService& roomService, QWidget* parent)
    : QWidget(parent), roomService_(roomService) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("<h2>Tìm kiếm phòng (FR-02 & FR-07)</h2>"));

    auto* filterLayout = new QHBoxLayout();
    filterByDateCheck_ = new QCheckBox("Lọc theo ngày:");
    filterByDateCheck_->setChecked(true);

    checkInEdit_ = new QDateEdit(QDate(2026, 12, 10));
    checkInEdit_->setCalendarPopup(true);
    checkInEdit_->setDisplayFormat("yyyy-MM-dd");

    checkOutEdit_ = new QDateEdit(QDate(2026, 12, 15));
    checkOutEdit_->setCalendarPopup(true);
    checkOutEdit_->setDisplayFormat("yyyy-MM-dd");

    guestsSpin_ = new QSpinBox();
    guestsSpin_->setRange(1, 10);
    guestsSpin_->setValue(2);

    roomTypeCombo_ = new QComboBox();
    auto* searchBtn = new QPushButton("Tìm kiếm");

    filterLayout->addWidget(filterByDateCheck_);
    filterLayout->addWidget(new QLabel("Nhận:"));
    filterLayout->addWidget(checkInEdit_);
    filterLayout->addWidget(new QLabel("Trả:"));
    filterLayout->addWidget(checkOutEdit_);
    filterLayout->addWidget(new QLabel("Khách:"));
    filterLayout->addWidget(guestsSpin_);
    filterLayout->addWidget(new QLabel("Loại:"));
    filterLayout->addWidget(roomTypeCombo_);
    filterLayout->addWidget(searchBtn);
    layout->addLayout(filterLayout);

    resultsTable_ = new QTableWidget(0, 6, this);
    resultsTable_->setHorizontalHeaderLabels({"ID", "Số phòng", "Loại phòng", "Sức chứa", "Giá/đêm ($)", "Tiện nghi"});
    resultsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(resultsTable_);

    auto* detailBtn = new QPushButton("Xem chi tiết & Kiểm tra đặt phòng");
    layout->addWidget(detailBtn);

    connect(filterByDateCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        checkInEdit_->setEnabled(checked);
        checkOutEdit_->setEnabled(checked);
    });
    connect(searchBtn, &QPushButton::clicked, this, &RoomSearchPage::onSearchClicked);

    connect(detailBtn, &QPushButton::clicked, this, [this]() {
        int row = resultsTable_->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "Thông báo", "Vui lòng chọn phòng cần xem.");
            return;
        }
        RoomId roomId = resultsTable_->item(row, 0)->text().toInt();
        std::optional<StayPeriod> period = std::nullopt;
        if (filterByDateCheck_->isChecked()) {
            period = StayPeriod{checkInEdit_->date(), checkOutEdit_->date()};
        }
        emit roomSelected(roomId, period);
    });

    reloadRoomTypes();
    onSearchClicked();
}

void RoomSearchPage::reloadRoomTypes() {
    roomTypeCombo_->clear();
    roomTypeCombo_->addItem("Tất cả loại phòng", 0);
    auto typesRes = roomService_.getAllRoomTypes();
    if (typesRes.ok) {
        for (const auto& t : typesRes.value) {
            roomTypeCombo_->addItem(t.name(), t.id());
        }
    }
}

void RoomSearchPage::applyExternalSearch(const StayPeriod& period, int guests) {
    filterByDateCheck_->setChecked(true);
    checkInEdit_->setDate(period.checkInDate);
    checkOutEdit_->setDate(period.checkOutDate);
    guestsSpin_->setValue(guests);
    onSearchClicked();
}

void RoomSearchPage::onSearchClicked() {
    RoomSearchCriteria criteria;
    criteria.minGuests = guestsSpin_->value();
    int selectedTypeId = roomTypeCombo_->currentData().toInt();
    if (selectedTypeId > 0) {
        criteria.roomTypeId = selectedTypeId;
    }
    if (filterByDateCheck_->isChecked()) {
        criteria.stayPeriod = StayPeriod{checkInEdit_->date(), checkOutEdit_->date()};
    }

    auto res = roomService_.searchRooms(criteria);
    if (!res.ok) {
        QMessageBox::warning(this, "Lỗi tìm kiếm", res.message);
        return;
    }

    resultsTable_->setRowCount(0);
    for (const auto& room : res.value) {
        int row = resultsTable_->rowCount();
        resultsTable_->insertRow(row);
        resultsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(room.id())));
        resultsTable_->setItem(row, 1, new QTableWidgetItem(room.roomNumber()));
        resultsTable_->setItem(row, 2, new QTableWidgetItem(room.roomTypeName()));
        resultsTable_->setItem(row, 3, new QTableWidgetItem(QString::number(room.capacity())));
        resultsTable_->setItem(row, 4, new QTableWidgetItem(QString::number(room.pricePerNightCents() / 100.0, 'f', 2)));
        resultsTable_->setItem(row, 5, new QTableWidgetItem(room.facilities().join(", ")));
    }
}

} // namespace aurel