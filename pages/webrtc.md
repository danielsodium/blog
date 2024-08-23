# Setting up WebRTC AndroidVOIP

These are the steps I took to setup my development environment for WebRTC Android Native development.

## Prerequisites
Clone the depot_tools into your home directory.
`git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git`

Then add to your `.bashrc`
`export PATH=/path/to/depot_tools:$PATH`

## Getting WebRTC
Create a directory to store the webrtc source code (or just clone into your home directory like me). This cloned directory will be referred to as `src` in a lot of WebRTC documentation.
`fetch --nohooks webrtc_android`

then sync with gclient (I have no clue what this does)
`gclient sync`

now `cd` into the newly created directory
`cd src`

## Start Developing
Every time you start a terminal session, run this in `src`
`./build/install-build-deps.sh`

This will give you access to adb which is needed to run android.

## Building
To build the project, run
`gn gen out/Debug --args='target_os="android" target_cpu="x64"'`
`ninja -C out/Default AppRTCMobile`

This will take more than a few minutes, and the newly created files will be in `out/Default`.

## Testing
Currently, Android Studio doesn't work, but we can still take advantage of Android Studio's Virtual Device Manager to emulate a android device.

The default emulation device will not work, citing a APK version discrepency. This can be resolved by editing one of the adb flags, but the app will crash due to an older version of apk even if this error is ignored. 

To resolve this, create a new device of any size, and for the system image, go under "recommended" and select "Tiramisu" which has API 33, ABI x86_64, and Android 13.0 (Google Play). Install and launch this device.

Then run to deploy the app.
`adb install -r out/Default/apks/androidvoip.apk`

The app will not automatically run, instead swipe up on the home screen to find the "androidvoip" app and click to run.

## Editing
To setup clangd, we can make use of `compile_commands.json`. In `src` run
`gn gen out/Default --export-compile-commands`

Then add as a compile command in whatever editor you use
`--compile-commands-dir=/home/daniel/src/out/Default`

and you should be good to go.
