-- A solution contains projects, and defines the available configurations
solution "MusOOEvaluator"
	configurations {"Release", "Debug"}
	if (_ACTION == nil) then
		return
	end
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

		configuration "Debug"
			defines {"DEBUG"}
			flags {"Symbols"}

		configuration "Release"
			defines {"NDEBUG"}
			flags {"Optimize"}

		configuration {"macosx", "gmake"}
			linkoptions {
				"$(shell test ${COMPILED_LIBRARIES_LIB} && echo ${COMPILED_LIBRARIES_LIB} || echo $(shell brew --prefix)/lib)/libboost_filesystem.a",
				"$(shell test ${COMPILED_LIBRARIES_LIB} && echo ${COMPILED_LIBRARIES_LIB} || echo $(shell brew --prefix)/lib)/libboost_program_options.a",
				"$(shell test ${COMPILED_LIBRARIES_LIB} && echo ${COMPILED_LIBRARIES_LIB} || echo $(shell brew --prefix)/lib)/libboost_system.a"
			}
			
		configuration {"macosx", "xcode3"}
			linkoptions {"${COMPILED_LIBRARIES_LIB}/libboost_filesystem.a", "${COMPILED_LIBRARIES_LIB}/libboost_program_options.a", "${COMPILED_LIBRARIES_LIB}/libboost_system.a"}

		configuration "linux"
			links {"boost_filesystem", "boost_program_options", "boost_system"}
			linkoptions {" -static $(shell test ${COMPILED_LIBRARIES_LIB} && echo -L${COMPILED_LIBRARIES_LIB})"}
			defines {"_SYS_SYSMACROS_H"}
		 
		configuration "vs*"
			defines {"_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS"}
			libdirs {"$(COMPILED_LIBRARIES_LIB)"}
