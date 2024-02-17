#include <Geode/Geode.hpp>

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

#include <Geode/modify/CCScheduler.hpp>

#include <Geode/binding/FModAudioEngine.hpp>

#include <Geode/binding/GameManager.hpp>

#include <functional>

#include <algorithm>
#include <random>

using namespace geode::prelude;

namespace RGlobal {
	bool isEnd = false;
	void *playLayer = nullptr;
	float speed = 1.f;
	float old_fps = 1.f;

	bool inHorrorMode = false;
	bool inRoulette = false;

	void fixCursor() {
		CCDirector::sharedDirector()->getOpenGLView()->toggleLockCursor(false);
		CCDirector::sharedDirector()->getOpenGLView()->showCursor(true);
	}
}

class HorrorControllerNode : public CCNode {
private:
	std::vector<CCNode *> _shakingObjects;
	std::map<CCNode *, CCPoint> _shakeDelta;
	std::function<void(HorrorControllerNode *, void *)> _callback;
	void *_ctx;
	double _time;
	double _randomJTime;
public:
	HorrorControllerNode() : CCNode() {
		_shakingObjects = {};
		_shakeDelta = {};
		_callback = nullptr;
		_time = 0;
		// _randomJTime = (double)(rand() % 45);
		_randomJTime = 48;
		_ctx = nullptr;
	}

	static HorrorControllerNode *create(bool play_sound) {
		auto ret = new HorrorControllerNode();

		if (ret && ret->init(play_sound)) {
			ret->autorelease();

			return ret;
		}

		CC_SAFE_DELETE(ret);

		return nullptr;
	}

	void addShakingObject(CCNode *obj) {
		_shakingObjects.push_back(obj);
	}

	void resetTimer() {
		_time = 0;
	}

	void setJumpscareCallback(std::function<void(HorrorControllerNode *, void *)> callback, void *ctx) {
		_callback = callback;
		_ctx = ctx;
	}

	void removeNodesAndCleanup(bool delete_objects) {
		for (auto obj : _shakingObjects) {
			if (delete_objects) {
				obj->removeMeAndCleanup();
			}
		}

		_shakingObjects = {};

		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
		engine->stopAllMusic();
	}

	void playSound() {
		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
		
		engine->stopAllMusic();
		engine->playMusic("geode/unzipped/dogotrigger.roulette/resources/dogotrigger.roulette/horror.mp3", true, 1.f, 0);
	}
	void playSoundJ() {
		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();

		engine->setChannelVolume(0, (AudioTargetType)0, 5.f);
	}

	bool init(bool play_sound) {
		if (!CCNode::init()) return false;

		scheduleUpdate();

		if (play_sound) playSound();

		return true;
	}

	void update(float delta) {
		for (auto obj : _shakingObjects) {
			bool _negX = rand() % 2;
			bool _negY = rand() % 2;

			int mulX = (_negX) ? -1 : 1;
			int mulY = (_negY) ? -1 : 1;

			int max = 6;

			int x = (int)(rand() % max) * mulX;
			int y = (int)(rand() % max) * mulY;

			CCPoint d(x, y);
			
			if (_shakeDelta.contains(obj)) {
				CCPoint old_pos = _shakeDelta[obj];

				obj->setPositionX(obj->getPositionX() - old_pos.x);
				obj->setPositionY(obj->getPositionY() - old_pos.y);
			}
			
			obj->setPositionX(obj->getPositionX() + x);
			obj->setPositionY(obj->getPositionY() + y);

			_shakeDelta[obj] = d;
		}

		if (RGlobal::inRoulette) return;

		_time += (double)delta;

		log::debug("t: {}/{}", _time, _randomJTime);

		if (_time > _randomJTime) {
			// _randomJTime = (double)(rand() % 45);
			_randomJTime = 48;
			resetTimer();

			playSoundJ();

			if (_callback) _callback(this, _ctx);
		}
	}
};

class ToiletNode : public CCNode {
private:
	CCSprite *_toilet;

	double _time;
public:
	ToiletNode() : CCNode() {
		_toilet = nullptr;
		_time = 0.f;
	}

	static ToiletNode *create() {
		auto ret = new ToiletNode();

		if (ret && ret->init()) {
			ret->autorelease();

			return ret;
		}

		CC_SAFE_DELETE(ret);

		return nullptr;
	}

	void playSound() {
		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
		
		engine->playEffect("geode/unzipped/dogotrigger.roulette/resources/dogotrigger.roulette/Tada.mp3", 1.f, 1.f, 1.f);
	}

