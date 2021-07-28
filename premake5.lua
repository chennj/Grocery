workspace "Grocery"
	architecture "x86_64"
	startproject "DemoRoom"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Import ---------------------------------------

-- ---------------------------------------------------

project "DemoRoom"
	location "DemoRoom"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("temp/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"DemoRoom/src",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "DemoRoom_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "DemoRoom_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "DemoRoom_DIST"
		runtime "Release"
		optimize "on"

	filter {"system:windows","configurations:Debug"}
		buildoptions "/MTd"

	filter {"system:windows","configurations:Release"}
		buildoptions "/MT"

	filter {"system:windows","configurations:Dist"}
		buildoptions "/MT"

-- ---------------------------------------------------

project "CommonDll"
	location "CommonDll"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("temp/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"CommonDll/src",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "CommonDll_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "CommonDll_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "CommonDll_DIST"
		runtime "Release"
		optimize "on"

	filter {"system:windows","configurations:Debug"}
		buildoptions "/MTd"

	filter {"system:windows","configurations:Release"}
		buildoptions "/MT"

	filter {"system:windows","configurations:Dist"}
		buildoptions "/MT"
