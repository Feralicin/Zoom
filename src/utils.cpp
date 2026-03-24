#include <algorithm>
#include "utils.hpp"

using namespace geode::prelude;

void zoomPlayLayer(CCNode* PlayLayer, float Delta, CCPoint ScreenAnchor) {
	if (!PlayLayer) return;

	CCSize ContentSize = PlayLayer->getContentSize();
	CCPoint AnchorPoint = ccp(
		ScreenAnchor.x - ContentSize.width / 2,
		ScreenAnchor.y - ContentSize.height / 2
	);

	float OldScale = PlayLayer->getScale();
	float NewScale;

	if (Delta < 0) {
		NewScale = OldScale / (1 - Delta);
	} else {
		NewScale = OldScale * (1 + Delta);
	}

	if (NewScale < 1.0f) NewScale = 1.0f;

	CCPoint DeltaFromAnchorPrev = PlayLayer->getPosition() - AnchorPoint;

	PlayLayer->setPosition(AnchorPoint);
	PlayLayer->setScale(NewScale);
	PlayLayer->setPosition(AnchorPoint + DeltaFromAnchorPrev * NewScale / OldScale);
}

CCSize getScreenSize() {
	float ScreenTop    = CCDirector::sharedDirector()->getScreenTop();
	float ScreenBottom = CCDirector::sharedDirector()->getScreenBottom();
	float ScreenLeft   = CCDirector::sharedDirector()->getScreenLeft();
	float ScreenRight  = CCDirector::sharedDirector()->getScreenRight();

	return CCSize{ ScreenRight - ScreenLeft, ScreenTop - ScreenBottom };
}

void clampPlayLayerPos(CCNode* PlayLayer) {
	if (!PlayLayer) return;

	CCPoint Pos        = PlayLayer->getPosition();
	CCSize  ScreenSize = getScreenSize();
	CCSize  ContentSize = PlayLayer->getContentSize();

	float XLimit = (ContentSize.width  * PlayLayer->getScale() - ScreenSize.width)  * 0.5f;
	float YLimit = (ContentSize.height * PlayLayer->getScale() - ScreenSize.height) * 0.5f;

	Pos.x = std::clamp(Pos.x, -XLimit, XLimit);
	Pos.y = std::clamp(Pos.y, -YLimit, YLimit);

	PlayLayer->setPosition(Pos);
}