	bool init() {
		if (!CCNode::init()) return false;

		_toilet = CCSprite::create("Toilet.png"_spr);

		addChild(_toilet);

		scheduleUpdate();

		return true;
	}

	CCSprite *getToiletSprite() {
		return _toilet;
	}

	void update(float delta) {
		_time += (double)delta;

		// _toilet->setRotation(sin(_time * 2) * 10.f);
		_toilet->setPositionY(sin(_time * 2) * 20.f);
	}
};

class NormalFaceLayer : public CCLayer {
private:
	float _currentVol;
	bool _playSound;
public:
	NormalFaceLayer(bool play_sound) : CCLayer() {
		_currentVol = 1.f;
		_playSound = play_sound;
	}

	static NormalFaceLayer *create(bool play_sound) {
		auto ret = new NormalFaceLayer(play_sound);

		if (ret && ret->init()) {
			ret->autorelease();

			return ret;
		}

		CC_SAFE_DELETE(ret);

		return nullptr;
	}

	void animEnd(float delta) {
		removeMeAndCleanup();
	}

	void playSound(float delta) {
		std::string sound_name = "sfx/s4451.ogg";
		std::string sound_path = "Resources/" + sound_name;

		if (!std::filesystem::exists(sound_path)) return;

		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
		engine->playEffect(sound_name, 1.f, 1.f, _currentVol);

		float cv = _currentVol * 100.f;

		switch((int)cv) {
			case 100: {
				_currentVol = 0.5f;
				break;
			}
			case 50: {
				_currentVol = 0.25f;
				break;
			}
			case 25: {
				_currentVol = 0.05;
				break;
			}
			case 5: {
				_currentVol = 0.02;
				break;
			}
			case 2: {
				_currentVol = 0.01;
				break;
			}
			case 0: {}
			case 1: {
				_currentVol = 0.00;
				break;
			}
			default: {
				break;
			}
		}
	}

	bool init() {
		if (!CCLayer::init()) return false;

		auto spr = CCSprite::createWithSpriteFrameName("diffIcon_02_btn_001.png");

		spr->setID("face");
		spr->setScale(0.f);

		float time = 1.5f;

		spr->runAction(CCEaseExponentialOut::create(CCScaleTo::create(time, 20.f)));

		scheduleOnce(schedule_selector(NormalFaceLayer::animEnd), time);
		spr->runAction(CCFadeTo::create(time, 0.f));

		if (_playSound) {
			schedule(schedule_selector(NormalFaceLayer::playSound), 0.2f, 5, 0.f);
		}

		addChild(spr);

		return true;
	}
};

#include <vector>
class RouletteObject : public CCLayer {
private:
	std::vector<std::string> _values;
	std::vector<CCLabelBMFont *> _labels;

	CCMenu *_menu1;
	CCMenu *_menu2;

	CCMenu *_currentMenu;

	float _scalingPowerX;
	float _scalingPowerY;

	CCScale9Sprite *_square07;
	CCScale9Sprite *_blackLeft;
	CCScale9Sprite *_blackRight;

	bool _infoCreated;

	bool _canRotate;
	float _rotationSpeed;

	bool _rouletteStopping;
	bool _rotationStarted;

	// CCMenuItemSpriteExtra *_stopBtn;
	// CCSprite *_stopBtnSpr;
	CCNode *_stopBtnNode;

	CCNode *_selectedObject;
	CCNode *_oldObject;
	CCNode *_oldActualObject;

	CCLabelBMFont *_selectedObjectText;

	CCSprite *_pointer;

	bool _shouldForce;

	std::function<void(RouletteObject *)> _callback;
public:
	RouletteObject(std::vector<std::string> values) : CCLayer() {
		_values = values;
		_labels = {};

		_menu1 = nullptr;
		_menu2 = nullptr;

		_oldActualObject = nullptr;
		_oldObject = nullptr;
		_selectedObject = nullptr;

		_selectedObjectText = nullptr;

		_rouletteStopping = false;

		_square07 = nullptr;
		_blackLeft = nullptr;
		_blackRight = nullptr;

		_pointer = nullptr;

		_scalingPowerX = 6.7;
		_scalingPowerY = 3.6;

		_infoCreated = false;

		_canRotate = false;
		_rouletteStopping = false;
		_shouldForce = false;

		_rotationSpeed = 0;
		_rotationStarted = false;

		_callback = nullptr;
	}

	static RouletteObject *create(std::vector<std::string> values) {
		auto ret = new RouletteObject(values);

		if (ret && ret->init()) {
			ret->autorelease();

			return ret;
		}

		CC_SAFE_DELETE(ret);

		return nullptr;
	}

