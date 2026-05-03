#include <QApplication>
#include "coinremovalgui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Coin Removal Algorithm");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Task10Algo");

    CoinRemovalWindow window;
    window.show();

    return app.exec();
}
