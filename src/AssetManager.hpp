#pragma once
#include <SFML/Graphics.hpp>
#include <variant>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <format>
#include <imgui.h>
#include <magic_enum.hpp>
#include "vendor/ImGuiFileBrowser.hpp"
#include "Random.hpp"


struct Texture {
	sf::Texture* texture;
	sf::IntRect rect;
};
/*
	Consider ref-counted handle system. Also support for other types of assets + subtextures (tilemaps?).
*/
class AssetManager {
public:
	/*
		Returns an texture if the texture has been imported into Maple.
		This method attempts to load the texture from disk if it has been imported but is not in memory.
	*/
	std::optional<Texture> GetTexture(const uint64_t id) {
		if (subTextures.contains(id)) {
			const TextureData& textureData = subTextures.at(id);

			assert(textures.contains(textureData.parent)); //parent must be loaded

			assert(textureData.texture.texture); //Non-null.

			return { textureData.texture };
		}

		const AssetProperties properties = nameToUUID.at((UUIDtoName.at(id)));

		assert(properties.type == AssetProperties::Type::SubTexture);

		const uint64_t parentID = properties.extraneous.subTextureData.parentUUID;

		if (!textures.contains(parentID)) {
			const bool success = textures[parentID].loadFromFile(std::format("{}.png", parentID));

			assert(success);
		}

		TextureData data;
		data.parent = parentID;
		data.texture.rect = properties.extraneous.subTextureData.rect;
		data.texture.texture = &textures.at(parentID);

		subTextures[id] = data;

		return subTextures[id].texture;
	}

	std::optional<Texture> GetTexture(const std::string& path) {
		assert(nameToUUID.contains(path));
		return GetTexture(nameToUUID.at(path).uuid);
	}

	//Must be called after loading the registry. It is ill formed if not.
	void LoadSceneAssets(const nlohmann::json& assetData) {
		for (const auto& id : assetData) {
			assert(UUIDtoName.contains(id));

			const std::string& name = UUIDtoName.at(id);
			
			assert(nameToUUID.contains(name));

			const AssetProperties properties = nameToUUID.at(name);

			assert(id == properties.uuid);

			if (properties.type == AssetProperties::Type::Texture) {
				assert(!textures.contains(properties.uuid));

				assert(textures[properties.uuid].loadFromFile(std::format("assets/textures/{}.png", uint64_t(id)))); //Note: stick a std::string in the union as name may not map to file?
				//Potentially move files.
			}
			else if (properties.type == AssetProperties::Type::SubTexture) {
				const AssetManager::SubTextureMetadata subData = properties.extraneous.subTextureData;
				const auto parent = subData.parentUUID;

				if (!textures.contains(parent)) {
					assert(textures[parent].loadFromFile(std::format("assets/textures/{}.png", parent)));
				}

				TextureData data;
				data.parent = parent;
				data.texture.rect = subData.rect;
				data.texture.texture = &textures[parent];
				subTextures[uint64_t(id)] = data;
			}
			else {
				assert(false);
			}
		}
	}

	void LoadRegistry(const nlohmann::json& assetData) {
		if (assetData["assets"].empty()) return;

		for (const auto& jsonObj : assetData["assets"]) {
			const std::string name = jsonObj["name"];
			const AssetProperties properties = fromJson(jsonObj);

			nameToUUID.insert({ name, properties });
			UUIDtoName.insert({ properties.uuid, name });
		}
	}


	[[nodiscard]] nlohmann::json SaveRegistry() const noexcept {
		nlohmann::json json;
		for (const auto& [name, properties] : nameToUUID) {
			nlohmann::json j = toJson(properties);
			j["name"] = name;
			json["assets"].push_back(j);

		}
		return json;
	}

	/*
		These two methods have zero to do with runtime loading of images. This is designed to register an asset as existing, and allow for runtime loading.
		For runtime loading, use AssetManager::GetTexture(uint64_t) or AssetManager::GetTexture(std::string).
	*/