	std::string getSelectedEntry() {
		if (_selectedObject != nullptr) {
			CCLabelBMFont *txt = dynamic_cast<CCLabelBMFont *>(_selectedObject->getChildByID("text"));

			std::string str = txt->getString();

			return str;
		}

		return "";
	}

	void beginEnd4(float delta) {
		if (_callback) _callback(this);
	}
	void beginEnd3(float delta) {
		_blackLeft->removeMeAndCleanup();
		_blackRight->removeMeAndCleanup();

		_blackLeft = nullptr;
		_blackRight = nullptr;

		_selectedObjectText->runAction(CCFadeTo::create(0.5, 0));
		_square07->runAction(CCFadeTo::create(0.5, 0));
		_pointer->runAction(CCFadeTo::create(0.5, 0));
		scheduleOnce(schedule_selector(RouletteObject::beginEnd4), 0.5);
	}
	void beginEnd2(float delta) {
		CCArray *children = _menu1->getChildren();
		CCObject *obj;

		CCARRAY_FOREACH(children, obj) {
			CCNode *nd = dynamic_cast<CCNode *>(obj);

			CCSprite *line_start = dynamic_cast<CCSprite *>(nd->getChildByID("line-start"));
			CCSprite *line_end = dynamic_cast<CCSprite *>(nd->getChildByID("line-end"));

			CCLabelBMFont *txt = dynamic_cast<CCLabelBMFont *>(nd->getChildByID("text"));
		
			line_start->runAction(CCFadeTo::create(0.5, 0));
			line_end->runAction(CCFadeTo::create(0.5, 0));

			txt->runAction(CCFadeTo::create(0.5, 0));
		}

		scheduleOnce(schedule_selector(RouletteObject::beginEnd3), 0.5);
	}
	void beginEnd() {
		scheduleOnce(schedule_selector(RouletteObject::beginEnd2), 0.5);
		_stopBtnNode->removeMeAndCleanup();
		unscheduleUpdate();
	}

	void setupEndSound() {
		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
		engine->playEffect("crystal01.ogg", 1.f, 0.5f, 0.5f);
	}

	void beginRotation(float delta) {
		// _rotationSpeed = 60;
		_canRotate = true;
		_rotationStarted = true;
	}

	void updateRouletteStartAnim(float delta_) {
		float delta = 0.05f;

		float speed = 4 * delta * 20;

		_scalingPowerX -= delta * speed;
		_scalingPowerY -= delta * speed;

		if (_scalingPowerX > 0) {
			auto cs = _square07->getContentSize();
			cs.width += _scalingPowerX * speed;

			_square07->setContentSize(cs);
		} else {
			auto cs = _square07->getContentSize();
			cs.width = 445.f;

			_square07->setContentSize(cs);
		}
		if (_scalingPowerY > 0) {
			auto cs = _square07->getContentSize();
			cs.height += _scalingPowerY * speed;

			_square07->setContentSize(cs);
		} else {
			auto cs = _square07->getContentSize();
			cs.height = 110.f;

			_square07->setContentSize(cs);
		}
	}

	bool init() {
		std::vector<CCScale9Sprite *> squares = {
			CCScale9Sprite::create("square.png"), CCScale9Sprite::create("square.png")
		};

		_blackLeft = squares[0];
		_blackRight = squares[1];
		CCScale9Sprite *square07 = CCScale9Sprite::create("GJ_square07.png");

		square07->setOpacity(0);
		square07->runAction(CCFadeTo::create(0.25f, 255));

		for (auto square : squares) {
			square->setContentSize({500.f, 100.f});
			square->setColor({0, 0, 0});
		}

		_menu1 = CCMenu::create();
		_menu2 = CCMenu::create();

		CCMenu *menu = _menu1;

		int idx = 0;

		for (auto str : _values) {
			CCNode *nd = CCNode::create();

			CCLabelBMFont *l = CCLabelBMFont::create(str.c_str(), "chatFont.fnt");

			l->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

			l->setRotation(90);

			l->setOpacity(0);
			l->runAction(CCFadeTo::create(0.5f, 255));

			l->setID("text");

			_labels.push_back(l);

			nd->addChild(l);

			std::vector<CCSprite *> lines;

			float l1X = -l->getContentSize().height / 2;
			float l2X = l->getContentSize().height / 2;

			auto line1 = CCSprite::createWithSpriteFrameName("floorLine_001.png");
			line1->setPosition({l1X, 0});
			line1->setID("line-start");
			lines.push_back(line1);

			auto line2 = CCSprite::createWithSpriteFrameName("floorLine_001.png");
			line2->setPosition({l2X, 0});
			line2->setID("line-end");
			lines.push_back(line2);

			for (auto line : lines) {
				line->setRotation(90);
				line->setScaleX(0.2);

				line->setOpacity(0);
				line->runAction(CCFadeTo::create(0.25f, 255));

				nd->addChild(line);
			}
			
			_menu1->addChild(nd);
			_menu2->addChild(nd);

			idx++;
		}

		menu->alignItemsHorizontallyWithPadding(445.f / _values.size());

		square07->setContentSize({0, 0});

		scheduleUpdate();

		menu->setPosition(0, 0);
		_menu2->setPosition((menu->getContentSize().width / 1.28), 0);

		addChild(square07);;
		addChild(menu);
		addChild(_menu2);
		addChild(_blackLeft);
		addChild(_blackRight);

		scheduleOnce(schedule_selector(RouletteObject::beginRotation), 1);

		_square07 = square07;

		_pointer = CCSprite::createWithSpriteFrameName("floorLine_001.png");
		
		_pointer->setRotation(90);
		_pointer->setScaleX(0.2);
		_pointer->setColor(ccYELLOW);
		_pointer->setOpacity(0);
		_pointer->runAction(CCFadeTo::create(0.25f, 255));

		addChild(_pointer);

		_currentMenu = _menu1;

		schedule(schedule_selector(RouletteObject::updateRouletteStartAnim), 0.05);

		return true;
	}

