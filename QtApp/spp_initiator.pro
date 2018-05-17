TEMPLATE = app

QT = core bluetooth widgets

CONFIG += c++11

SOURCES = \
    main.cpp \
    chat.cpp \
    remoteselector.cpp \
    ts_proto.c \
    ts_proto_client.cpp

HEADERS = \
    chat.h \
    remoteselector.h \
    ts_proto.h \
    ts_proto_client.h \
    average_buffer.h

FORMS = \
    chat.ui \
    remoteselector.ui
