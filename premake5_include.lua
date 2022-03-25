-- To compile BSPParser with a garrysmod_common project

project("BSPParser")
kind("StaticLib")
language("C++")
targetdir("premakeout/%{cfg.buildcfg}")

files({ "**.h", "**.cpp" })

filter("configurations:ReleaseWithSymbols")
defines("NDEBUG")
symbols("On")

filter("configurations:Release")
defines("NDEBUG")
optimize("On")