	void stopRoulette(CCObject *obj) {
		if (_rouletteStopping) return;
		_rouletteStopping = true;

		CCSprite *black_square = dynamic_cast<CCSprite *>(_stopBtnNode->getChildByID("black-square"));
		CCMenuItemSpriteExtra *stop_btn = dynamic_cast<CCMenuItemSpriteExtra *>(_stopBtnNode->getChildByIDRecursive("stop-button"));

		black_square->runAction(CCFadeTo::create(0.5f, 255));

		stop_btn->setEnabled(false);
	}

	void createRouletteInfo(float delta) {
		if (_infoCreated && _rotationStarted) return;

		_infoCreated = true;

		auto btnSpr = ButtonSprite::create("Stop Roulette");

		auto stopBtn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(RouletteObject::stopRoulette));
		stopBtn->setID("stop-button");

		CCMenu *men = CCMenu::createWithItem(stopBtn);
		men->setID("button-menu");

		men->setPosition({0, 0});

		CCSprite *black_square = CCSprite::create("square.png");

		black_square->setID("black-square");
		black_square->setScaleX(40);
		black_square->setScaleY(5);
		black_square->setColor(ccBLACK);
		black_square->setOpacity(255);
		black_square->runAction(CCFadeTo::create(0.5f, 0));

		_stopBtnNode = CCNode::create();

		_stopBtnNode->setPositionX(0);
		_stopBtnNode->setPositionY(-_square07->getContentSize().height + 15);

		_stopBtnNode->addChild(men);
		_stopBtnNode->addChild(black_square);

		// men->setScale(0.5f);

		addChild(_stopBtnNode);

		_selectedObjectText = CCLabelBMFont::create("(Selected entry would be here.)", "chatFont.fnt");
		_selectedObjectText->setPositionX(0);
		_selectedObjectText->setPositionY(_square07->getContentSize().height - _selectedObjectText->getContentSize().height);
		
		_selectedObjectText->setScale(1.5f);

		_selectedObjectText->setOpacity(0);
		_selectedObjectText->runAction(CCFadeTo::create(0.5f, 255));

