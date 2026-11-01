
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto sprite = CCSprite::create(
            "WL-Boton-lmao.png"_spr
        );

        if (!sprite) {
            log::error("No se pudo cargar el boton de WeonList");
            return true;
        }

        auto boton = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(MenuLayer::onWeonList)
        );

        boton->setScale(0.8f);

        auto menu = this->getChildByID("bottom-menu");

        if (menu) {
            menu->addChild(boton);
            menu->updateLayout();
        }

        return true;
    }

    void onWeonList(CCObject*) {
        FLAlertLayer::create(
            "WeonList",
            "ola y bienvenido a la WeonList",
            "Cerrar"
        )->show();
    }
};
