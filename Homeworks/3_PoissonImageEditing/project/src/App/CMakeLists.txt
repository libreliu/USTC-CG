if(MSVC)
Ubpa_AddTarget_GDR(MODE "EXE" QT ON
	LIBS_GENERAL "Qt5::Widgets"
	LIBS_DEBUG "opencv_world420d.lib"
	LIBS_RELEASE "opencv_world420.lib")
else()
Ubpa_AddTarget_GDR(MODE "EXE" QT ON
	LIBS_GENERAL "Qt5::Widgets"
	LIBS_DEBUG ${OpenCV_LIBS}
	LIBS_RELEASE ${OpenCV_LIBS})

endif()