#!/bin/bash

/android-sdk-linux/platform-tools/adb logcat -dn 300 | grep 'I/DEBUG' | cut -f2-99 -d':' > /tmp/crash
/android-ndk-r10e/ndk-stack -sym $PWD/tutorial4_build_apk/libs/armeabi-v7a/ -dump /tmp/crash
