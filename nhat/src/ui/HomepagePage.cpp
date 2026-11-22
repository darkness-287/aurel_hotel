// src/ui/HomepagePage.cpp
#include "ui/HomepagePage.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace aurel {

HomepagePage::HomepagePage(RoomService& roomService, QWidget* parent)
    : QWidget(parent), roomService_(roomService) {
    auto* mainLayout = new QVBoxLayout(this);

    auto* bannerLabel = new QLabel("<h2>Aurel House — Luxury & Boutique Stays</h2><p>Khám phá không gian nghỉ dưỡng đẳng cấp và đặt phòng trực tuyến.</p>");
    mainLayout->addWidget(bannerLabel);

    // Thanh tìm kiếm nhanh trên Homepage
    auto* searchBarLayout = new QHBoxLayout();
    checkInEdit_ = new QDateEdit(QDate(2026, 12, 10));
    checkInEdit_->setCalendarPopup(true);
    checkInEdit_->setDisplayFormat("yyyy-MM-dd");

    checkOutEdit_ = new QDateEdit(QDate(2026, 12, 12));
    checkOutEdit_->setCalendarPopup(true);
    checkOutEdit_->setDisplayFormat("yyyy-MM-dd");

    guestsSpin_ = new QSpinBox();
    guestsSpin_->setRange(1, 10);
    guestsSpin_->setValue(2);

    auto* searchBtn = new QPushButton("Tìm phòng ngay");
    searchBarLayout->addWidget(new QLabel("Nhận phòng:"));
    searchBarLayout->addWidget(checkInEdit_);
    searchBarLayout->addWidget(new QLabel("Trả phòng:"));
    searchBarLayout->addWidget(checkOutEdit_);
    searchBarLayout->addWidget(new QLabel("Số khách:"));
    searchBarLayout->addWidget(guestsSpin_);
    searchBarLayout->addWidget(searchBtn);
    mainLayout->addLayout(searchBarLayout);

    // Danh sách phòng nổi bật (FR-01)
    mainLayout->addWidget(new QLabel("<h3>Phòng nổi bật (FR-01)</h3>"));
    featuredTable_ = new QTableWidget(0, 6, this);
    featuredTable_->setHorizontalHeaderLabels({"ID", "Số phòng", "Loại phòng", "Sức chứa", "Giá/đêm ($)", "Ảnh đại diện"});
    featuredTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    featuredTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    featuredTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(featuredTable_);

    auto* viewDetailBtn = new QPushButton("Xem chi tiết phòng đang chọn");
    mainLayout->addWidget(viewDetailBtn);

    connect(searchBtn, &QPushButton::clicked, this, [this]() {
        StayPeriod period{checkInEdit_->date(), checkOutEdit_->date()};
        if (!period.isValid()) {
            QMessageBox::warning(this, "Ngày không hợp lệ", "Ngày trả phòng phải sau ngày nhận phòng.");
            return;
        }
        emit searchRequested(period, guestsSpin_->value());
    });

    connect(viewDetailBtn, &QPushButton::clicked, this, [this]() {
        int row = featuredTable_->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "Thông báo", "Vui lòng chọn một phòng.");
            return;
        }
        RoomId roomId = featuredTable_->item(row, 0)->text().toInt();
        emit roomSelected(roomId, std::nullopt);
    });

    refreshFeaturedRooms();
}

void HomepagePage::refreshFeaturedRooms() {
    auto res = roomService_.getFeaturedRooms();
    if (!res.ok) {
        QMessageBox::critical(this, "Lỗi", res.message);
        return;
    }

    featuredTable_->setRowCount(0);
    for (const auto& room : res.value) {
        int row = featuredTable_->rowCount();
        featuredTable_->insertRow(row);
        featuredTable_->setItem(row, 0, new QTableWidgetItem(QString::number(room.id())));
        featuredTable_->setItem(row, 1, new QTableWidgetItem(room.roomNumber()));
        featuredTable_->setItem(row, 2, new QTableWidgetItem(room.roomTypeName()));
        featuredTable_->setItem(row, 3, new QTableWidgetItem(QString::number(room.capacity()) + " khách"));
        featuredTable_->setItem(row, 4, new QTableWidgetItem(QString::number(room.pricePerNightCents() / 100.0, 'f', 2)));
        featuredTable_->setItem(row, 5, new QTableWidgetItem(room.primaryImageUrl()));
    }
}

} // namespace aurel