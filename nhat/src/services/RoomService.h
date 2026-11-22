#pragma once

#include "common/Types.h"
#include "domain/Room.h"
#include "domain/RoomType.h"
#include "repositories/RoomRepository.h"
#include <optional>
#include <vector>

namespace aurel {

class RoomService {
public:
    explicit RoomService(RoomRepository& roomRepository);

    // FR-01: Xem danh sách phòng (Guest & Customer)
    Result<std::vector<Room>> getFeaturedRooms() const;

    // FR-02: Tìm kiếm phòng theo ngày nhận/trả, số khách và loại phòng
    Result<std::vector<Room>> searchRooms(const RoomSearchCriteria& criteria) const;

    // FR-03: Xem chi tiết phòng
    Result<RoomDetails> getRoomDetails(RoomId roomId, const std::optional<StayPeriod>& stayPeriod) const;

    // FR-07: Hàm kiểm tra trùng lịch dùng chung (Người 3 gọi trong giao dịch xác nhận)
    Result<bool> isRoomAvailable(RoomId roomId, const StayPeriod& stayPeriod) const;

    // FR-13: Quản lý phòng (Yêu cầu quyền Admin)
    Result<std::vector<Room>> getAllRoomsForAdmin(const Session& session) const;
    Result<RoomId> createRoom(const Session& session, const Room& room,
                              const QStringList& imageUrls, const QStringList& facilityNames);
    Result<void> updateRoom(const Session& session, const Room& room,
                            const QStringList& imageUrls, const QStringList& facilityNames);
    Result<void> deleteRoom(const Session& session, RoomId roomId);

    // FR-14: Quản lý loại phòng (Yêu cầu quyền Admin)
    Result<std::vector<RoomType>> getAllRoomTypes() const;
    Result<RoomTypeId> createRoomType(const Session& session, const RoomType& roomType);
    Result<void> updateRoomType(const Session& session, const RoomType& roomType);
    Result<void> deleteRoomType(const Session& session, RoomTypeId roomTypeId);

private:
    RoomRepository& roomRepository_;
};

} // namespace aurel
