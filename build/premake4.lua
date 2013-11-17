-- A solution contains projects, and defines the available configurations
solution "KeyChordEvaluation"
	configurations {"Release", "Debug"}
	location (_ACTION)
	targetdir (_ACTION)

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
			links {"boost_filesystem-xgcc42-mt", "boost_program_options-xgcc42-mt", "boost_system-xgcc42-mt"}
			linkoptions {"-undefined dynamic_lookup"}

		configuration "linux"
			links {"boost_filesystem-gcc41-mt", "boost_program_options-gcc41-mt", "boost_system-gcc41-mt"}
			defines {"_SYS_SYSMACROS_H"}
		 
		configuration "vs*"
			defines {"_CRT_SECURE_NO_WARNINGS"}
			targetdir("$(SolutionDir)$(ConfigurationName)")
			objdir("$(ConfigurationName)")
			targetname("$(ProjectName)")
