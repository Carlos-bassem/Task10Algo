QT       += core gui widgets

TARGET   = CoinRemovalGUI
TEMPLATE = app

CONFIG  += c++17

SOURCES += \
    main.cpp \
    coinremovalgui.cpp

HEADERS += \
    coinremovalgui.h

# Suppress deprecation warnings for Qt5/Qt6 compatibility
DEFINES += QT_DEPRECATED_WARNINGS

# Default rules for deployment
qnx:   target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