	//Precondition: path exists and is a path to a PNG. 
	void ImportTexture(const std::filesystem::path& path, const std::string& name) {
		//Validate
		assert(path.extension() == ".png");
		assert(std::filesystem::exists(path));
		const bool valid = sf::Texture().loadFromFile(path.string());
		assert(valid);


		//Create
		const auto uuid = generateUUID();

		AssetProperties properties;
		properties.type = AssetProperties::Type::Texture;
		properties.uuid = uuid;

		{
			assert(std::filesystem::exists(std::filesystem::current_path() / std::filesystem::path("assets/"))); //Checks that we have an asset folder. Todo: Automate asset folder creation.
			const std::filesystem::path newPath = std::format("{}/assets/textures/{}.png", std::filesystem::current_path().string(), uuid);

			std::filesystem::copy(path, newPath); //Can fail (throws).
		}


		assert(!UUIDtoName.contains(uuid));
		assert(!nameToUUID.contains(name));

		nameToUUID[name] = properties;
		UUIDtoName[uuid] = name;
	}

	//Adds this to the registry.
	//Note: doesn't load it.
	void ImportSubTexture(const uint64_t parentID, const std::string name, const sf::IntRect subset) {
		
		//Validate parentUUID exists
		const AssetProperties parentProperties = nameToUUID.at(UUIDtoName.at(parentID));

		assert(parentProperties.type == AssetProperties::Type::Texture);
		 
		//assert(textures.contains(parentID)); Actually, doesn't have to be loaded in-memory at the time of registration. 

		const SubTextureMetadata extraneous(parentID, subset);
		const uint64_t uuid = generateUUID();

		AssetProperties properties;
		properties.extraneous.subTextureData = extraneous;
		properties.uuid = uuid;
		properties.type = AssetProperties::Type::SubTexture;

		assert(!UUIDtoName.contains(uuid));
		assert(!nameToUUID.contains(name));

		UUIDtoName[uuid] = name;
		nameToUUID[name] = properties;
	}

	void openTextureWindow() {
		//static struct 
	}

