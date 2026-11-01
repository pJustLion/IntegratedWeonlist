#include <Geode/Geode.hpp>

using namespace geode::prelude;

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto boton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_listBtn_001.png"),
            this,
            menu_selector(MenuLayer::onWeonList)
        );

        auto menu = this->getChildByID("bottom-menu");

        if (menu) {
            menu->addChild(boton);
            menu->updateLayout();
        }

        return true;
    }

    void onWeonList(CCObject*) {
        FLAlertLayer::create(
            "Weonlist",
            "ola y bienvenido a la weonlist",
            "Cerrar"
        )->show();
    }
};
