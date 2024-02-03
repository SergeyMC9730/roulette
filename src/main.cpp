#include <Geode/Geode.hpp>

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

#include <Geode/modify/CCScheduler.hpp>

#include <Geode/binding/FModAudioEngine.hpp>

#include <functional>

#include <algorithm>
#include <random>

using namespace geode::prelude;

namespace RGlobal {
	bool isEnd = false;
	void *playLayer = nullptr;
	float speed = 1.f;
	float old_fps = 1.f;
}

// class $modify(CCScheduler) {
// 	void setTimeScale(float time_scale) {
// 		CCScheduler::setTimeScale(RGlobal::speed);
// 	}
// };

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

	CCLabelBMFont *_info;

	CCNode *_selectedObject;
	CCNode *_oldObject;
	CCNode *_oldActualObject;

	CCLabelBMFont *_selectedObjectText;

	CCSprite *_nadezhdin;

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

		_info = nullptr;

		_square07 = nullptr;
		_blackLeft = nullptr;
		_blackRight = nullptr;

		_nadezhdin = nullptr;

		_scalingPowerX = 2.5;
		_scalingPowerY = 1.25;

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
		_nadezhdin->runAction(CCFadeTo::create(0.5, 0));
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

			auto line1 = CCSprite::createWithSpriteFrameName("floorLine_001.png");
			line1->setPosition({-l->getContentSize().height / 2, 0});
			line1->setID("line-start");
			lines.push_back(line1);

			auto line2 = CCSprite::createWithSpriteFrameName("floorLine_001.png");
			line2->setPosition({l->getContentSize().height / 2, 0});
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
		}

		menu->alignItemsHorizontallyWithPadding(445.f / _values.size());

		// printf("padding=%f\n", 445.f / text_sz_mid);

		square07->setContentSize({0, 0});

		// square07->runAction(CCActionTween::create(0.5, "contentSizeWidth", 0, 100));
		// square07->runAction(CCActionTween::create(0.5, "contentSizeY", 0, 50));

		scheduleUpdate();

		menu->setPosition(0, 0);
		_menu2->setPosition((menu->getContentSize().width / 1.28), 0);

		addChild(square07);;
		addChild(menu);
		addChild(_menu2);
		addChild(_blackLeft);
		addChild(_blackRight);

		// const auto& winSize = CCDirector::sharedDirector()->getWinSize();
		// menu->setPosition(winSize / 2);

		scheduleOnce(schedule_selector(RouletteObject::beginRotation), 1);

		_square07 = square07;

		_nadezhdin = CCSprite::createWithSpriteFrameName("floorLine_001.png");
		
		_nadezhdin->setRotation(90);
		_nadezhdin->setScaleX(0.2);
		// _nadezhdin->setOpacity(128);
		_nadezhdin->setColor(ccYELLOW);
		_nadezhdin->setOpacity(0);
		_nadezhdin->runAction(CCFadeTo::create(0.25f, 255));

		addChild(_nadezhdin);

		_currentMenu = _menu1;

		return true;
	}

	void setupRouletteInput() {
		this->setKeyboardEnabled(true);
	}

	void stopRoulette() {
		if (_rouletteStopping) return;

		_rouletteStopping = true;
		_info->runAction(CCFadeTo::create(0.5f, 0));
	}

	void keyDown(cocos2d::enumKeyCodes kc) override {
		CCLayer::keyDown(kc);

		if (!_infoCreated) return;

		if (kc == cocos2d::enumKeyCodes::KEY_Enter) {
			stopRoulette();
		}
	}

	void createRouletteInfo(float delta) {
		if (_infoCreated && _rotationStarted) return;

		_infoCreated = true;

		CCLabelBMFont *txt = CCLabelBMFont::create("Press Enter to stop Roulette.", "bigFont.fnt");

		txt->setPositionX(0);
		txt->setPositionY(-_square07->getContentSize().height + txt->getContentSize().height);

		txt->setScale(0.4f);

		txt->setOpacity(0);
		txt->runAction(CCFadeTo::create(0.5f, 255));

		addChild(txt);

		_info = txt;

		_selectedObjectText = CCLabelBMFont::create("(Selected entry would be here.)", "chatFont.fnt");
		_selectedObjectText->setPositionX(0);
		_selectedObjectText->setPositionY(_square07->getContentSize().height - _selectedObjectText->getContentSize().height);
		
		_selectedObjectText->setScale(1.5f);

		_selectedObjectText->setOpacity(0);
		_selectedObjectText->runAction(CCFadeTo::create(0.5f, 255));

		addChild(_selectedObjectText);

		setupRouletteInput();
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
			if (_nadezhdin->getPositionX() > (r.origin.x - r.size.width / 2) && _nadezhdin->getPositionX() < (r.origin.x + r.size.width)) {
				
				obj = children->objectAtIndex(index);
				_selectedObject = dynamic_cast<CCNode *>(obj);

				// break;
			}

			index++;
		}

		return _selectedObject != nullptr;
	}

	void update(float delta) override {
		CCNode::update(delta);

		if (_rotationStarted) {
			setupRouletteInput();
		}

		if (_rotationStarted && !_rouletteStopping && _rotationSpeed < 60.f && !_shouldForce) {
			_rotationSpeed += delta * 15;
		}

		if (_rouletteStopping) {
			_rotationSpeed -= delta * 10;

			if (_rotationSpeed < 0) _rotationSpeed = 0;
		}

		float speed = 4;

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
				// log::debug("menu 1 set!");
				_currentMenu = _menu2;
			}
			if (_menu2->getPositionX() < -(_menu2->getContentSize().width / 1.28)) {
				_menu2->setPosition((_menu1->getContentSize().width / 1.28), 0);
				// log::debug("menu 2 set!");
				_currentMenu = _menu1;
			}

			scheduleOnce(schedule_selector(RouletteObject::createRouletteInfo), 0);
		}

		// if (_rotationSpeed == 0.f) return;
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
					// _selectedObjectText->setOpacity(255);
					// _selectedObjectText->runAction(CCFadeTo::create(0.1f, 0));
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

		// printf("OBJECT AT %p\n", _selectedObject);
	}

	// void visit(void) override {
	// 	glEnable(GL_SCISSOR_TEST);

	// 	auto spos = _square07->getPosition();
	// 	auto wpos = getParent()->convertToWorldSpace(_square07->getPosition());
	// 	// auto winSize = CCDirector::sharedDirector()->getWinSize();

	// 	// CCPoint wpos = {winSize.width - _square07->getContentSize().width, winSize.height - _square07->getContentSize().height};

	// 	printf("wpos: %f %f\n", wpos.x, wpos.y);

	// 	glScissor(wpos.x, wpos.y, _square07->getContentSize().width, _square07->getContentSize().height);

	// 	CCNode::visit();

	// 	glDisable(GL_SCISSOR_TEST);
	// }
};

