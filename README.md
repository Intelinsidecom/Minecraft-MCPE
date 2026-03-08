# Minecraft-MCPE-Alpha-0.6.1

This repo includes the leak files for MCPE 0.6.1 version. Im looking into adding more features to make it better and interesting to play. (ports, adjustments, fixes) 

# Todos
- Build the project for all supported platforms and confirm they all work fine.
  - [x] Android
  - [ ] IOS
  - [ ] Raspberry PI
  - [ ] Server
  - [x] Win32 (GL project shouldve used OpenGL (not GLES) but its buggy with OpenGL and GLES is almost the same so its deprecated and will be replaced as DirectX version before UI changes are done)
- [ ] Make the Win32 Release more friendly to end-user. (hiding console on release, adding icon and proper name for the app, add mouse support and fix up the keyboard binds properly.)

- Add Ports to other Platforms
  - [ ] Linux
  - [ ] WinRT
  - [ ] UWP (Windows Phone, Desktop, Xbox)


- [ ] Add Proper support for more Graphics API's (DirectX, Vulkan)
- [ ] Design Proper Settings Menu or Redesign the whole UI to look and functions more like Bedrock
- [ ] Backport Legacy Console, Bedrock and Java Features or adjustmnets so it feels more like Regular Minecraft.

# Current bugs
- Sound doesnt work on Win32.

# Building

## Win32
1. Obtain Visual Studio with vs_xxx C++ toolchain
2. Open the sln project file in VS
3. Choose build config you prefer
4. Build

To Play mount the game from visual studio since my fix doesnt actually change anything in some way?

## Android
- This is where things are tricker but still managable

1. Download NDK r10e from here for your platform:
https://github.com/android/ndk/wiki/unsupported-downloads
2. Add the r10e to PATH so ndk-build is detected but dont add to ANDROID_HOME at any costs, it will break everything
3. Download Android Studio and for SDK, choose an folder that isnt the NDK, then choose Android-9 platform (gingerbread) and for sdk, look for 20.5xxxxxx version.
4. once sdk and ndk part is done, go to the android folder in the project folder and open the terminal or command prompt
5. then run gradlew to initialize everything and to make sure everything will work fine
6. once gradlew command returns "BUILD SUCCESSFUL", then run ndk-build and confirm it builds the so library for minecraft.
7. Run gradlew build to compile the apk and to package everything
8. Sideload to whatewer device you want and run it

Notes for modern android users:
  - The UI on big screens is going to be small (change DPI if you really need), this is going to be fixed when UI overhaul is going to happen.

        
  
