#ifdef GEODE_IS_DESKTOP

#include "utils.hpp"
#include "desktop.hpp"
#include "settings.hpp"

#include <Geode/Geode.hpp>

#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#ifdef GEODE_IS_WINDOWS
#include <Geode/modify/CCEGLView.hpp>
#include <windows.h>
#else
#include <objc/message.h>
#endif
#include <Geode/modify/CCScheduler.hpp>

#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

WindowsZoomManager* WindowsZoomManager::get() {
	static auto Inst = new WindowsZoomManager;
	return Inst;
}

void WindowsZoomManager::togglePauseMenu() {
	if (!isPaused) return;

	CCNode* PauseLayer = CCScene::get()->getChildByID("PauseLayer");
	if (!PauseLayer) return;

	PauseLayer->setVisible(!PauseLayer->isVisible());
}

void WindowsZoomManager::setPauseMenuVisible(bool Visible) {
	CCNode* PauseLayer = CCScene::get()->getChildByID("PauseLayer");
	if (!PauseLayer) return;

	PauseLayer->setVisible(Visible);
}

void WindowsZoomManager::setZoom(float Zoom) {
	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return;

	PlayLayer->setScale(Zoom);
	onScreenModified();
}

void WindowsZoomManager::zoom(float Delta) {
	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return;

	CCPoint MousePos = getMousePos();

	zoomPlayLayer(PlayLayer, Delta, MousePos);
	onScreenModified();
}

void WindowsZoomManager::move(CCPoint Delta) {
	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return;

	CCPoint Pos = PlayLayer->getPosition();
	PlayLayer->setPosition(Pos + Delta);

	onScreenModified();
}

void WindowsZoomManager::setPos(float X, float Y) {
	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return;

	PlayLayer->setPosition(CCPoint{ X, Y });

	onScreenModified();
}

float WindowsZoomManager::getZoom() {
	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return 1.0f;

	return PlayLayer->getScale();
}

CCPoint WindowsZoomManager::screenToWorld(CCPoint Pos) {
	CCSize ScreenSize = getScreenSize();
	CCSize WinSize    = CCEGLView::get()->getFrameSize();

	return CCPoint{ Pos.x * (ScreenSize.width / WinSize.width), Pos.y * (ScreenSize.height / WinSize.height) };
}

CCPoint WindowsZoomManager::getMousePosOnNode(CCNode* Node) {
	return Node->convertToNodeSpace(getMousePos());
}

void WindowsZoomManager::update(float Dt) {
	auto MousePos     = getMousePos();
	auto LastMousePos = WindowsZoomManager::get()->lastMousePos;

	WindowsZoomManager::get()->deltaMousePos = CCPoint{ MousePos.x - LastMousePos.x, MousePos.y - LastMousePos.y };
	WindowsZoomManager::get()->lastMousePos  = MousePos;

	if (!isPaused) return;

	if (isPanning) {
		CCPoint Delta = WindowsZoomManager::get()->deltaMousePos;
		move(Delta);
	}
}

void WindowsZoomManager::onResume() {
	setZoom(1.0f);
	setPos(0.0f, 0.0f);

	isPaused  = false;
	WindowsZoomManager::get()->isPanning = false;
}

void WindowsZoomManager::onPause() {
	isPaused  = true;
	WindowsZoomManager::get()->isPanning = false;
}

void WindowsZoomManager::onScroll(float Y, float X) {
	if (!isPaused) return;

	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return;

	if (SettingsManager::get()->altDisablesZoom) {
		auto Kb = CCKeyboardDispatcher::get();
		if (Kb->getAltKeyPressed()) return;
	}

	float ZoomDelta = SettingsManager::get()->zoomSensitivity * 0.1f;

	if (Loader::get()->isModLoaded("prevter.smooth-scroll")) {
		zoom(-Y * ZoomDelta * 0.1f);
	} else if (Y > 0) {
		zoom(-ZoomDelta);
	} else {
		zoom(ZoomDelta);
	}

	if (Y > 0) {
		if (SettingsManager::get()->autoShowMenu && PlayLayer->getScale() <= 1.01f) {
			setPauseMenuVisible(true);
		}
	} else {
		if (SettingsManager::get()->autoHideMenu) setPauseMenuVisible(false);
	}
}

