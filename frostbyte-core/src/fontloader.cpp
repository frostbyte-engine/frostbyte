#include "fontloader.hpp"

#include "libraries/filesystemlib.hpp"

#include "common.hpp"

namespace frostbyte {

lua_State* FontLoader::L = nullptr;
size_t FontLoader::font_count = 5;
std::vector<Font*> FontLoader::font_list = {};
std::unordered_map<std::string, size_t> FontLoader::hash_font_map;
std::vector<std::string> FontLoader::font_name_list;

std::unordered_map<std::string, Font*> FontLoader::engine_font_map;

#ifdef FROSTBYTE_HEADLESS
void FontLoader::load() { }
void FontLoader::unload() { }

size_t FontLoader::getFont(unsigned char *data, int data_size) {
    return 0;
}
#else
void FontLoader::load() {
    // NOTE: L will not be initialized yet

    font_list.reserve(font_count);
    font_name_list.reserve(font_count);

    Font font_default = GetFontDefault();

    std::string tmp_file_path;

    #define getFont(varname, path)                                                 \
        tmp_file_path.assign(FileSystem::assets_path);                             \
        tmp_file_path.append("fonts/" path);                                       \
        Font varname = LoadFontEx(tmp_file_path.c_str(), 256, NULL, 0);            \
        if (!IsFontValid(varname))                                                 \
            throw std::runtime_error("failed to load font " + std::string(path));

    getFont(font_ui, "Segoe UI.ttf")
    getFont(font_proggy, "ProggyClean.ttf")
    getFont(font_plex, "IBMPlexSans.ttf")
    getFont(font_monospace, "SometypeMono.ttf")

    #undef getFont

    font_list.push_back(new Font(font_default));
    font_list.push_back(new Font(font_ui));
    font_list.push_back(new Font(font_proggy));
    font_list.push_back(new Font(font_plex));
    font_list.push_back(new Font(font_monospace));

    font_name_list.push_back("Default");
    font_name_list.push_back("UI");
    font_name_list.push_back("System");
    font_name_list.push_back("Plex");
    font_name_list.push_back("Monospace");

    std::string directory = FileSystem::assets_path;
    directory.append("rbxasset/fonts");
    for (const auto& file : std::filesystem::directory_iterator(directory)) {
        tmp_file_path.assign(file.path().string());

        if (tmp_file_path.size() < 10)
            continue;

        std::string extension = tmp_file_path.substr(tmp_file_path.size() - 3, 3);
        if (extension != "ttf" && extension != "otf")
            continue;

        Font* font = new Font(LoadFontEx(tmp_file_path.c_str(), 256, NULL, 0));

        char* name = tmp_file_path.data();
        name += directory.size() + 1;
        name[tmp_file_path.size() - directory.size() - 5] = 0; 

        engine_font_map[name] = font;
        font_list.push_back(font);
        font_name_list.push_back(name);
        font_count++;
    }

    directory = FileSystem::assets_path;
    directory.append("rbxasset/fonts/families");
    for (const auto& file : std::filesystem::directory_iterator(directory)) {
        tmp_file_path.assign(file.path().string());

        if (tmp_file_path.size() < 10)
            continue;

        std::string extension = tmp_file_path.substr(tmp_file_path.size() - 4, 4);
        if (extension != "json")
            continue;

        std::string content = readFileToString(tmp_file_path.c_str());
        json family_json = json::parse(content);

        char* name = tmp_file_path.data();
        name += directory.size() + 1;
        name[tmp_file_path.size() - directory.size() - 6] = 0; 

        for (auto& face : family_json["faces"]) {
            std::string face_name = face["name"].template get<std::string>();
            std::string id_string = face["assetId"].template get<std::string>();

            if (id_string.substr(0, 10) != "rbxassetid")
                continue;

            tmp_file_path.assign(FileSystem::assets_path);
            tmp_file_path.append("rbxassetid/").append(id_string.data() + 13);

            Font* font = nullptr;
            try {
                std::string data = readFileToString(tmp_file_path.c_str());
                const char* extension = getFontType(reinterpret_cast<unsigned char*>(data.data()), data.size());
                font = new Font(LoadFontFromMemory(extension, reinterpret_cast<const unsigned char*>(data.data()), data.size(), 256, NULL, 0));
            } catch(std::exception&) { }

            if (!(font && IsFontValid(*font)))
                continue;

            face_name.insert(0, "-");
            face_name.insert(0, name);

            engine_font_map[face_name] = font;
            font_list.push_back(font);
            font_name_list.push_back(face_name);
            font_count++;
        }
    }
}
void FontLoader::unload() {
    for (unsigned int i = 0; i < font_list.size(); i++) {
        UnloadFont(*font_list[i]);
        delete font_list[i];
    }
    font_list.clear();
}


static bool checkSignatureTTF(const unsigned char* data, int data_size) {
    if (data_size < 4)
        return false;

    return data[0] == 0x00 && data[1] == 0x01 &&
        data[2] == 0x00 && data[3] == 0x00;
}

static bool checkSignatureOTF(const unsigned char* data, int data_size) {
    if (data_size < 4)
        return false;

    return data[0] == 'O' && data[1] == 'T' &&
        data[2] == 'T' && data[3] == 'O';
}

const char* FontLoader::getFontType(unsigned char* data, int data_size) {
    if (checkSignatureTTF(data, data_size))
        return ".ttf";
    if (checkSignatureOTF(data, data_size))
        return ".otf";
    return nullptr;
}

size_t FontLoader::getFont(unsigned char *data, int data_size) {
    const char* file_extension = getFontType(data, data_size);
    assert(file_extension);

    std::string hashed = sha1ToString(ComputeSHA1(data, data_size));

    auto cached = hash_font_map.find(hashed);
    if (cached != hash_font_map.end())
        return cached->second;

    size_t index = font_count++;

    Font* font = new Font(LoadFontFromMemory(file_extension, data, data_size, 256, nullptr, 0));

    font_list.push_back(font);
    hash_font_map[hashed] = index;
    font_name_list.push_back(hashed);

    lua_getglobal(L, "Drawing");
    lua_getfield(L, -1, "Fonts");
    lua_pushunsigned(L, index);
    lua_setfield(L, -2, hashed.c_str());

    return index;
}
#endif

}; // namespace fakerobox
