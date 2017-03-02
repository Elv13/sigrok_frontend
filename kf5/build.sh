#!/bin/bash

export ANDROID_NDK=/android-ndk-r10e
export ANDROID_SDK_ROOT=/android-sdk-linux
Qt5_android=$ADIR/Qt5.5.0/5.5/android_armv7/

# Build KF5, run and pray

function build_framework()
{
    echo -e "\n\n\n\n" BUILDING: $1

    if [ ! -d $1 ]; then
        git clone https://anongit.kde.org/$1
    fi

    mkdir -p build/$1
    cd build/$1
    cmake ../../$1 -DCMAKE_INSTALL_PREFIX=../../native \
        -DCMAKE_PREFIX_PATH=/usr/local/Qt-5.7.1/lib/cmake/ \
        -DCMAKE_TOOLCHAIN_FILE=../extra-cmake-modules/toolchain/Android.cmake
        -Wno-dev \
        -DCMAKE_INSTALL_PREFIX=../../native

    make -j8
    make install

    cd ../..
}

mkdir -p native

build_framework extra-cmake-modules || exit

build_framework kcoreaddons || exit
build_framework kitemmodels || exit
build_framework kwidgetsaddons || exit

# build_framework kconfig || exit
#build_framework kconfigwidgets || exit
# build_framework ki18n || exit
# build_framework kxmlgui

### QWT

cd build
unzip ../qwt-6.1.3.zip
cd qwt-6.1.3
export CXX=/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++
sed 's/#QWT_CONFIG     += QwtDesigner/#/' -i qwtconfig.pri

# Disable printing support
echo 'DEFINES += QT_NO_PRINTER=1' >> qwtconfig.pri
sed 's/Qt5PrintSupport//' -i src/src.pro
/usr/local/Qt-5.7.1/bin/qmake  -r -spec android-g++ CONFIG+=debug QT_INSTALL_PREFIX=$PWD/../
sed 's/-lQt5PrintSupport//' -i src/MakeFile

make -j8
