-- Aurel House | SQLite schema v1
-- Based on Relational_Schema_Diagram.pdf and the Stage 3 team assignment.
-- Run on a new database. IF NOT EXISTS allows replay, but does not migrate older schemas.
-- Enable foreign_keys on every application connection, before beginning a transaction.
-- Dates use YYYY-MM-DD. Timestamps use UTC as YYYY-MM-DD HH:MM:SS.
-- USD amounts use integer cents. Percentage discounts use basis points (1000 = 10%).
PRAGMA foreign_keys = ON;

BEGIN;

CREATE TABLE IF NOT EXISTS "User" (
    userID        INTEGER PRIMARY KEY,
    fullName      TEXT NOT NULL,
    email         TEXT NOT NULL COLLATE NOCASE UNIQUE,
    phone         TEXT NOT NULL,
    passwordHash  TEXT NOT NULL,
    role          TEXT NOT NULL DEFAULT 'Customer' CHECK (role IN ('Customer', 'Admin')),
    accountStatus TEXT NOT NULL DEFAULT 'Active' CHECK (accountStatus IN ('Active', 'Locked')),
    createdAt     TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
) STRICT;

CREATE TABLE IF NOT EXISTS DiscountCode (
    discountID    INTEGER PRIMARY KEY,
    code          TEXT NOT NULL COLLATE NOCASE UNIQUE,
    discountType  TEXT NOT NULL CHECK (discountType IN ('Percentage', 'FixedAmount')),
    discountValue INTEGER NOT NULL CHECK (discountValue >= 0),
    minAmount     INTEGER NOT NULL DEFAULT 0 CHECK (minAmount >= 0),
    startDate     TEXT NOT NULL CHECK (date(startDate, '+0 days') IS startDate),
    expiryDate    TEXT NOT NULL CHECK (date(expiryDate, '+0 days') IS expiryDate),
    usageLimit    INTEGER CHECK (usageLimit >= 0),
    usedCount     INTEGER NOT NULL DEFAULT 0 CHECK (usedCount >= 0),
    status        TEXT NOT NULL DEFAULT 'Active' CHECK (status IN ('Active', 'Inactive')),
    CHECK (expiryDate >= startDate),
    CHECK (discountType <> 'Percentage' OR discountValue <= 10000),
    CHECK (usageLimit IS NULL OR usedCount <= usageLimit)
) STRICT;

CREATE TABLE IF NOT EXISTS RoomType (
    roomTypeID  INTEGER PRIMARY KEY,
    name        TEXT NOT NULL COLLATE NOCASE UNIQUE,
    description TEXT NOT NULL DEFAULT '',
    capacity    INTEGER NOT NULL CHECK (capacity > 0),
    basePrice   INTEGER NOT NULL CHECK (basePrice >= 0)
) STRICT;

CREATE TABLE IF NOT EXISTS Room (
    roomID        INTEGER PRIMARY KEY,
    roomTypeID    INTEGER NOT NULL REFERENCES RoomType(roomTypeID),
    roomNumber    TEXT NOT NULL UNIQUE,
    floor         INTEGER NOT NULL,
    description   TEXT NOT NULL DEFAULT '',
    pricePerNight INTEGER NOT NULL CHECK (pricePerNight >= 0),
    status        TEXT NOT NULL DEFAULT 'Available'
                  CHECK (status IN ('Available', 'Occupied', 'Cleaning', 'Maintenance'))
) STRICT;

CREATE TABLE IF NOT EXISTS Facility (
    facilityID INTEGER PRIMARY KEY,
    name       TEXT NOT NULL COLLATE NOCASE UNIQUE
) STRICT;

CREATE TABLE IF NOT EXISTS Room_Facility (
    roomID     INTEGER NOT NULL REFERENCES Room(roomID) ON DELETE CASCADE,
    facilityID INTEGER NOT NULL REFERENCES Facility(facilityID) ON DELETE CASCADE,
    PRIMARY KEY (roomID, facilityID)
) STRICT;

CREATE TABLE IF NOT EXISTS RoomImage (
    imageID  INTEGER PRIMARY KEY,
    roomID   INTEGER NOT NULL REFERENCES Room(roomID) ON DELETE CASCADE,
    imageUrl TEXT NOT NULL
) STRICT;

