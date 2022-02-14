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
-- 定义函数，包含三方库头文件，可被其他工程调用
function include3DMAX()
	-- includedirs "D:/写意公司/3ds Max 2014 SDK/include"
end
-- 定义函数，链接三方库
function link3DMAX()
	-- 指定lib的文件路径
	-- libdirs "D:/写意公司/3ds Max 2014 SDK/lib/x64/Release"
	-- 指定lib文件名，Maxscrpt.lib，此处使用的是动态库
	-- links "Maxscrpt"
end

project "DemoRoom"
	location "DemoRoom"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("temp/" .. outputdir .. "/%{prj.name}")

	include3DMAX()
	link3DMAX()
	
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
-- 增量文件多线程扫描
project "DeltaFileScan"
	location "DeltaFileScan"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("temp/" .. outputdir .. "/%{prj.name}")

	include3DMAX()
	link3DMAX()
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"DeltaFileScan/src",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "DeltaFileScan_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "DeltaFileScan_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "DeltaFileScan_DIST"
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

	include3DMAX()
	link3DMAX()

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