		addChild(_selectedObjectText);
	}

	void setEndCallback(std::function<void(RouletteObject *)> callback) {
		_callback = callback;
	}

	bool checkCollisionWithMenu(CCMenu *menu) {
		_selectedObject = nullptr;

		auto children = menu->getChildren();
		CCObject *obj;

		std::vector<CCRect> entries;

		CCARRAY_FOREACH(children, obj) {
			CCNode *nd = dynamic_cast<CCNode *>(obj);
			
			CCNode *line_start = nd->getChildByID("line-start");
			CCNode *line_end = nd->getChildByID("line-end");

			float szX = line_end->getPositionX() - line_start->getPositionX();
			
			CCRect r;
			r.origin.x = menu->getPositionX() + nd->getPositionX();
			r.size.width = szX;

			entries.push_back(r);
		}

		int index = 0;

		for (CCRect r : entries) {
			if (_pointer->getPositionX() > (r.origin.x - r.size.width / 2) && _pointer->getPositionX() < (r.origin.x + r.size.width)) {
				obj = children->objectAtIndex(index);
				_selectedObject = dynamic_cast<CCNode *>(obj);

				break;
			}

			index++;
		}

		return _selectedObject != nullptr;
	}

	void update(float delta) override {
		CCNode::update(delta);

		RGlobal::fixCursor();

		if (_rotationStarted && !_rouletteStopping && _rotationSpeed < 60.f && !_shouldForce) {
			_rotationSpeed += delta * 15;
		}

		if (_rouletteStopping) {
			_rotationSpeed -= delta * 10;

			if (_rotationSpeed < 0) _rotationSpeed = 0;
		}

		if (_blackLeft != nullptr) {
			_blackLeft->setPositionX(_square07->getPositionX() - (_square07->getContentSize().width / 2) - (_blackLeft->getContentSize().width / 2));
		}

		if (_blackRight != nullptr) {
			_blackRight->setPositionX(_square07->getPositionX() + (_square07->getContentSize().width / 2) + (_blackLeft->getContentSize().width / 2));
		}

		if (_canRotate || _shouldForce) {
			_menu1->setPositionX(_menu1->getPositionX() - (10 * delta * _rotationSpeed));
			_menu2->setPositionX(_menu2->getPositionX() - (10 * delta * _rotationSpeed));

			if (_menu1->getPositionX() < -(_menu1->getContentSize().width / 1.28)) {
				_menu1->setPosition((_menu2->getContentSize().width / 1.28), 0);
				_currentMenu = _menu2;
			}
			if (_menu2->getPositionX() < -(_menu2->getContentSize().width / 1.28)) {
				_menu2->setPosition((_menu1->getContentSize().width / 1.28), 0);
				_currentMenu = _menu1;
			}

			scheduleOnce(schedule_selector(RouletteObject::createRouletteInfo), 0);
		}

		_oldObject = _selectedObject;
		_selectedObject = nullptr;

		if (!checkCollisionWithMenu(_menu1)) {
			checkCollisionWithMenu(_menu2);
		}

		if (_selectedObject == nullptr) {
			if (_oldActualObject != nullptr) {
				CCLabelBMFont *txt = dynamic_cast<CCLabelBMFont *>(_oldActualObject->getChildByID("text"));
				txt->setColor(ccWHITE);

				if (_selectedObjectText) {
					_selectedObjectText->setString(" ", true);
				}
			}
		}

		if (_shouldForce && _selectedObject != nullptr) {
			_shouldForce = false;
			_canRotate = false;
			setupEndSound();
			beginEnd();
		}

		if (_rotationSpeed == 0.f && _selectedObject == nullptr && _rotationStarted) {
			_shouldForce = true;
			_rouletteStopping = false;
			_rotationSpeed = 10;
		}

		if (_rotationSpeed == 0 && _selectedObject != nullptr && _rouletteStopping) {
			setupEndSound();
			beginEnd();
		}


		if (_selectedObject != _oldObject && _selectedObject != nullptr && _rotationStarted) {
			FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
			engine->playEffect("counter003.ogg", 1.f, 0.5f, 0.5f);

			if (_oldActualObject != nullptr) {
				CCLabelBMFont *txt = dynamic_cast<CCLabelBMFont *>(_oldActualObject->getChildByID("text"));
				txt->setColor(ccWHITE);
			}

			CCLabelBMFont *txt = dynamic_cast<CCLabelBMFont *>(_selectedObject->getChildByID("text"));
			txt->setColor(ccYELLOW);

			_selectedObjectText->setString(txt->getString(), true);
			_selectedObjectText->stopAllActions();
			_selectedObjectText->setOpacity(255);
			_selectedObjectText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

			_oldActualObject = _selectedObject;
		}
	}
};

#include <map>

// class TouletteAppereance {
// public:
// 	bool placedRight;
// };

class $modify(XPlayLayer, PlayLayer) {
	float player_x_old;
	float player_x_new;
	float player_x_delta;

	bool levelStarted;
	bool rouletteStarted;

	bool shouldHideCursor = false;

	CCNode *rouletteNode;
	CCSprite *blackSquare;

	std::string task;

	RouletteObject *_Robj;

	std::vector<ToiletNode *> _Tobj;

	bool _payloadRandomBlock = false;

	float _time = 0.f;

	std::map<std::string, std::function<void(XPlayLayer *)>> taskMapping;