CREATE TABLE IF NOT EXISTS Booking (
    bookingID        INTEGER PRIMARY KEY,
    bookingCode      TEXT NOT NULL COLLATE NOCASE UNIQUE, -- Public confirmation code.
    userID           INTEGER NOT NULL REFERENCES "User"(userID),
    roomID           INTEGER NOT NULL REFERENCES Room(roomID),
    discountID       INTEGER REFERENCES DiscountCode(discountID),
    checkInDate      TEXT NOT NULL CHECK (date(checkInDate, '+0 days') IS checkInDate),
    checkOutDate     TEXT NOT NULL CHECK (date(checkOutDate, '+0 days') IS checkOutDate),
    actualCheckInAt  TEXT,
    actualCheckOutAt TEXT,
    numGuests        INTEGER NOT NULL CHECK (numGuests > 0),
    pricePerNight    INTEGER NOT NULL CHECK (pricePerNight >= 0),
    discountAmount   INTEGER NOT NULL DEFAULT 0 CHECK (discountAmount >= 0),
    totalPrice       INTEGER NOT NULL CHECK (totalPrice >= 0),
    bookingStatus    TEXT NOT NULL DEFAULT 'Confirmed'
                     CHECK (bookingStatus IN ('Confirmed', 'CheckedIn', 'Completed', 'Cancelled')),
    createdAt        TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    cancelledAt      TEXT,
    cancelledBy     INTEGER REFERENCES "User"(userID), -- FR-09 / FR-17.
    cancellationReason TEXT,
    CHECK (checkOutDate > checkInDate),
    CHECK (actualCheckOutAt IS NULL OR
           (actualCheckInAt IS NOT NULL AND actualCheckOutAt >= actualCheckInAt))
) STRICT;

-- FR-16 / FR-21: the application verifies gateway result and amount first.
-- In one write transaction, recheck availability, create Booking, link the
-- paid attempt, update voucher usage if applicable and save the confirmation.
-- Generate a unique paymentSessionID for each attempt, including each retry.
-- Keep failed or cancelled attempts without a bookingID.
CREATE TABLE IF NOT EXISTS Payment (
    paymentID        INTEGER PRIMARY KEY,
    bookingID        INTEGER REFERENCES Booking(bookingID),
    paymentSessionID TEXT NOT NULL UNIQUE,
    amount           INTEGER NOT NULL CHECK (amount >= 0),
    method           TEXT NOT NULL,
    transactionRef   TEXT UNIQUE,
    paymentStatus    TEXT NOT NULL DEFAULT 'Pending'
                     CHECK (paymentStatus IN ('Pending', 'Paid', 'Failed', 'Cancelled')),
    paidAt           TEXT,
    createdAt        TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CHECK (paymentStatus <> 'Paid' OR
           (paidAt IS NOT NULL AND transactionRef IS NOT NULL)),
    CHECK (bookingID IS NULL OR paymentStatus = 'Paid')
) STRICT;

CREATE TABLE IF NOT EXISTS Notification (
    notificationID INTEGER PRIMARY KEY,
    bookingID      INTEGER NOT NULL REFERENCES Booking(bookingID),
    eventType      TEXT NOT NULL,
    recipientEmail TEXT NOT NULL,
    message        TEXT NOT NULL,
    status         TEXT NOT NULL CHECK (status IN ('Sent', 'Failed')),
    sentAt         TEXT,
    CHECK (status <> 'Sent' OR sentAt IS NOT NULL)
) STRICT;

CREATE TABLE IF NOT EXISTS Review (
    reviewID  INTEGER PRIMARY KEY,
    bookingID INTEGER NOT NULL UNIQUE REFERENCES Booking(bookingID),
    roomID    INTEGER NOT NULL REFERENCES Room(roomID),
    rating    INTEGER NOT NULL CHECK (rating BETWEEN 1 AND 5),
    comment   TEXT NOT NULL DEFAULT '',
    createdAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
) STRICT;