	void ImGuiDisplay() {
		ImGui::Begin("Asset Registry");

		ImGui::Text("Textures");
		for (const auto& [uuid, texture] : textures) {
			ImGui::Text(UUIDtoName.at(uuid).c_str());
			ImGui::Image(texture, sf::Vector2f{ 64, 64 });
		}

		ImGui::Text("Subtextures");

		for (const auto& [uuid, subTexture] : subTextures) {
			ImGui::Text(UUIDtoName.at(uuid).c_str());
			sf::Sprite sprite;
			assert(textures.contains(subTexture.parent));
			sprite.setTextureRect(subTexture.texture.rect);
			sprite.setTexture(*subTexture.texture.texture);
			ImGui::Image(sprite, sf::Vector2f{ 64,64 });
		}

		ImGui::End();

		constexpr int size = 1;

		const char* arr[size] = { "Texture" };
		const AssetProperties::Type types[size] = { AssetProperties::Type::Texture };

		static_assert(sizeof(types) / sizeof(AssetProperties::Type) == sizeof(arr) / sizeof(char*), "Combo arrays must remain the same size!");

		static struct AssetGUIData {
			int selectedType = 0; //Index into both arrays.
			bool beginImport = false; //Used to determine if should open a new window.
		} data;

		ImGui::Begin("Assets");

		ImGui::Combo("Asset Type", &data.selectedType, arr, size);
		if (ImGui::Button("Import new asset")) {
			assert(data.selectedType >= 0 && data.selectedType < size);
			data.beginImport = true;

			switch (types[data.selectedType]) {
				using enum AssetProperties::Type;
			case Texture:
				ImGui::OpenPopup("TextureImport");
				break;
			default:
				assert(false);
			}
		}
		if (data.beginImport) {
			if (ImGui::BeginPopupModal("TextureImport")) {
				static struct TextureImportData {
					TextureImportData() {
						imported = false;
						fileDialog.SetTypeFilters({ ".png" });
						fileDialog.SetTitle("Texture Import");
					}
					std::string name;
					std::filesystem::path path;
					bool imported;
					sf::Texture t;
					ImGui::FileBrowser fileDialog;
				} importData;

				static struct SpritesheetData {
					SpritesheetData() {
						isSpritesheet = false;
						rows = 0;
						cols = 0;
						tilesetRatio = 0;
						tileSize = 0;
					}
					bool isSpritesheet;
					int rows;
					int cols;
					int tilesetRatio;
					int tileSize;
				} spriteSheetData;


				static sf::RenderTexture texture;

				texture.clear();
				
				if (importData.imported) {
					sf::Sprite s;
					s.setTexture(importData.t);
					texture.draw(s);
				}

				if (importData.imported) {
					ImGui::InputText("Name", &importData.name);
					ImGui::Checkbox("Spritesheet", &spriteSheetData.isSpritesheet);

					if (spriteSheetData.isSpritesheet) {

						ImGui::InputInt("Tile size (uniform)", &spriteSheetData.tileSize);
						if (spriteSheetData.tileSize > 0 && importData.imported) { //Safe to recalculate
							static std::vector<sf::Vertex> gridLine;
							spriteSheetData.rows = importData.t.getSize().y / spriteSheetData.tileSize;
							spriteSheetData.cols = importData.t.getSize().x / spriteSheetData.tileSize;
							spriteSheetData.tilesetRatio = importData.t.getSize().x / spriteSheetData.tileSize;

							//Allocatinng each frame :skull:, though we can probably do this conditionally.
							gridLine.clear();
							gridLine.reserve((importData.t.getSize().x / spriteSheetData.tileSize) + (importData.t.getSize().y / spriteSheetData.tileSize));


							for (int row = 0; row < importData.t.getSize().y / spriteSheetData.tileSize; row++) { //Deal w/ tile size
								const auto begin = sf::Vertex(sf::Vector2f(0, row * spriteSheetData.tileSize));
								const auto end = sf::Vertex(sf::Vector2f(importData.t.getSize().x, row * spriteSheetData.tileSize));

								gridLine.push_back(begin);
								gridLine.push_back(end);
							}

							for (int col = 0; col < importData.t.getSize().x / spriteSheetData.tileSize; col++) {
								const auto begin = sf::Vertex(sf::Vector2f(col * spriteSheetData.tileSize, 0));
								const auto end = sf::Vertex(sf::Vector2f(col * spriteSheetData.tileSize, importData.t.getSize().y));

								gridLine.push_back(begin);
								gridLine.push_back(end);
							}

							texture.draw(gridLine.data(), gridLine.size(), sf::Lines);
						}
					}
					texture.display();
					ImGui::Image(texture);

					if (ImGui::Button("Import")) {
						if (importData.imported) {
							this->ImportTexture(importData.path, importData.name);
							std::cout << "Imported texture!\n";
							 
							if (spriteSheetData.isSpritesheet) {
								const auto UUID = nameToUUID.at(importData.name).uuid;

								for (int id = 0; id < spriteSheetData.rows * spriteSheetData.cols; id++) {
									const std::string name = std::format("{}_{}", importData.name, id);

									const int tu = id % spriteSheetData.tilesetRatio;
									const int tv = id / spriteSheetData.tilesetRatio;

									const sf::IntRect rect(tu* spriteSheetData.tileSize, tv* spriteSheetData.tileSize, spriteSheetData.tileSize, spriteSheetData.tileSize);

									this->ImportSubTexture(UUID, name, rect);
								}

							}

							data.beginImport = false;
						}
					};

					if (ImGui::Button("Cancel")) {
						importData = TextureImportData();
						data.beginImport = false;
					}
				}
				else {
					if (!importData.fileDialog.IsOpened()) {
						importData.fileDialog.Open();
					}
					if (importData.fileDialog.HasSelected()) {
						const std::filesystem::path path(importData.fileDialog.GetSelected());

						importData.fileDialog.ClearSelected();
						importData.fileDialog.Close();

						importData.imported = true;

						assert(std::filesystem::exists(path));
						assert(path.extension() == ".png");

						importData.path = path;
						assert(importData.t.loadFromFile(path.string()));

						texture.create(importData.t.getSize().x, importData.t.getSize().y); //Is this a memory leak, surely it frees the old texture.
					}
				}
				importData.fileDialog.Display();

				ImGui::EndPopup();
			}
			else if (ImGui::BeginPopupModal("SubTextureImport")) {
				static struct SubTextureImportData {
					SubTextureImportData() {
						selected = false;
						selectedTexture = 0;
					}
					bool selected;
					uint64_t selectedTexture;
				} importData;

				if (!importData.selected) {
					for (const auto& [uuid, texture] : textures) {
						ImGui::PushID(uuid); //Silent narrowing conversion :skull:
						ImGui::Text(UUIDtoName.at(uuid).c_str());
						if (ImGui::ImageButton(texture, sf::Vector2f{ 64, 64 })) {
							importData.selected = true;
							importData.selectedTexture = uuid;
						}
						ImGui::PopID();
					}
				}
				else {
					ImGui::Text(std::format("Selected {}", importData.selected).c_str());
				}
				ImGui::EndPopup();
			}
			else {
				assert(false);
			}
		}

		ImGui::End();

	}
private:
	struct SubTextureMetadata {
		SubTextureMetadata(const uint64_t parentID, const sf::IntRect subset) : parentUUID(parentID), rect(subset) {};
		uint64_t parentUUID;
		sf::IntRect rect;
	};