	void unloadPayload(bool ending) {
		bool pernamentEffects = Mod::get()->getSettingValue<bool>("pernament-effects");

		unschedule(schedule_selector(XPlayLayer::rotatingWorld));
		unschedule(schedule_selector(XPlayLayer::roulette3DWorldLoop));
		unschedule(schedule_selector(XPlayLayer::rouletteGiantPlayerLoop));
		unschedule(schedule_selector(XPlayLayer::rouletteLobotomyBegin));

		setRotation(0.f);

		if (ending || !pernamentEffects) m_fields->_payloadRandomBlock = false;
		
		CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.f);
		if (RGlobal::old_fps != 1.f) CCDirector::sharedDirector()->setAnimationInterval(RGlobal::old_fps);

		if (ending || !pernamentEffects) {
			RGlobal::old_fps = 1.f;
			RGlobal::speed = 1.f;
		}

		setSkewX(0);
		setSkewY(0);
		setScale(1.f);

		m_fields->_time = 0.f;
	}

	void rotatingWorld(float delta) {
		this->setRotation(getRotation() + (delta * 200));
	}
	static void rouletteRotatingWorld(XPlayLayer *pl) {
		pl->schedule(schedule_selector(XPlayLayer::rotatingWorld));
	}

	static void rouletteDoNothing(XPlayLayer *pl) {}
	static void rouletteClose(XPlayLayer *pl) {
		CCApplication::sharedApplication()->shutdownApplication();
		exit(0);
	}
	static void rouletteHalfSpeed(XPlayLayer *pl) {
		RGlobal::speed = 0.5;
		CCDirector::sharedDirector()->getScheduler()->setTimeScale(0.5f);
	}
	void roulette3DWorldLoop(float delta) {
		m_fields->_time += delta;

		float tm = sin(m_fields->_time);

		setSkewX(tm * 24.f);
	}
	static void roulette3DLevel(XPlayLayer *pl) {
		pl->schedule(schedule_selector(XPlayLayer::roulette3DWorldLoop));
	}
	static void rouletteDoubleSpeed(XPlayLayer *pl) {
		RGlobal::speed = 2;
		CCDirector::sharedDirector()->getScheduler()->setTimeScale(2.0f);
	}
	void rouletteGiantPlayerLoop(float delta) {
		m_player1->setScale(3.f);
	}
	static void rouletteGiantPlayer(XPlayLayer *pl) {
		pl->schedule(schedule_selector(XPlayLayer::rouletteGiantPlayerLoop));
	}

	void addTrash(float delta) {
		GameObject *obj = GameObject::createWithKey(1331);

		float base_offsetX = rand() % 200;
		float base_offsetY = rand() % 200;

		auto pos = m_player1->getPosition();
		pos.x += base_offsetX;
		pos.y += base_offsetY;

		obj->setPosition(pos);

		obj->setVisible(true);
	
		addObject(obj);

		obj->setVisible(true);
	}

	static void rouletteTrash(XPlayLayer *pl) {
		pl->m_fields->_payloadRandomBlock = true;
	}

	void rouletteLobotomy01(float delta) {
		auto js = NormalFaceLayer::create(true);
		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		js->setPosition(winSize / 2);

		addChild(js, 99999);
	}
	void rouletteLobotomy02(float delta) {
		auto js = NormalFaceLayer::create(true);
		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		js->setPosition(winSize / 4);

		addChild(js, 99999);
	}
	void rouletteLobotomy03(float delta) {
		auto js = NormalFaceLayer::create(true);
		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		js->setPosition(winSize);

		addChild(js, 99999);
	}
	void rouletteLobotomy04(float delta) {
		auto js = NormalFaceLayer::create(true);

		addChild(js, 99999);
	}

	void rouletteLobotomyBegin(float delta) {
		schedule(schedule_selector(XPlayLayer::rouletteLobotomy01), 0.f, 0, 0.0f);
		schedule(schedule_selector(XPlayLayer::rouletteLobotomy02), 0.f, 0, 0.2f);
		schedule(schedule_selector(XPlayLayer::rouletteLobotomy03), 0.f, 0, 0.4f);
		schedule(schedule_selector(XPlayLayer::rouletteLobotomy04), 0.f, 0, 0.6f);
	}

	static void rouletteLobotomy(XPlayLayer *pl) {
		pl->rouletteLobotomyBegin(0.f);
		pl->schedule(schedule_selector(XPlayLayer::rouletteLobotomyBegin), 4.f);
	}

	static void rouletteHorrorCallback(HorrorControllerNode *nd, void *ctx) {
		XPlayLayer *pl = static_cast<XPlayLayer *>(ctx);

		FMODAudioEngine *engine = FMODAudioEngine::sharedEngine();
		engine->stopAllMusic();

		Sleep(1500);

		for (auto obj : pl->m_fields->_Tobj) {
			obj->playSound();

			float time = 1.5f;

			obj->runAction(CCEaseExponentialOut::create(CCScaleTo::create(time, 30.f)));
			obj->getToiletSprite()->runAction(CCFadeTo::create(time, 0.f));
		}

		auto ndh_ = pl->getChildByIDRecursive("horror-controller");

		if (ndh_) {
			auto ndh = dynamic_cast<HorrorControllerNode *>(ndh_);

			ndh->removeNodesAndCleanup(false);
			ndh->removeMeAndCleanup();
		}

		pl->m_fields->_Tobj = {};
	}

	void rouletteHorrorEnd(float delta) {
		auto ndh_ = getChildByIDRecursive("horror-controller");

		if (ndh_) {
			auto ndh = dynamic_cast<HorrorControllerNode *>(ndh_);

			ndh->removeNodesAndCleanup(true);
			ndh->removeMeAndCleanup();
		}

		m_fields->_Tobj = {};
	}

	static void rouletteHorror(XPlayLayer *pl) {
		ToiletNode *nd = ToiletNode::create();

		HorrorControllerNode *ndh;

		if (!pl->getChildByIDRecursive("horror-controller")) {
			ndh = HorrorControllerNode::create(true);
			ndh->setJumpscareCallback(XPlayLayer::rouletteHorrorCallback, pl);
			ndh->setID("horror-controller");

			pl->addChild(ndh);

			RGlobal::inHorrorMode = true;
		} else {
			auto ndh_ = pl->getChildByIDRecursive("horror-controller");

			ndh = dynamic_cast<HorrorControllerNode *>(ndh_);

			RGlobal::inHorrorMode = true;
		}

		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		nd->setPosition(winSize / 2);
		// nd->setPositionX(winSize.width - nd->getToiletSprite()->getTextureRect().getMaxX() - 50.f);

		bool right = rand() % 2;

		if (right) {
			nd->runAction(CCEaseInOut::create(CCMoveTo::create(
				1.f, {winSize.width - nd->getToiletSprite()->getTextureRect().getMaxX(), nd->getPositionY()}
			), 2.f));
		} else {
			nd->runAction(CCEaseInOut::create(CCMoveTo::create(
				1.f, {nd->getToiletSprite()->getTextureRect().getMaxX(), nd->getPositionY()}
			), 2.f));
		}

		// nd->setID(fmt::format("toiletnd_{}_{}", right, ));

		ndh->addShakingObject(nd);

		// pl->schedule(schedule_selector(XPlayLayer::rouletteHorrorEnd), 0.f, 0, 60000.f);

		pl->addChild(nd, 99999);
		
		pl->m_fields->_Tobj.push_back(nd);
	}

	void levelComplete() {
		unloadPayload(true);
		
		_payloadRandomBlock = false;
		
		RGlobal::speed = 1;
		RGlobal::isEnd = true;

		PlayLayer::levelComplete();
	}

	void endRoulette2(float delta) {
		m_fields->rouletteNode->removeMeAndCleanup();

		RGlobal::inRoulette = false;

		// log::debug("selected entry is {}", m_fields->task);

		m_fields->levelStarted = false;
		RGlobal::isEnd = false;
		m_fields->rouletteStarted = false;

		scheduleUpdate();

		resetLevel();
 
		if (m_fields->taskMapping.contains(m_fields->task)) {
			auto func = m_fields->taskMapping[m_fields->task];

			func(this);
		}

		m_fields->levelStarted = true;

		CCDirector::sharedDirector()->getScheduler()->setTimeScale(RGlobal::speed);

		log::debug("inHorrorMode: {}", RGlobal::inHorrorMode);

		if (RGlobal::inHorrorMode) {
			auto ndh_ = getChildByIDRecursive("horror-controller");
			auto ndh = dynamic_cast<HorrorControllerNode *>(ndh_);
			
			ndh->playSound();
			ndh->resetTimer();
		}
	}

	static void endRoulette(RouletteObject *obj) {
		XPlayLayer *pl = static_cast<XPlayLayer *>(RGlobal::playLayer);

		pl->m_fields->task = pl->m_fields->_Robj->getSelectedEntry();

		pl->m_fields->blackSquare->runAction(CCFadeTo::create(0.5, 0));
		pl->scheduleOnce(schedule_selector(XPlayLayer::endRoulette2), 0.5);
	}

	void startRoulette(float delta) {
		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		bool crazyEvents = Mod::get()->getSettingValue<bool>("crazy-events");

		std::vector<std::string> values_total = {
			"3D Level", "Giant Icon", 
			"Do Nothing", "Trash", 
			"Rotated\nGameplay", 
			"Lobotomy", "Horror"
		};

		std::vector<std::string> values = {};

		std::map<int, bool> valmap = {};

		srand(time(0));

		// // get random 5 events from main list
		// int i = 0;
		// while (i < 5) {
		// 	int idx = rand() % values_total.size();

		// 	if (!valmap.count(idx)) {
		// 		valmap[idx] = true;

		// 		values.push_back(values_total.at(idx));

		// 		i++;
		// 	}
		// }
		values = values_total;

		// add additional events

		if (m_level->m_stars.value() == 0 || m_level->m_normalPercent.value() == 100) {
			bool do_default = rand() % 2;
			do_default = true;

			if (do_default) {
				values.push_back("2x Speed");
				values.push_back("0.5x Speed");
			} else {
				// for v1.1.1
				values.push_back("2x Speed");
				values.push_back("Random Speed");
			}
		} 

		if (crazyEvents) {
			values.push_back("Close Game");
		}

		auto clock = std::chrono::system_clock::now();
		auto seed = clock.time_since_epoch().count();

		auto rng = std::default_random_engine(seed);

  		std::shuffle(values.begin(), values.end(), rng);

		RouletteObject *robj = RouletteObject::create(values);

		CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.f);

		robj->setPosition(winSize / 2);

		m_fields->rouletteNode->addChild(robj, 1);

		robj->setEndCallback(XPlayLayer::endRoulette);

		m_fields->_Robj = robj;
	}

	void beginRoulette() {
		if (m_fields->rouletteStarted) return;

		RGlobal::inRoulette = true;

		// m_fields->shouldHideCursor = GameManager::sharedState()->getGameVariable("gv_0024");

		// log::debug("gv_0026 is {}", m_fields->shouldHideCursor);

		m_fields->rouletteStarted = true;

		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		auto base = CCSprite::create("square.png");
		base->setPosition({ 0, 0 });
		base->setScale(500.f);
		base->setColor({0, 0, 0});
		base->setOpacity(0);
		base->runAction(CCFadeTo::create(0.5f, 255));

		unloadPayload(false);
		
		m_fields->blackSquare = base;

		CCNode *nd1 = CCNode::create();

		nd1->addChild(base, -1);

		m_unknownE90->addChild(nd1, 9999);

		m_fields->rouletteNode = nd1;

		scheduleOnce(schedule_selector(XPlayLayer::startRoulette), 0.5f);

		unscheduleUpdate();
	}

	void onQuit() {
		unloadPayload(true);

		PlayLayer::onQuit();
	}

	void resetLevel() {
		bool softEnable = Mod::get()->getSettingValue<bool>("soft-enable");

		if (m_fields->levelStarted && !RGlobal::isEnd && softEnable) {
			// onQuit();
			beginRoulette();
		} else {
			PlayLayer::resetLevel();
			RGlobal::isEnd = false;
			RGlobal::inHorrorMode = false;
		}
	}
	void updateVisibility(float delta) {
		if (m_fields->_payloadRandomBlock) {
			addTrash(delta);
		}

		PlayLayer::updateVisibility(delta);

		m_fields->player_x_old = m_fields->player_x_new;
		m_fields->player_x_new = m_player1->getPositionX();

		m_fields->player_x_delta = m_fields->player_x_new - m_fields->player_x_old;

		if (m_fields->player_x_delta != 0.f) m_fields->levelStarted = true;
	}

	bool init(GJGameLevel *lvl, bool idk1, bool idk2) {
		RGlobal::isEnd = false;
		RGlobal::playLayer = this;
		RGlobal::inHorrorMode = false;

		m_fields->taskMapping["2x Speed"] = XPlayLayer::rouletteDoubleSpeed;
		m_fields->taskMapping["0.5x Speed"] = XPlayLayer::rouletteHalfSpeed;
		m_fields->taskMapping["Giant Icon"] = XPlayLayer::rouletteGiantPlayer;
		m_fields->taskMapping["Do Nothing"] = XPlayLayer::rouletteDoNothing;
		m_fields->taskMapping["3D Level"] = XPlayLayer::roulette3DLevel;
		m_fields->taskMapping["Trash"] = XPlayLayer::rouletteTrash;
		m_fields->taskMapping["Rotated\nGameplay"] = XPlayLayer::rouletteRotatingWorld;
		m_fields->taskMapping["Close Game"] = XPlayLayer::rouletteClose;
		m_fields->taskMapping["Lobotomy"] = XPlayLayer::rouletteLobotomy;
		m_fields->taskMapping["Horror"] = XPlayLayer::rouletteHorror;

		return PlayLayer::init(lvl, idk1, idk2);
	}
};
