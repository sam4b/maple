#pragma once
#include <SFML/Graphics.hpp>

class Tileset {
public:
    Tileset() : texture(nullptr), tileSize(-1), rows(-1), cols(-1), tilesetRatio(-1), setID(-1) {}; //just to make it REALLY obvious when debugging if uninitialised.
    Tileset(sf::Texture* texture, const int tileSize, const int setID) : texture(texture), tileSize(tileSize), setID(setID) {
        rows = texture->getSize().y / tileSize;
        cols = texture->getSize().x / tileSize;
        tilesetRatio = texture->getSize().x / tileSize;
    };

    sf::IntRect idToSubTexture(const int id) const {
        assert(texture);

        const int tu = id % tilesetRatio;
        const int tv = id / tilesetRatio;

        const sf::IntRect tile(tu * tileSize, tv * tileSize, tileSize, tileSize);

        return tile;
    }

    int getTilesetID() const noexcept {
        return setID;
    }

    int tileCount() const {
        return rows * cols;
    }

    const sf::Texture& getTexture() const {
        assert(texture);
        return *texture;
    }
private:
    sf::Texture* texture;
    int tileSize; //Uniform tile size (x by x) is assumed.
    int rows;
    int cols;
    int tilesetRatio;
    int setID;
};

int GetTilesetRatio(const sf::Texture& texture, const int rowLength, const int colLength) {
    assert(rowLength > 0);
    assert(colLength > 0);
    const sf::Vector2i tileSize = { (int)texture.getSize().x / rowLength, (int)texture.getSize().y / colLength };
    const int tileSetRatio = texture.getSize().x / tileSize.x;
    assert(tileSetRatio > 0);

    return tileSetRatio;
}

sf::Vector2i GetTilesetSize(const sf::Texture& texture, const int rowLength, const int colLength) {
    const sf::Vector2i tileSize = { (int)texture.getSize().x / rowLength, (int)texture.getSize().y / colLength };
    return tileSize;
}


sf::IntRect fromTileset(const sf::Texture& texture, const int id, const int rowLength, const int colLength) {
    assert(id <= rowLength * colLength); //bounds check (is it <= or <?)
    const sf::Vector2i tileSize = GetTilesetSize(texture, rowLength, colLength);

    const int tilesetRatio = GetTilesetRatio(texture, rowLength, colLength);

    const int tu = id % tilesetRatio;
    const int tv = id / tilesetRatio;

    const sf::IntRect tile(tu * tileSize.x, tv * tileSize.y, tileSize.x, tileSize.y);

    return tile;
}

Tileset loadTileset(const uint64_t uuid) {
    const std::filesystem::path path("assets/tilesets/" + std::to_string(uuid) + ".json");
    assert(std::filesystem::exists(path));

    std::ifstream in(path);
    assert(in.is_open());

    const nlohmann::json json = nlohmann::json::parse(in);

    assert(true); //For now, no checks - but we will want to reject files with extraneous data (or do we?)

    assert(json.contains("tileSize") && json.at("tileSize").is_number_integer());
    assert(json.contains("path") && json.at("path").is_string());

    const std::filesystem::path pngPath(std::string(json.at("path")));

    assert(pngPath.extension() == ".png");
    assert(std::filesystem::exists(pngPath));

    sf::Texture* texture = new sf::Texture;
    assert(texture->loadFromFile(pngPath.string()));

    const int tileSize = json.at("tileSize");

    const int setID = uuid;

    Tileset set(texture, tileSize, setID);

    return set;
}