void WindowsZoomManager::onScreenModified() {
	CCNode* PlayLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!PlayLayer) return;

	clampPlayLayerPos(PlayLayer);
	if (!isPaused) return;
}

class $modify(PauseLayer) {
	void customSetup() {
		this->addEventListener(
			KeybindSettingPressedEventV3(Mod::get(), "toggle-menu"),
			[=](Keybind const& Keybind, bool Down, bool Repeat, double Timestamp) {
				if (Down && !Repeat) {
					WindowsZoomManager::get()->togglePauseMenu();
				}
			}
		);

		PauseLayer::customSetup();
	}

	void onResume(CCObject* Sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onResume(Sender);
	}

	void onRestart(CCObject* Sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onRestart(Sender);
	}

	void onRestartFull(CCObject* Sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onRestartFull(Sender);
	}

	void onNormalMode(CCObject* Sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onNormalMode(Sender);
	}

	void onPracticeMode(CCObject* Sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onPracticeMode(Sender);
	}
};

class $modify(PlayLayer) {
	void pauseGame(bool P0) {
		WindowsZoomManager::get()->onPause();
		PlayLayer::pauseGame(P0);
	}

	void startGame() {
		WindowsZoomManager::get()->onResume();
		PlayLayer::startGame();
	}

	bool init(GJGameLevel* Level, bool UseReplay, bool DontCreateObjects) {
		WindowsZoomManager::get()->onResume();
		return PlayLayer::init(Level, UseReplay, DontCreateObjects);
	}
};

class $modify(CCScheduler) {
	virtual void update(float Dt) {
		WindowsZoomManager::get()->update(Dt);
		CCScheduler::update(Dt);
	}
};

#ifdef GEODE_IS_WINDOWS
static GLFWmousebuttonfun PrevMouseButtonCallback = nullptr;

using GlfwSetMouseButtonCallbackFn = GLFWmousebuttonfun(*)(GLFWwindow*, GLFWmousebuttonfun);

$execute {
	auto GlfwModule = GetModuleHandleA("glfw3.dll");
	if (!GlfwModule) return;

	auto SetCallback = reinterpret_cast<GlfwSetMouseButtonCallbackFn>(
		GetProcAddress(GlfwModule, "glfwSetMouseButtonCallback")
	);
	if (!SetCallback) return;

	auto* Window        = CCEGLView::get()->getWindow();
	PrevMouseButtonCallback = SetCallback(Window, [](GLFWwindow* Window, int Button, int Action, int Mods) {
		if (Button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (Action == GLFW_PRESS) {
				WindowsZoomManager::get()->isPanning = true;
			} else if (Action == GLFW_RELEASE) {
				WindowsZoomManager::get()->isPanning = false;
			}
		}

		if (PrevMouseButtonCallback) PrevMouseButtonCallback(Window, Button, Action, Mods);
	});
}
#else
void otherMouseDownHook(void* Self, SEL Sel, void* Event) {
	WindowsZoomManager::get()->isPanning = true;
	reinterpret_cast<void(*)(void*, SEL, void*)>(objc_msgSend)(Self, Sel, Event);
}

void otherMouseUpHook(void* Self, SEL Sel, void* Event) {
	WindowsZoomManager::get()->isPanning = false;
	reinterpret_cast<void(*)(void*, SEL, void*)>(objc_msgSend)(Self, Sel, Event);
}

$execute {
	if (auto Hook = ObjcHook::create("EAGLView", "otherMouseDown:", &otherMouseDownHook)) {
		(void) Mod::get()->claimHook(Hook.unwrap());
	}

	if (auto Hook = ObjcHook::create("EAGLView", "otherMouseUp:", &otherMouseUpHook)) {
		(void) Mod::get()->claimHook(Hook.unwrap());
	}
}
#endif

class $modify(CCMouseDispatcher) {
	bool dispatchScrollMSG(float Y, float X) {
		WindowsZoomManager::get()->onScroll(Y, X);
		return CCMouseDispatcher::dispatchScrollMSG(Y, X);
	}
};
#endif