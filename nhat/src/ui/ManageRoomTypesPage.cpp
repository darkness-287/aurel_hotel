// src/ui/ManageRoomTypesPage.cpp
#include "ui/ManageRoomTypesPage.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace aurel {

ManageRoomTypesPage::ManageRoomTypesPage(RoomService& roomService, const Session& session, QWidget* parent)
    : QWidget(parent), roomService_(roomService), session_(session) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("<h2>Admin — Quản lý Loại phòng (FR-14)</h2>"));

    typesTable_ = new QTableWidget(0, 5, this);
    typesTable_->setHorizontalHeaderLabels({"ID", "Tên loại phòng", "Sức chứa", "Giá cơ bản (cent)", "Mô tả"});
    typesTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    typesTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    typesTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(typesTable_);

    auto* form = new QFormLayout();
    nameEdit_ = new QLineEdit();
    capacitySpin_ = new QSpinBox();
    capacitySpin_->setRange(1, 20);
    basePriceSpin_ = new QSpinBox();
    basePriceSpin_->setRange(0, 10000000);
    basePriceSpin_->setSingleStep(500);
    descEdit_ = new QLineEdit();

    form->addRow("Tên loại phòng:", nameEdit_);
    form->addRow("Sức chứa:", capacitySpin_);
    form->addRow("Giá cơ bản (cent):", basePriceSpin_);
    form->addRow("Mô tả:", descEdit_);
    layout->addLayout(form);

    auto* btnLayout = new QHBoxLayout();
    auto* addBtn = new QPushButton("Thêm loại phòng");
    auto* updateBtn = new QPushButton("Cập nhật loại phòng");
    auto* deleteBtn = new QPushButton("Xóa loại phòng (Demo chặn xóa nếu còn phòng)");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(updateBtn);
    btnLayout->addWidget(deleteBtn);
    layout->addLayout(btnLayout);

    connect(typesTable_, &QTableWidget::cellClicked, this, [this](int row, int) {
        selectedTypeId_ = typesTable_->item(row, 0)->text().toInt();
        nameEdit_->setText(typesTable_->item(row, 1)->text());
        capacitySpin_->setValue(typesTable_->item(row, 2)->text().toInt());
        basePriceSpin_->setValue(typesTable_->item(row, 3)->text().toInt());
        descEdit_->setText(typesTable_->item(row, 4)->text());
    });

    connect(addBtn, &QPushButton::clicked, this, [this]() {
        RoomType rt(0, nameEdit_->text(), descEdit_->text(), capacitySpin_->value(), basePriceSpin_->value());
        auto res = roomService_.createRoomType(session_, rt);
        if (!res.ok) {
            QMessageBox::warning(this, "Từ chối", res.message);
            return;
        }
        QMessageBox::information(this, "Thành công", "Đã thêm loại phòng.");
        refreshData();
        emit roomTypesChanged();
    });

    connect(updateBtn, &QPushButton::clicked, this, [this]() {
        if (selectedTypeId_ <= 0) return;
        RoomType rt(selectedTypeId_, nameEdit_->text(), descEdit_->text(), capacitySpin_->value(), basePriceSpin_->value());
        auto res = roomService_.updateRoomType(session_, rt);
        if (!res.ok) {
            QMessageBox::warning(this, "Từ chối", res.message);
            return;
        }
        QMessageBox::information(this, "Thành công", "Đã cập nhật loại phòng.");
        refreshData();
        emit roomTypesChanged();
    });

    connect(deleteBtn, &QPushButton::clicked, this, [this]() {
        if (selectedTypeId_ <= 0) return;
        auto res = roomService_.deleteRoomType(session_, selectedTypeId_);
        if (!res.ok) {
            QMessageBox::critical(this, "Xóa bị từ chối (FR-14)", res.message);
            return;
        }
        QMessageBox::information(this, "Thành công", "Đã xóa loại phòng.");
        selectedTypeId_ = 0;
        refreshData();
        emit roomTypesChanged();
    });

    refreshData();
}

void ManageRoomTypesPage::setSession(const Session& session) {
    session_ = session;
    refreshData();
}

void ManageRoomTypesPage::refreshData() {
    typesTable_->setRowCount(0);
    auto res = roomService_.getAllRoomTypes();
    if (!res.ok) return;

    for (const auto& t : res.value) {
        int row = typesTable_->rowCount();
        typesTable_->insertRow(row);
        typesTable_->setItem(row, 0, new QTableWidgetItem(QString::number(t.id())));
        typesTable_->setItem(row, 1, new QTableWidgetItem(t.name()));
        typesTable_->setItem(row, 2, new QTableWidgetItem(QString::number(t.capacity())));
        typesTable_->setItem(row, 3, new QTableWidgetItem(QString::number(t.basePriceCents())));
        typesTable_->setItem(row, 4, new QTableWidgetItem(t.description()));
    }
}

} // namespace aurel
