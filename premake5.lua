workspace "Choreograph"
  configurations { "Release", "Debug" }
  location "build/"

  startproject "Project"

project "Choreograph"

  kind "StaticLib"

  cppdialect "C++11"

  files {
      "src/choreograph/**.hpp",
      "src/choreograph/**.h",
      "src/choreograph/**.cpp"
  }

  filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"
	targetsuffix "-d"

  filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"

	filter "platforms:x86"
	   architecture "x86"

	filter "platforms:x64"
     architecture "x86_64"
	
	filter {}

	includedirs { "./src/" }
