#include "settings.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

SettingsManager* SettingsManager::get() {
	static auto Inst = new SettingsManager;
	return Inst;
}

void SettingsManager::init() {
	#ifdef GEODE_IS_DESKTOP
	autoHideMenu = Mod::get()->getSettingValue<bool>("auto-hide-menu");
	listenForSettingChanges<bool>("auto-hide-menu", [&](bool Enable) {
		autoHideMenu = Enable;
	});

	autoShowMenu = Mod::get()->getSettingValue<bool>("auto-show-menu");
	listenForSettingChanges<bool>("auto-show-menu", [&](bool Enable) {
		autoShowMenu = Enable;
	});

	altDisablesZoom = Mod::get()->getSettingValue<bool>("alt-disables-zoom");
	listenForSettingChanges<bool>("alt-disables-zoom", [&](bool Enable) {
		altDisablesZoom = Enable;
	});

	zoomSensitivity = Mod::get()->getSettingValue<float>("zoom-sensitivity");
	listenForSettingChanges<float>("zoom-sensitivity", [&](float Sensitivity) {
		zoomSensitivity = Sensitivity;
	});
	#endif
}