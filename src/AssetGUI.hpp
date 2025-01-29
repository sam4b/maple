#pragma once
#include "AssetManager.hpp"
#include <iostream>
#include "vendor/imgui-filebrowser/imfilebrowser.h"
#include <imgui.h>
#include <imgui-SFML.h>

bool TextureImport(AssetManager& manager, AssetRegistry& registry, const std::filesystem::path& projectRoot) {
	static struct TextureImportData {
		TextureImportData() {
			imported = false;
			fileDialog.SetTypeFilters({ ".png" });
			fileDialog.SetTitle("Texture Import");
			isSpritesheet = false;
		}
		std::string name;
		std::filesystem::path path;
		bool imported;
		bool isSpritesheet;
		sf::Texture t;
		ImGui::FileBrowser fileDialog;
	} importData;

	static SpritesheetData spriteSheetData;


	static sf::RenderTexture texture;

	const bool imageLoaded = importData.imported;

	texture.clear();

	 
	if (importData.imported) { ///Display the image etc etc
		sf::Sprite s;
		s.setTexture(importData.t);
		texture.draw(s);
		ImGui::InputText("Name", &importData.name);
		ImGui::Checkbox("Spritesheet", &importData.isSpritesheet);

		if (importData.isSpritesheet) {
			ImGui::InputInt("Tile size (uniform)", &spriteSheetData.tileSize);
			if (spriteSheetData.tileSize > 0 && importData.imported) { //Safe to recalculate
				static std::vector<sf::Vertex> gridLine;
				gridLine.reserve((importData.t.getSize().x / spriteSheetData.tileSize) + (importData.t.getSize().y / spriteSheetData.tileSize));

				createGrid(spriteSheetData.tileSize, importData.t.getSize(), gridLine); //Allocatinng each frame :skull:, though we can probably do this conditionally.

				texture.draw(gridLine.data(), gridLine.size(), sf::Lines);
			}
		}
		texture.display();
		ImGui::Image(texture);

		if (ImGui::Button("Import")) {
			if (importData.imported) {
				manager.ImportTexture(importData.path, importData.name, projectRoot);
				std::cout << "Imported texture!\n";

				if (importData.isSpritesheet) {
					const auto UUID = registry.getProperties(importData.name).uuid;

					manager.ImportSubTextures(UUID, spriteSheetData, projectRoot);
				}

				return false;
			}
		}

		if (ImGui::Button("Cancel")) {
			return false;
		}
	}
	else { //Open a file dialog.
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

	return true;
}

void AssetWindow(AssetManager& manager, AssetRegistry& registry, const std::filesystem::path& projectRoot) {
	constexpr int size = 2;

	const char* arr[size] = { "Texture/Spritesheet", "Animation"};
	const AssetProperties::Type types[size] = { AssetProperties::Type::Texture, AssetProperties::Type::Animation };

	static_assert(size > 0);
	static_assert(sizeof(types) / sizeof(AssetProperties::Type) == sizeof(arr) / sizeof(char*), "Combo arrays must remain the same size!");

	static struct AssetGUIData {
		int selectedType = 0; //Index into both arrays.
		bool beginImport = false; //Used to determine if should open a new window.
	} assetType;


	static bool showAssetTypeScreen = !assetType.beginImport;

	static const char* textureString = "TextureImport";
	ImGui::Begin("Assets");
	if (showAssetTypeScreen) {
		ImGui::Combo("Asset Type", &assetType.selectedType, arr, size);
		if (ImGui::Button("Import asset")) {
			assert(assetType.selectedType >= 0 && assetType.selectedType < size);

			const auto type = types[assetType.selectedType];

			switch (type) {
				using enum AssetProperties::Type;

			case Texture:
				ImGui::OpenPopup(textureString);
				break;
			case Spritesheet:
				ImGui::OpenPopup("SpritesheetImport");
				break;
			case Animation:
				ImGui::OpenPopup("AnimationImport");
				break;
			default:
				assert(false);
				break;
			}
			showAssetTypeScreen = false;
		}
	}
	else {
		if (ImGui::BeginPopupModal(textureString)) {
			const bool continuing = TextureImport(manager, registry, projectRoot);
			if (!continuing) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();

		}
	}
	ImGui::End();
}

/*
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

		static struct AnimationCreatorData {
			bool selected;
			uint64_t textureID;

			AnimationCreatorData() {
				selected = false;
				textureID = 0;
			}

		} animData;

		if (!animData.selected) {
			ImGui::Begin("Animations");
			for (const auto& [id, texture] : textures) {
				const auto& name = UUIDtoName.at(id);
				assert(nameToUUID.at(name).type == AssetProperties::Type::Texture);

				sf::Sprite sprite;
				sprite.setTexture(texture);

				ImGui::PushID(id); //narrowing
				ImGui::Text(std::format("{}", name).c_str());
				ImGui::Image(sprite, { 128, 128 });
				if (ImGui::Button("Select")) {
					if (animData.selected == false) { //Don't overwrite, though it should be impossible for
						//us to be here if so.
						animData.selected = true;
						animData.textureID = id;
					}
				}
				ImGui::PopID();



			}
			ImGui::End();
		}
		else {
			static struct AnimationData {
				std::string name;
				std::vector<uint64_t> ids; //Could optimise to be a smaller index into the same texture and calculate on fly?
			} data;

			ImGui::Begin("Animation Creator");
			ImGui::InputText("Name", &data.name);



			if (!data.ids.empty()) {
				ImGui::Text("Frames");
				for (const auto id : data.ids) {
					sf::Sprite sprite;
					const auto tex = this->GetTexture(id);
					sprite.setTexture(*tex.value().texture);
					sprite.setTextureRect(tex.value().rect);
					ImGui::Image(sprite, { 64, 64 });
					ImGui::SameLine();
				}

				ImGui::NewLine();
			}

			const sf::Texture& tex = textures.at(animData.textureID);

#define FUCK 16
			const int columns = tex.getSize().x / FUCK;
			const int rows = tex.getSize().y / FUCK;

			const int numIDS = columns * rows;

			for (int i = 0; i < numIDS; i++) {
				sf::Sprite sprite;
				const std::string name = std::format("{}_{}", UUIDtoName.at(animData.textureID), i);
				sprite.setTexture(tex);
				sprite.setTextureRect(this->GetTexture(name).value().rect);
				ImGui::PushID(i);
				if (ImGui::ImageButton(sprite, { 64, 64 })) {
					data.ids.push_back(nameToUUID.at(name).uuid);
				}
				if (i % 16 != 0 || i == 0) {
					ImGui::SameLine();
				}
				ImGui::PopID();
			}

			if (ImGui::Button("Create")) {
				const uint64_t id = generateUUID();

				if (data.ids.empty()) {
					assert(false);
				}



			}

			if (ImGui::Button("Cancel")) {
				animData.selected = false;
			}
			ImGui::End();
		}



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
							gridLine.reserve((importData.t.getSize().x / spriteSheetData.tileSize) + (importData.t.getSize().y / spriteSheetData.tileSize));

							createGrid(spriteSheetData.tileSize, importData.t.getSize(), gridLine); //Allocatinng each frame :skull:, though we can probably do this conditionally.

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

	}*/