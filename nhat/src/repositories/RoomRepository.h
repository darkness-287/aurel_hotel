#pragma once

#include "common/Types.h"
#include "domain/Room.h"
#include "domain/RoomType.h"
#include <QSqlDatabase>
#include <optional>
#include <vector>

namespace aurel {

struct RoomSearchCriteria {
    std::optional<StayPeriod> stayPeriod;
    int minGuests{1};
    std::optional<RoomTypeId> roomTypeId;
    bool onlyAvailableForBooking{true};
};

class RoomRepository {
public:
    explicit RoomRepository(QSqlDatabase db);

    // FR-01, FR-02: Lấy danh sách & tìm kiếm phòng
    Result<std::vector<Room>> findAllRooms() const;
    Result<std::vector<Room>> searchRooms(const RoomSearchCriteria& criteria) const;

    // FR-03: Chi tiết phòng kèm tiện nghi, ảnh, đánh giá
    Result<RoomDetails> getRoomDetails(RoomId roomId, const std::optional<StayPeriod>& stayPeriod) const;

    // FR-07: Kiểm tra trùng lịch dùng chung (Người 3 gọi lại trong giao dịch xác nhận)
    Result<bool> isRoomAvailable(RoomId roomId, const StayPeriod& stayPeriod) const;

    // FR-13: Quản lý phòng (CRUD + kiểm tra ràng buộc xóa)
    Result<RoomId> createRoom(const Room& room, const QStringList& imageUrls, const QStringList& facilityNames);
    Result<void> updateRoom(const Room& room, const QStringList& imageUrls, const QStringList& facilityNames);
    Result<bool> hasReferencingBookings(RoomId roomId) const;
    Result<void> deleteRoom(RoomId roomId);

    // FR-14: Quản lý loại phòng (CRUD + kiểm tra ràng buộc xóa)
    Result<std::vector<RoomType>> findAllRoomTypes() const;
    Result<RoomTypeId> createRoomType(const RoomType& roomType);
    Result<void> updateRoomType(const RoomType& roomType);
    Result<bool> hasRoomsUsingType(RoomTypeId roomTypeId) const;
    Result<void> deleteRoomType(RoomTypeId roomTypeId);

private:
    QStringList loadFacilitiesForRoom(RoomId roomId) const;
    QStringList loadImagesForRoom(RoomId roomId) const;
    void syncRoomImagesAndFacilities(RoomId roomId, const QStringList& imageUrls, const QStringList& facilityNames);

    QSqlDatabase db_;
};

} // namespace aurel