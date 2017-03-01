QTDIR=/usr/local/Qt-5.7.1

# Bootstrap KF5
#TODO

if [ "$(pwd | grep -o mbuild)" != "mbuild" ]; then
    # Create a build directory
    mkdir -p mbuild

    # Clear everything
    rm -rf ./mbuild/*

    cd mbuild

    PDIR=$PWD
else
    rm -rf ../mbuild/*
    PDIR=$PWD/../
fi

export QTANDROID_EXPORTED_TARGET=tutorial4

QT_PATH=/usr/local/Qt-5.7.1/ cmake $PDIR \
    -DCMAKE_PREFIX_PATH=$QTDIR/lib/cmake/ \
    -DANDROID_APK_DIR=$PDIR/android \
    -DQTANDROID_EXPORTED_TARGET=tutorial4 \
    -DCMAKE_TOOLCHAIN_FILE=$PDIR/kf5/extra-cmake-modules/toolchain/Android.cmake \
    -DDISABLE_SIGROK=1 -DBUILD_MOBILE=1 -Wno-dev -DCMAKE_INSTALL_PREFIX=$PDIR/kf5/native/

