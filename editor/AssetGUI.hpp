#pragma once
#include "AssetManager.hpp"
#include <iostream>
#include <imgui.h>
#include <imfilebrowser.h>
#include <imgui-SFML.h>

bool AnimationImport(AssetManager& manager, AssetRegistry& registry) {

	static struct AnimationCreatorData {
		bool selected;
		uint64_t textureID;

		AnimationCreatorData() {
			selected = false;
			textureID = 0;
		}

	} animData;

	if (!animData.selected) {
		int textureCount = 0;
		for (const auto& [id, texture] : manager.textures) {
			const auto& name = registry.getName(id);
			const auto properties = registry.getProperties(name);

			if (properties.type != AssetProperties::Type::Spritesheet) continue;

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
			textureCount++;
			ImGui::PopID();
		}
	
		if (textureCount == 0) {
			std::cout << "No spritesheets loaded!\n";
			return false;
		}
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
				const auto tex = manager.GetTexture(id);
				sprite.setTexture(*tex.value().texture);
				sprite.setTextureRect(tex.value().rect);
				ImGui::Image(sprite, { 64, 64 });
				ImGui::SameLine();
			}

			ImGui::NewLine();
		}

		const auto& properties = registry.getProperties(animData.textureID);
		assert(properties.type == AssetProperties::Type::Spritesheet);
		const auto& spriteSheetData = properties.extraneous.spriteSheetData;
		const sf::Texture& tex = manager.textures.at(animData.textureID);
		const int columns = tex.getSize().x / spriteSheetData.tileSize;
		const int rows = tex.getSize().y / spriteSheetData.tileSize;

		const int numIDS = columns * rows;

		for (int i = 0; i < numIDS; i++) {
			sf::Sprite sprite;
			const std::string name = std::format("{}_{}", registry.getName(animData.textureID), i);
			const AssetProperties properties = registry.getProperties(name);
			const auto uuid = properties.uuid;
			sprite.setTexture(tex);
			sprite.setTextureRect(manager.GetTexture(name).value().rect);
			ImGui::PushID(i);
			if (ImGui::ImageButton(sprite, { 64, 64 })) {
				data.ids.push_back(uuid);
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
			ImGui::End();
			return false;

		}

		if (ImGui::Button("Cancel")) {
			animData.selected = false;
			ImGui::End();
			return false;
		}
		ImGui::End();
	}
	return true;
}

bool SpritesheetImport(AssetManager& manager, AssetRegistry& registry) {
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

	static SpritesheetData spriteSheetData;


	static sf::RenderTexture texture;

	const bool imageLoaded = importData.imported;

	texture.clear();

	if (importData.imported) { ///Display the image etc etc
		sf::Sprite s;
		s.setTexture(importData.t);
		texture.draw(s);
		ImGui::InputText("Name", &importData.name);
		ImGui::InputInt("Tile size (uniform)", &spriteSheetData.tileSize);
		if (spriteSheetData.tileSize > 0 && importData.imported) { //Safe to recalculate
			static std::vector<sf::Vertex> gridLine;
			gridLine.reserve((importData.t.getSize().x / spriteSheetData.tileSize) + (importData.t.getSize().y / spriteSheetData.tileSize));

			createGrid(spriteSheetData.tileSize, importData.t.getSize(), gridLine); //Allocatinng each frame :skull:, though we can probably do this conditionally.

			texture.draw(gridLine.data(), gridLine.size(), sf::Lines);
		}
	
		texture.display();
		ImGui::Image(texture);

		if (ImGui::Button("Import")) {
			if (importData.imported) {
				manager.ImportSpritesheet(importData.path, importData.name, spriteSheetData);
				return false;
			}
		}

		if (ImGui::Button("Cancel")) {
			return false;
		}
	}
	else { //Open a file dialog.
		static bool hasBeenOpened = false;
		if (!importData.fileDialog.IsOpened()) {
			importData.fileDialog.Open();
			if (hasBeenOpened) {
				hasBeenOpened = false;
				return false; //Exit
			}
			hasBeenOpened = true;
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

bool TextureImport(AssetManager& manager, AssetRegistry& registry) {
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
		texture.display();
		ImGui::Image(texture);

		if (ImGui::Button("Import")) {
			if (importData.imported) {
				manager.ImportTexture(importData.path, importData.name);
				return false;
			}
		}

		if (ImGui::Button("Cancel")) {
			return false;
		}
	}
	else { //Open a file dialog.
		static bool hasBeenOpened = false;
		if (!importData.fileDialog.IsOpened()) {
			importData.fileDialog.Open();
			if (hasBeenOpened) {
				hasBeenOpened = false;
				return false; //Exit
			}
			hasBeenOpened = true;
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

void AssetWindow(AssetManager& manager, AssetRegistry& registry) {
	constexpr int size = 3;

	const char* arr[size] = { "Texture", "Animation", "Spritesheet"};
	const AssetProperties::Type types[size] = { AssetProperties::Type::Texture, AssetProperties::Type::Animation, AssetProperties::Type::Spritesheet };

	static_assert(size > 0);
	static_assert(sizeof(types) / sizeof(AssetProperties::Type) == sizeof(arr) / sizeof(char*), "Combo arrays must remain the same size!");

	static struct AssetGUIData {
		int selectedType = 0; //Index into both arrays.
		bool beginImport = false; //Used to determine if should open a new window.
	} assetType;


	static bool showAssetTypeScreen = !assetType.beginImport;

	static const char* textureString = "TextureImport";
	static const char* animationString = "AnimationImport";
	static const char* spritesheetString = "SpritesheetImport";
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
				ImGui::OpenPopup(spritesheetString);
				break;
			case Animation:
				ImGui::OpenPopup(animationString);
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
			const bool continuing = TextureImport(manager, registry);
			if (!continuing) {
				ImGui::CloseCurrentPopup();
				showAssetTypeScreen = true;
			}
			ImGui::EndPopup();

		}
		else if (ImGui::BeginPopupModal(animationString)) {
			const bool continuing = AnimationImport(manager, registry);
			if (!continuing) {
				ImGui::CloseCurrentPopup();
				showAssetTypeScreen = true;
			}
			ImGui::EndPopup();
		}
		else if (ImGui::BeginPopupModal(spritesheetString)) {
			const bool continuing = SpritesheetImport(manager, registry);
			if (!continuing) {
				ImGui::CloseCurrentPopup();
				showAssetTypeScreen = true;
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}