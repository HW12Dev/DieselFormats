add_requires("assimp v6.0.2")

target("DieselFormatsAssimpTest")
  set_languages("cxxlatest")
  set_exceptions("cxx")
  set_symbols("debug")

  add_includedirs("./src")
  add_files("./src/**.cpp")
  add_headerfiles("./src/**.h")

  add_defines("WIN32_LEAN_AND_MEAN")

  add_includedirs(path.join(os.projectdir(), "dieselformats/src"))
  add_deps("DieselFormats")

  add_packages("assimp")

  set_runtimes(is_mode("debug") and "MDd" or "MD")
  
target_end()