	struct AssetProperties {
		enum class Type {
			Texture,
			SubTexture
		} type;

		uint64_t uuid;

		union AssetData {
			AssetData() {
				memset(this, 0, sizeof(AssetData));
			}
			SubTextureMetadata subTextureData;
		} extraneous; //Type implies union data
	};


	//Converters

	[[nodiscard]] AssetProperties fromJson(const nlohmann::json& jsonObj) const noexcept {
		const uint64_t id = jsonObj["id"];

		const AssetProperties::Type type = [](const std::string& s) -> AssetProperties::Type {
			if (s == "Texture") {
				return AssetProperties::Type::Texture;
			}
			else if (s == "SubTexture") {
				return AssetProperties::Type::SubTexture;
			}
			else {
				assert(false);
			}

			}(jsonObj["type"]);

		union AssetProperties::AssetData extraneous;

		switch (type) {
			using enum AssetProperties::Type;
		case Texture:
			break;
		case SubTexture:
			extraneous.subTextureData.parentUUID = jsonObj["extraneous"]["parentUUID"];
			extraneous.subTextureData.rect = sf::IntRect{ jsonObj["extraneous"]["rect"][0], jsonObj["extraneous"]["rect"][1], jsonObj["extraneous"]["rect"][2], jsonObj["extraneous"]["rect"][3] };
			break;
		default:
			assert(false);
			break;
		}

		AssetProperties properties;
		properties.type = type;
		properties.uuid = id;
		properties.extraneous = extraneous;

		return properties;
	}

	[[nodiscard]]  nlohmann::json toJson(const AssetProperties& properties) const noexcept {
		nlohmann::json out;

		out["id"] = properties.uuid;
		out["type"] = [&]() -> std::string { switch (properties.type) {
			using enum AssetProperties::Type;

		case Texture:
			return "Texture";
		case SubTexture:
			return "SubTexture";
		default:
			assert(false);
		} } ();

		switch (properties.type) {
			using enum AssetProperties::Type;

		case Texture:
			break;
		case SubTexture:
			out["extraneous"]["parentUUID"] = properties.extraneous.subTextureData.parentUUID;
			out["extraneous"]["rect"] = { properties.extraneous.subTextureData.rect.getPosition().x, properties.extraneous.subTextureData.rect.getPosition().y,
			properties.extraneous.subTextureData.rect.getSize().x, properties.extraneous.subTextureData.rect.getSize().y };
		}

		return out;
	}

	struct TextureData {
		Texture texture;
		uint64_t parent;
	};

	std::unordered_map<uint64_t, TextureData> subTextures;
	std::unordered_map<uint64_t, sf::Texture> textures;



	std::unordered_map<std::string, AssetProperties> nameToUUID;
	std::unordered_map<uint64_t, std::string> UUIDtoName;
};