#include <map>



class $modify(XPlayLayer, PlayLayer) {
	float player_x_old;
	float player_x_new;
	float player_x_delta;

	bool levelStarted;
	bool rouletteStarted;

	CCNode *rouletteNode;
	CCSprite *blackSquare;

	std::string task;

	RouletteObject *_Robj;

	bool _payloadRandomBlock = false;

	float _time = 0.f;

	std::map<std::string, std::function<void(XPlayLayer *)>> taskMapping;

	void unloadPayload() {
		unschedule(schedule_selector(XPlayLayer::selectRandomMode));
		unschedule(schedule_selector(XPlayLayer::rotatingWorld));
		unschedule(schedule_selector(XPlayLayer::roulette3DWorldLoop));
		setRotation(0.f);
		m_fields->_payloadRandomBlock = false;
		
		CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.f);
		if (RGlobal::old_fps != 1.f) CCDirector::sharedDirector()->setAnimationInterval(RGlobal::old_fps);

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
	static void rouletteGiantPlayer(XPlayLayer *pl) {
		pl->m_player1->setScale(3.f);
	}
	static void rouletteLobotomy(XPlayLayer *pl) {
		log::debug("WIP");
	}
	static void roulette5FPS(XPlayLayer *pl) {
		RGlobal::old_fps = CCDirector::sharedDirector()->getSecondsPerFrame();

		CCDirector::sharedDirector()->setAnimationInterval(1.f / 5.f);
	}

