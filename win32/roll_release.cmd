@echo off

:: Update the qt_dir as necessary
set qt_dir=K:\Qt\Qt5.1.0\5.1.0\mingw48_32
set qif_dir=K:\Qt\QtIFW-1.3.0\bin
set openssl_dir=C:\OpenSSL-Win32

cd packages\pumpa\data

:: Removing existing dlls
del /q imageformats
del /q phonon5.dll
del /q Qt5Core.dll
del /q Qt5OpenGL.dll
del /q Qt5PrintSupport.dll
del /q Qt5Quick.dll
del /q Qt5Qml.dll
del /q Qt5V8.dll
del /q Qt5Sql.dll
del /q Qt5Gui.dll
del /q Qt5Widgets.dll
del /q Qt5Network.dll
del /q Qt5WebKit.dll
del /q Qt5WebKitWidgets.dll
del /q Qt5Multimedia.dll
del /q Qt5MultimediaWidgets.dll
del /q libGLESv2.dll
del /q icudt51.dll
del /q icuin51.dll
del /q icuuc51.dll
del /q libEGL.dll
del /q mingw*.dll
del /q libgcc*.dll
del /q platforms
del /q D3DCompiler_46.dll
del /q pumpa.exe
del /q lib*.dll
del /q libeay32.dll
del /q ssleay32.dll
del /q libssl32.dll

:: Copying new dlls
xcopy /I %qt_dir%\plugins\imageformats imageformats
xcopy %qt_dir%\bin\phonon5.dll
xcopy %qt_dir%\bin\Qt5Core.dll
xcopy %qt_dir%\bin\Qt5OpenGL.dll
xcopy %qt_dir%\bin\Qt5PrintSupport.dll
xcopy %qt_dir%\bin\Qt5Quick.dll
xcopy %qt_dir%\bin\Qt5Qml.dll
xcopy %qt_dir%\bin\Qt5V8.dll
xcopy %qt_dir%\bin\Qt5Sql.dll
xcopy %qt_dir%\bin\Qt5Gui.dll
xcopy %qt_dir%\bin\Qt5Widgets.dll
xcopy %qt_dir%\bin\Qt5Network.dll
xcopy %qt_dir%\bin\Qt5WebKit.dll
xcopy %qt_dir%\bin\Qt5WebKitWidgets.dll
xcopy %qt_dir%\bin\Qt5Multimedia.dll
xcopy %qt_dir%\bin\Qt5MultimediaWidgets.dll
xcopy %qt_dir%\bin\libGLESv2.dll
xcopy %qt_dir%\bin\icudt51.dll
xcopy %qt_dir%\bin\icuin51.dll
xcopy %qt_dir%\bin\icuuc51.dll
xcopy %qt_dir%\bin\libEGL.dll
xcopy %qt_dir%\bin\libgcc_s_dw2-1.dll
xcopy %qt_dir%\bin\libwinpthread-1.dll
xcopy %qt_dir%\bin\libstdc++-6.dll
xcopy %qt_dir%\bin\D3DCompiler_46.dll


xcopy %qt_dir%\plugins\platforms\qminimal.dll platforms\
xcopy %qt_dir%\plugins\platforms\qwindows.dll platforms\

xcopy %openssl_dir%\ssleay32.dll
xcopy %openssl_dir%\libssl32.dll
xcopy %openssl_dir%\libeay32.dll

xcopy ..\..\..\..\release\pumpa.exe

cd ..\..\..\

%qif_dir%\binarycreator.exe --offline-only --config config\config.xml --packages packages pumpa-installer.exe