include(GenerateExportHeader)

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

set(qjoypad_SOURCES 
	axis.cpp
	axis_edit.cpp
	axisw.cpp
	button.cpp
	button_edit.cpp
	buttonw.cpp
	event.cpp
	flash.cpp
	floatingicon.cpp
	joypad.cpp
	joypadw.cpp
	joyslider.cpp
	keycode.cpp
	keydialog.cpp
	layout.cpp
	layout_edit.cpp
	main.cpp
	quickset.cpp
	trayicon.cpp
)

set(qjoypad_QOBJECT_HEADERS
	axis_edit.h
	axis.h
	axisw.h
	button_edit.h
	button.h
	buttonw.h
	flash.h
	floatingicon.hpp
	joypad.h
	joypadw.h
	joyslider.h
	keycode.h
	keydialog.hpp
	layout_edit.h
	layout.h
	quickset.h
	trayicon.hpp
)

qt5_wrap_cpp(qjoypad_HEADERS_MOC ${qjoypad_QOBJECT_HEADERS})
add_executable(qjoypad ${qjoypad_SOURCES} ${qjoypad_HEADERS_MOC})
target_link_libraries(qjoypad Qt5::Widgets Qt5::X11Extras Xtst X11 ${LIBUDEV_LIBRARIES})

install(TARGETS qjoypad RUNTIME DESTINATION "bin")
