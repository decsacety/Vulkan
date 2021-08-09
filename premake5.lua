-- premake5.lua
-- version: premake-5.0.0-alpha14

-- %Y:\WorkSpace\themachinery-master% should set to the directory of The Machinery SDK

workspace "VKulkan"
    configurations { "Debug", "Release" }
    buildoptions "/WX-"  -- "/MDd" ,
    language "C++"
    cppdialect "C++17"   
    flags { "FatalWarnings", "MultiProcessorCompile" }
    warnings "Extra"
    inlining "Auto"
    sysincludedirs { "include" }
    targetdir "bin/%{cfg.buildcfg}"

filter "system:windows"
    platforms { "Win64" }
    systemversion "latest"

filter "platforms:Win64"
    defines { "_CRT_SECURE_NO_WARNINGS" }
    staticruntime "On"
    architecture "x64"
    libdirs { "bin/Debug"  } -- .. _ACTION .. "/%{cfg.buildcfg}"
    disablewarnings {
        "4057", -- Slightly different base types. Converting from type with volatile to without.
        "4100", -- Unused formal parameter. I think unusued parameters are good for documentation.
        "4152", -- Conversion from function pointer to void *. Should be ok.
        "4200", -- Zero-sized array. Valid C99.
        "4201", -- Nameless struct/union. Valid C11.
        "4204", -- Non-constant aggregate initializer. Valid C99.
        "4206", -- Translation unit is empty. Might be #ifdefed out.
        "4214", -- Bool bit-fields. Valid C99.
        "4221", -- Pointers to locals in initializers. Valid C99.
        "4702", -- Unreachable code. We sometimes want return after exit() because otherwise we get an error about no return value.
    }
    linkoptions { "/ignore:4099" } -- warning LNK4099: linking object as if no debug info

filter "configurations:Debug"
    staticruntime "Off"
    defines {  "DEBUG" }
    symbols "On"

filter "configurations:Release"
    defines {}
    optimize "On"

project "VKulkan"
    location "solutions"
    targetname "VKulkan"
    targetdir "bin/%{cfg.buildcfg}"
    kind "ConsoleApp"
    language "C++"
        files { "Vulkan.cpp" }
        links { "glfw3.lib" }
        links { "vulkan-1.lib" }