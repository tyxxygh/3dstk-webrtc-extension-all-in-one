#!/bin/sh

#Note [TBD] : There is no check for ndk-version
#Please use the ndk-version as per host machine for now

#Get the machine type
PROCTYPE=`uname -m`

if [ "$PROCTYPE" = "i686" ] || [ "$PROCTYPE" = "i386" ] || [ "$PROCTYPE" = "i586" ] ; then
        echo "Host machine : x86"
        ARCHTYPE="x86"
else
        echo "Host machine : x86_64"
        ARCHTYPE="x86_64"
fi

#Get the Host OS
HOST_OS=`uname -s`
case "$HOST_OS" in
    Darwin)
        HOST_OS=darwin
        ;;
    Linux)
        HOST_OS=linux
        ;;
esac

#NDK-path
if [[ $1 == *ndk* ]]; then
	echo "----------------- NDK Path is : $1 ----------------"
	Input=$1;
else
	echo "Please enter your android ndk path:"
	echo "For example:/home/astro/android-ndk-r10c"
	read Input
	echo "You entered:$Input"
fi

#Set path
echo "----------------- Exporting the android-ndk path ----------------"
#export PATH=$PATH:$Input:$Input/toolchains/llvm-3.5/prebuilt/$HOST_OS-$ARCHTYPE/bin

#create install directories
mkdir -p ./../../../build
mkdir -p ./../../../build/android


#zsLib module build
echo "------------------- Building idnkit for ANDROID platform ---------------"
pushd `pwd`
mkdir -p ./../../../build/android/idnkit

rm -rf ./obj/*
export ANDROIDNDK_PATH=$Input
export NDK_PROJECT_PATH=`pwd`

ndk-build  NDK_DEBUG=1 APP_PLATFORM=android-19
popd

echo "-------- Installing idnkit libs -----"
cp -r ./obj/local/armeabi/lib* ./../../../build/android/idnkit/

#clean
rm -rf ./obj

