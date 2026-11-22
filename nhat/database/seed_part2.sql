PRAGMA foreign_keys = ON;
BEGIN;

-- 1. Tài khoản mẫu (Customer & Admin)
INSERT OR IGNORE INTO "User" (userID, fullName, email, phone, passwordHash, role, accountStatus) VALUES
(1, 'Nguyen Van An', 'an.customer@aurel.vn', '0901234567', 'hash_cust_123', 'Customer', 'Active'),
(2, 'Tran Thi Binh', 'binh.admin@aurel.vn', '0909876543', 'hash_admin_123', 'Admin', 'Active');

-- 2. Loại phòng (RoomType) - Giá tính bằng cent (USD)
INSERT OR IGNORE INTO RoomType (roomTypeID, name, description, capacity, basePrice) VALUES
(1, 'Standard Single', 'Phòng tiêu chuẩn ấm cúng cho 1 khách, đầy đủ tiện nghi cơ bản.', 1, 8000),
(2, 'Deluxe King', 'Phòng hạng sang giường King lớn, ban công hướng phố.', 2, 15000),
(3, 'Executive Suite', 'Căn hộ cao cấp phòng khách riêng biệt, bồn tắm nằm.', 4, 28000),
(4, 'Empty Unused Type', 'Loại phòng chưa có phòng nào (dùng để test xóa thành công).', 2, 10000);

-- 3. Phòng (Room) - Bao gồm phòng 204 theo quy ước nhóm
INSERT OR IGNORE INTO Room (roomID, roomTypeID, roomNumber, floor, description, pricePerNight, status) VALUES
(101, 1, '101', 1, 'Phòng tầng 1 yên tĩnh, gần sảnh.', 8000, 'Available'),
(204, 2, '204', 2, 'Phòng Deluxe King tiêu chuẩn mẫu của hệ thống.', 16500, 'Available'),
(205, 2, '205', 2, 'Phòng Deluxe King góc view đẹp.', 15000, 'Maintenance'),
(301, 3, '301', 3, 'Phòng Suite tầng cao nhất.', 30000, 'Available');

-- 4. Tiện nghi (Facility) & Liên kết (Room_Facility)
INSERT OR IGNORE INTO Facility (facilityID, name) VALUES
(1, 'Free High-Speed Wi-Fi'),
(2, 'Air Conditioning'),
(3, 'Smart TV 55-inch'),
(4, 'Mini Bar'),
(5, 'Ocean / City View Balcony'),
(6, 'Bathtub');

INSERT OR IGNORE INTO Room_Facility (roomID, facilityID) VALUES
(101, 1), (101, 2),
(204, 1), (204, 2), (204, 3), (204, 4), (204, 5),
(205, 1), (205, 2), (205, 3),
(301, 1), (301, 2), (301, 3), (301, 4), (301, 5), (301, 6);

-- 5. Hình ảnh phòng (RoomImage)
INSERT OR IGNORE INTO RoomImage (imageID, roomID, imageUrl) VALUES
(1, 101, ':/images/room_101.png'),
(2, 204, ':/images/deluxe_king_main.png'),
(3, 204, ':/images/deluxe_king_bath.png'),
(4, 301, ':/images/suite_301.png');

-- 6. Booking mẫu để test FR-07 (Trùng lịch) & FR-13 (Chặn xóa phòng có booking)
-- Phòng 204 đã có khách đặt từ 2026-12-10 đến 2026-12-15 (Confirmed)
INSERT OR IGNORE INTO Booking (
    bookingID, bookingCode, userID, roomID, checkInDate, checkOutDate,
    numGuests, pricePerNight, discountAmount, totalPrice, bookingStatus
) VALUES
(1, 'AH-2026-0001', 1, 204, '2026-12-10', '2026-12-15', 2, 16500, 0, 82500, 'Confirmed'),
(2, 'AH-2026-0002', 1, 204, '2026-11-01', '2026-11-03', 2, 16500, 0, 33000, 'Completed');

-- 7. Đánh giá mẫu cho phòng 204 (Hiển thị ở FR-03 Room Details)
INSERT OR IGNORE INTO Review (reviewID, bookingID, roomID, rating, comment) VALUES
(1, 2, 204, 5, 'Phòng 204 rất sạch sẽ, giường êm và view ban công cực đẹp!');

COMMIT;