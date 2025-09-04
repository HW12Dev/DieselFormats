add_requires("zlib v1.3.1")

target("DieselFormats")
  set_kind("static")
  set_languages("cxxlatest")
  set_exceptions("cxx")
  set_symbols("debug")

  add_includedirs("./src")
  add_files("./src/**.cpp")
  add_headerfiles("./src/**.h")
  add_headerfiles("./DieselFormats.natvis")

  add_defines("WIN32_LEAN_AND_MEAN")

  add_packages("zlib")

  set_runtimes(is_mode("debug") and "MDd" or "MD")

  --add_cxxflags("-Wall")

target_end()