	void selectRandomModeFly() {
		std::vector<int> values = {
			5, 0x13, 0x29, 0x40
		};
		std::map<int, int> keys = {
			{5, 13}, {0x13, 111}, 
			{0x29, 1933}, {0x40, 660}
		};

		int val = values[rand() % values.size()];

		// log::debug("asking for {} (id={})", val, keys[val]);

		GameObject *obj = GameObject::createWithKey(keys[val]);
		obj->setPosition(m_player1->getPosition());

		addObject(obj);

		switchToFlyMode(m_player1, obj, true, val);
	}
	void selectRandomMode(float delta) {
		int val = rand() % 4;

		std::map<int, int> keys = {
			{1, 748}, {2, 47}, {3, 1331}
		};

		GameObject *obj = nullptr;
		if (val != 0 && val != 1) {
			// log::debug("asking for {} (id={})", val, keys[val]);

			obj = GameObject::createWithKey(keys[val]);
			obj->setPosition(m_player1->getPosition());

			addObject(obj);
		}

		switch(val) {
			default:
			// case 0: {
			// 	selectRandomModeFly();
			// 	break;
			// }
			case 1: {
				switchToRobotMode(m_player1, obj, true);
				break;
			}
			case 2: {
				switchToRollMode(m_player1, obj, true);
				break;
			}
			case 3: {
				switchToSpiderMode(m_player1, obj, true);
				break;
			}
		}

		selectRandomModeFly();
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

	static void rouletteModeEverySecond(XPlayLayer *pl) {
		pl->schedule(schedule_selector(XPlayLayer::selectRandomMode), 1.f);
	}
	static void rouletteTrash(XPlayLayer *pl) {
		pl->m_fields->_payloadRandomBlock = true;
	}

	void levelComplete() {
		unloadPayload();
		
		RGlobal::speed = 1;
		RGlobal::isEnd = true;

		PlayLayer::levelComplete();
	}

	void endRoulette2(float delta) {
		m_fields->rouletteNode->removeMeAndCleanup();

		// log::debug("selected entry is {}", m_fields->task);

		auto func = m_fields->taskMapping[m_fields->task];

		m_fields->levelStarted = false;
		RGlobal::isEnd = false;
		m_fields->rouletteStarted = false;

		scheduleUpdate();

		resetLevel();

		func(this);

		m_fields->levelStarted = true;

		CCDirector::sharedDirector()->getScheduler()->setTimeScale(RGlobal::speed);
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

		std::vector<std::string> values_old = {
			"2x Speed", "3D Level", "Giant Icon", "0.5x Speed", "Do Nothing", "Trash", "Rotated\nGameplay"
		};

		if (crazyEvents) {
			values_old.push_back("Close the\nGame");
			values_old.push_back("Random Mode\nEvery Sec");
		}

		std::vector<std::string> values = {};
		std::vector<bool> values_took = {};

		srand(time(0));

		values_took.reserve(values_old.size());

		for (size_t i = 0; i < values_old.size(); i++) {
			int max_iters = values_old.size();

			for (int j = 0; j < max_iters; j++) {
				size_t idx = rand() % values_old.size();
		
				if (!values_took[idx]) {
					values_took[idx] = true;
					values.push_back(values_old[idx]);

					break;
				}
			}
		}
		// std::vector<std::string> values = {
		// 	"Random Mode\nEvery Sec", "Random Mode\nEvery Sec", "Random Mode\nEvery Sec"
		// };

		// auto rng = std::default_random_engine {(unsigned int)time(0)};
		// std::shuffle(std::begin(values), std::end(values), rng);

		RouletteObject *robj = RouletteObject::create(values);

		CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.f);

		// RouletteObject *robj = RouletteObject::create({
		// 	"2x Speed", "2x Speed", "2x Speed", "2x Speed", "2x Speed", "2x Speed", "2x Speed"
		// });

		robj->setPosition(winSize / 2);

		m_fields->rouletteNode->addChild(robj, 1);

		robj->setEndCallback(XPlayLayer::endRoulette);

		m_fields->_Robj = robj;
	}

	void beginRoulette() {
		if (m_fields->rouletteStarted) return;

		m_fields->rouletteStarted = true;

		const auto& winSize = CCDirector::sharedDirector()->getWinSize();

		auto base = CCSprite::create("square.png");
		base->setPosition({ 0, 0 });
		base->setScale(500.f);
		base->setColor({0, 0, 0});
		base->setOpacity(0);
		base->runAction(CCFadeTo::create(0.5f, 255));

		unloadPayload();
		
		m_fields->blackSquare = base;

		CCNode *nd1 = CCNode::create();

		nd1->addChild(base, -1);

		m_unknownE90->addChild(nd1, 9999);

		m_fields->rouletteNode = nd1;

		scheduleOnce(schedule_selector(XPlayLayer::startRoulette), 0.5f);

		unscheduleUpdate();
	}

	void onQuit() {
		unloadPayload();

		PlayLayer::onQuit();
	}

	void resetLevel() {
		if (m_fields->levelStarted && !RGlobal::isEnd) {
			// onQuit();
			beginRoulette();
		} else {
			PlayLayer::resetLevel();
			RGlobal::isEnd = false;
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

	// "2x Speed", "0.5x Speed", "Giant Icon", "5 FPS", "Lobotomy", "Do Nothing"
	bool init(GJGameLevel *lvl, bool idk1, bool idk2) {
		RGlobal::isEnd = false;
		RGlobal::playLayer = this;

		m_fields->taskMapping["2x Speed"] = XPlayLayer::rouletteDoubleSpeed;
		m_fields->taskMapping["0.5x Speed"] = XPlayLayer::rouletteHalfSpeed;
		m_fields->taskMapping["Giant Icon"] = XPlayLayer::rouletteGiantPlayer;
		m_fields->taskMapping["5 FPS"] = XPlayLayer::roulette5FPS;
		m_fields->taskMapping["Lobotomy"] = XPlayLayer::rouletteLobotomy;
		m_fields->taskMapping["Do Nothing"] = XPlayLayer::rouletteDoNothing;
		m_fields->taskMapping["3D Level"] = XPlayLayer::roulette3DLevel;
		m_fields->taskMapping["Random Mode\nEvery Sec"] = XPlayLayer::rouletteModeEverySecond;
		m_fields->taskMapping["Trash"] = XPlayLayer::rouletteTrash;
		m_fields->taskMapping["Rotated\nGameplay"] = XPlayLayer::rouletteRotatingWorld;
		m_fields->taskMapping["Close the\nGame"] = XPlayLayer::rouletteClose;

		return PlayLayer::init(lvl, idk1, idk2);
	}
};
