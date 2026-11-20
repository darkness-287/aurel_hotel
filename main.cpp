#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QLabel window("Hotel Room Booking System");
    window.setWindowTitle("Aurel House");
    window.resize(480, 240);
    window.show();

    return app.exec();
}