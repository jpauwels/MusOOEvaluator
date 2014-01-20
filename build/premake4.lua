-- A solution contains projects, and defines the available configurations
solution "MusOOEvaluator"
	configurations {"Release", "Debug"}
	location (os.get() .. '-' .. _ACTION)
	targetdir (os.get() .. '-' .. _ACTION)

	-- A project defines one build target
	project "MusOOEvaluator"
		kind "ConsoleApp"
		language "C++"
		files { 
			"../*.h",
			"../*.cpp",
			"../libMusOO/MusOO/*.h",
			"../libMusOO/MusOO/*.cpp",
			"../libMusOO/MusOOFile/*.h",
			"../libMusOO/MusOOFile/*.cpp",
			"../libMusOO/tinyxml2/tinyxml2.cpp",
		}
		includedirs {
			"..",
			"../libMusOO",
			"../third-party/eigen",
			"$(COMPILED_LIBRARIES_INCLUDE)"
		}
		libdirs {"$(COMPILED_LIBRARIES_LIB)"}

		configuration "Debug"
			defines {"DEBUG"}
			flags {"Symbols"}

		configuration "Release"
			defines {"NDEBUG"}
			flags {"Optimize"}

		configuration "macosx"
			links {"boost_filesystem", "boost_program_options", "boost_system"}
			linkoptions {"-undefined dynamic_lookup"}

		configuration "linux"
			links {"boost_filesystem", "boost_program_options", "boost_system"}
			defines {"_SYS_SYSMACROS_H"}
		 
		configuration "vs*"
			defines {"_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS"}
			targetdir("$(SolutionDir)$(ConfigurationName)")
			objdir("$(ConfigurationName)")
			targetname("$(ProjectName)")
