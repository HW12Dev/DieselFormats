
target("DieselLookupTool")
  set_languages("cxxlatest")
  set_exceptions("cxx")
  set_symbols("debug")

  add_includedirs("./src")
  add_files("./src/**.cpp")
  add_headerfiles("./src/**.h")

  add_defines("WIN32_LEAN_AND_MEAN")

  add_includedirs(path.join(os.projectdir(), "dieselformats/src"))
  add_deps("DieselFormats")

  add_rules("win.sdk.application")

  add_packages("qt6widgets")
  add_cxxflags("-Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -permissive- -Zc:__cplusplus -Zc:externConstexpr -utf-8 -w34100 -w34189 -w44996 -w44456 -w44457 -w44458")
  set_runtimes(is_mode("debug") and "MDd" or "MD")

  
  after_build(function()
    os.cp("$(scriptdir)/icon-white.png", "$(buildir)/windows/" .. os.arch() .. "/" .. (is_mode("debug") and "debug" or "release") .. "/icon-white.png")
  end)
target_end()
