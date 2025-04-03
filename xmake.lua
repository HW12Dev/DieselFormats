add_rules("mode.release", "mode.debug")


add_requires("qt6widgets")

includes("dieselformats")

includes("tools/lookuptool")
includes("tools/xmbparser")
includes("tools/banditsdecryptor")

target("DieselFormatsPlayground")
  set_languages("cxxlatest")
  set_exceptions("cxx")
  set_symbols("debug")

  add_includedirs("./tools/dieselformatsplayground/src")
  add_files("./tools/dieselformatsplayground/src/**.cpp")
  add_headerfiles("./tools/dieselformatsplayground/src/**.h")

  add_defines("WIN32_LEAN_AND_MEAN")

  add_includedirs("./dieselformats/src")
  add_deps("DieselFormats")
  set_runtimes(is_mode("debug") and "MDd" or "MD")
target_end()