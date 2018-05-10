TEMPLATE = app
TARGET = btchat

QT = core bluetooth widgets

CONFIG += c++11

SOURCES = \
    main.cpp \
    chat.cpp \
    remoteselector.cpp \
    chatserver.cpp \
    chatclient.cpp \
    ts_proto.c

HEADERS = \
    chat.h \
    remoteselector.h \
    chatserver.h \
    chatclient.h \
    ts_proto.h

FORMS = \
    chat.ui \
    remoteselector.ui

#target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/btchat
#INSTALLS += target
