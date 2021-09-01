# Linking options for different platforms

linux: LIBS += -lxcb -lxcb-keysyms
mac: LIBS += -framework Carbon

windows {
    *-g++* {
        LIBS += -luser32
    }
    *-msvc* {
        LIBS += user32.lib
    }
}