-- FR-09 / FR-17 / FR-20: separate refunds from original verified payments.
-- The cancellation service checks the refund policy and total refundable amount.
CREATE TABLE IF NOT EXISTS Refund (
    refundID      INTEGER PRIMARY KEY,
    paymentID     INTEGER NOT NULL REFERENCES Payment(paymentID),
    amount        INTEGER NOT NULL CHECK (amount > 0),
    refundStatus  TEXT NOT NULL DEFAULT 'Pending'
                  CHECK (refundStatus IN ('Pending', 'Refunded', 'Failed')),
    refundRef     TEXT UNIQUE,
    refundedAt    TEXT,
    createdAt     TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CHECK (refundStatus <> 'Refunded' OR refundedAt IS NOT NULL)
) STRICT;

-- FR-25 / FR-26: save the issued values for later viewing, printing and export.
CREATE TABLE IF NOT EXISTS BookingConfirmation (
    bookingID      INTEGER PRIMARY KEY REFERENCES Booking(bookingID),
    guestName      TEXT NOT NULL,
    roomNumber     TEXT NOT NULL,
    roomTypeName   TEXT NOT NULL,
    checkInDate    TEXT NOT NULL CHECK (date(checkInDate, '+0 days') IS checkInDate),
    checkOutDate   TEXT NOT NULL CHECK (date(checkOutDate, '+0 days') IS checkOutDate),
    numGuests      INTEGER NOT NULL CHECK (numGuests > 0),
    amountPaid     INTEGER NOT NULL CHECK (amountPaid >= 0),
    bookingStatus  TEXT NOT NULL DEFAULT 'Confirmed' CHECK (bookingStatus = 'Confirmed'),
    issuedAt       TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CHECK (checkOutDate > checkInDate)
) STRICT;

CREATE INDEX IF NOT EXISTS idx_room_room_type ON Room(roomTypeID);
CREATE INDEX IF NOT EXISTS idx_room_image_room ON RoomImage(roomID);
CREATE INDEX IF NOT EXISTS idx_room_facility_facility ON Room_Facility(facilityID);
CREATE INDEX IF NOT EXISTS idx_booking_user_created ON Booking(userID, createdAt DESC);
CREATE INDEX IF NOT EXISTS idx_booking_discount ON Booking(discountID);
CREATE INDEX IF NOT EXISTS idx_booking_room_dates ON Booking(roomID, checkInDate, checkOutDate);
CREATE UNIQUE INDEX IF NOT EXISTS uq_payment_booking
    ON Payment(bookingID) WHERE bookingID IS NOT NULL;
CREATE INDEX IF NOT EXISTS idx_notification_booking ON Notification(bookingID);
CREATE INDEX IF NOT EXISTS idx_review_room ON Review(roomID);
CREATE INDEX IF NOT EXISTS idx_refund_payment ON Refund(paymentID);

-- Stays occupy [checkInDate, checkOutDate). Same-day turnover is allowed.
CREATE TRIGGER IF NOT EXISTS booking_prevent_overlap_on_insert
BEFORE INSERT ON Booking
WHEN NEW.bookingStatus IN ('Confirmed', 'CheckedIn') AND EXISTS (
    SELECT 1
    FROM Booking AS existing
    WHERE existing.roomID = NEW.roomID
      AND existing.bookingStatus IN ('Confirmed', 'CheckedIn')
      AND NEW.checkInDate < existing.checkOutDate
      AND existing.checkInDate < NEW.checkOutDate
)
BEGIN
    SELECT RAISE(ABORT, 'Room already booked for these dates');
END;

CREATE TRIGGER IF NOT EXISTS booking_prevent_overlap_on_update
BEFORE UPDATE OF roomID, checkInDate, checkOutDate, bookingStatus ON Booking
WHEN NEW.bookingStatus IN ('Confirmed', 'CheckedIn') AND EXISTS (
    SELECT 1
    FROM Booking AS existing
    WHERE existing.roomID = NEW.roomID
      AND existing.bookingID <> OLD.bookingID
      AND existing.bookingStatus IN ('Confirmed', 'CheckedIn')
      AND NEW.checkInDate < existing.checkOutDate
      AND existing.checkInDate < NEW.checkOutDate
)
BEGIN
    SELECT RAISE(ABORT, 'Room already booked for these dates');
END;

COMMIT;
