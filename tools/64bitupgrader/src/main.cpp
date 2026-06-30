#include <diesel/modern/scriptdata.h>
#include <iostream>

bool is_32bit_scriptdata(const std::filesystem::path& path)
{
    Reader reader(path);

    uint64_t numbers_allocator = reader.ReadType<uint64_t>();
    reader.AddPosition(24);
    uint64_t strings_allocator = reader.ReadType<uint64_t>();
    reader.AddPosition(24);
    uint64_t vector3s_allocator = reader.ReadType<uint64_t>();
    reader.AddPosition(24);
    uint64_t quaternions_allocator = reader.ReadType<uint64_t>();
    reader.AddPosition(24);
    uint64_t idstrings_allocator = reader.ReadType<uint64_t>();
    reader.AddPosition(24);
    uint64_t tables_allocator = reader.ReadType<uint64_t>();
    reader.AddPosition(24);


    return !(numbers_allocator == 0 && strings_allocator == 0 && vector3s_allocator == 0 &&
             quaternions_allocator == 0 && idstrings_allocator == 0 && tables_allocator == 0);
}

void upgrade_file(const std::filesystem::path& path)
{
    Reader reader(path);

    diesel::modern::ScriptData sd;

    if(!sd.Read(reader, diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LATEST, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::WINDOWS_32)))
    {
        std::cout << "Error occurred while reading 32bit ScriptData, is the file corrupt?" << std::endl;
        return;
    }

    reader.Close();
    Writer writer(path);
    sd.Write(writer, diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::DIESEL_V3, diesel::Renderer::UNSPECIFIED, diesel::FileSourcePlatform::WINDOWS_64));

    writer.Close();
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, ".utf-8");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::locale::global(std::locale(".utf-8"));
    std::cin.imbue(std::locale());
    std::cout.imbue(std::locale());

    if (argc == 1)
    {
        std::cout << "Please execute this tool with a file provided on the command line" << std::endl;
        return 1;
    }

    std::filesystem::path path = argv[1];

    if (!is_32bit_scriptdata(path))
    {
        std::cout << "Input file " << path << " is already 64bit ScriptData" << std::endl;
        return 1;
    }

    std::cout << "Upgrading " << path << " from 32bit to 64bit" << std::endl;

    upgrade_file(path);

    return 0;
}