if [ "$1"  == "tab" ]; then
	ADB="adb -s emulator-5554"
	SDK=36
	COMPILER="x86_64-linux-android${SDK}-clang++"
else
	ADB="adb -s $(adb devices | grep W8 | awk '{print $1}')"
	SDK=35
	COMPILER="aarch64-linux-android${SDK}-clang++"
fi;
HEADER="./src/include/"
CCROOT="$HOME/Android/Sdk/ndk/30.0.16138531/toolchains/llvm/prebuilt/linux-x86_64/bin"
CXXFLAGS="-static-libstdc++ --std=c++23 -O2 -Wall -Wextra -Werror \
    -Wshadow -Wconversion -Wsign-conversion \
    -Wpointer-arith -Wcast-qual \
    -Wformat=2 -Wundef \
    -Wpedantic \
    -Wnull-dereference \
    -Wdouble-promotion \
    -Woverloaded-virtual \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Wextra-semi \
    -Wimplicit-fallthrough \
    -fPIE \
    -fstack-protector-strong \
    -fno-omit-frame-pointer \
    -Wno-deprecated-declarations \
    -s"
${CCROOT}/${COMPILER} $CXXFLAGS -I$HEADER ${HEADER}/system_utils.cpp ${HEADER}/system_custom_props.cpp ${HEADER}/system_null.cpp ${HEADER}/test.cpp &>log;
$ADB shell rm -rf /data/local/tmp/a.out
$ADB push a.out /data/local/tmp
rm -rf a.out
$ADB shell chmod 755 /data/local/tmp/a.out
$ADB shell /data/local/tmp/a.out
$ADB shell cat /data/local/tmp/core
echo hitmark 2 del log
read foo
rm -rf log
