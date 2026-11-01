
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/file.hpp>
#include <matjson.hpp>
#include <unordered_map>

using namespace geode::prelude;

class WeonListLayer : public CCLayer {
protected:
    std::unordered_map<int, std::string> m_levelIDs;

    bool init() {
        if (!CCLayer::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto fondo = CCLayerColor::create({0, 0, 0, 255});
        this->addChild(fondo, -1);

        // Titulo
        auto titulo = CCLabelBMFont::create("WEONLIST", "bigFont.fnt");
        titulo->setPosition({winSize.width / 2, winSize.height - 35});
        titulo->setScale(0.8f);
        this->addChild(titulo);

        // Creditos
        auto creditos = CCLabelBMFont::create(
            "Creado por: pJustLion", "chatFont.fnt"
        );
        creditos->setColor({119, 119, 119});
        creditos->setPosition({winSize.width / 2, winSize.height - 58});
        creditos->setScale(0.7f);
        this->addChild(creditos);

        // Boton cerrar
        auto cerrarSprite = CCSprite::createWithSpriteFrameName(
            "GJ_closeBtn_001.png"
        );

        if (cerrarSprite) {
            auto cerrarBoton = CCMenuItemSpriteExtra::create(
                cerrarSprite, this,
                menu_selector(WeonListLayer::onClose)
            );

            auto menuCerrar = CCMenu::createWithItem(cerrarBoton);
            menuCerrar->setPosition({
                winSize.width - 25, winSize.height - 25
            });
            this->addChild(menuCerrar, 10);
        }

        // Leer JSON
        auto ruta = Mod::get()->getResourcesDir() / "levels.json";
        auto datos = file::readString(ruta);

        if (!datos) {
            log::error("No se pudo leer levels.json");
            return true;
        }

        auto json = matjson::parse(datos.unwrap());

        if (!json) {
            log::error("Error leyendo JSON de WeonList");
            return true;
        }

        auto niveles = json.unwrap()["levels"];

        if (!niveles.isArray()) {
            log::error("La lista de niveles no es un array");
            return true;
        }

        // Crear scroll
        auto scroll = ScrollLayer::create({
            winSize.width - 50,
            winSize.height - 145
        });

        if (!scroll) return true;

        scroll->setPosition({25, 35});
        this->addChild(scroll);

        int totalNiveles = static_cast<int>(niveles.size());
        float alturaTarjeta = 65.0f;
        float separacion = 10.0f;
        float alturaTotal = totalNiveles * (alturaTarjeta + separacion);

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(),
            alturaTotal
        });

        int index = 0;

        for (auto const& nivel : niveles) {
            int rank = nivel["rank"].asInt().unwrapOr(0);

            std::string nombre =
                nivel["name"].asString().unwrapOr("Sin nombre");

            std::string creador =
                nivel["creator"].asString().unwrapOr("Desconocido");

            std::string id = std::to_string(
                nivel["id"].asInt().unwrapOr(0)
            );

            std::string verificador =
                nivel["verifier"].asString().unwrapOr("Desconocido");

            m_levelIDs[index] = id;

            // Tarjeta
            auto tarjeta = CCLayerColor::create(
                {10, 10, 10, 255},
                scroll->getContentWidth(),
                alturaTarjeta
            );

            float y = alturaTotal -
                ((index + 1) * (alturaTarjeta + separacion));

            tarjeta->setPosition({0, y});
            scroll->m_contentLayer->addChild(tarjeta);

            // Nombre
            auto nombreLabel = CCLabelBMFont::create(
                (std::to_string(rank) + ". " + nombre).c_str(),
                "bigFont.fnt"
            );

            nombreLabel->setAnchorPoint({0, 0.5f});
            nombreLabel->setPosition({12, 45});
            nombreLabel->setScale(0.45f);
            tarjeta->addChild(nombreLabel);

            // Informacion
            auto info = CCLabelBMFont::create(
                ("Creado por: " + creador + " | ID: " + id).c_str(),
                "chatFont.fnt"
            );

            info->setColor({119, 119, 119});
            info->setAnchorPoint({0, 0.5f});
            info->setPosition({12, 25});
            info->setScale(0.38f);
            tarjeta->addChild(info);

            // Boton copiar ID
            auto copiarLabel = CCLabelBMFont::create(
                "Copiar ID", "goldFont.fnt"
            );

            copiarLabel->setScale(0.75f);

            auto copiarBoton = CCMenuItemLabel::create(
                copiarLabel, this,
                menu_selector(WeonListLayer::onCopyID)
            );

            copiarBoton->setTag(index);

            auto copiarMenu = CCMenu::createWithItem(copiarBoton);
            copiarMenu->setPosition({
                tarjeta->getContentSize().width - 58,
                25
            });

            tarjeta->addChild(copiarMenu, 5);

            // Verificador
            auto verificadorLabel = CCLabelBMFont::create(
                ("Verificado por: " + verificador).c_str(),
                "chatFont.fnt"
            );

            verificadorLabel->setColor({119, 119, 119});
            verificadorLabel->setAnchorPoint({0, 0.5f});
            verificadorLabel->setPosition({12, 9});
            verificadorLabel->setScale(0.4f);
            tarjeta->addChild(verificadorLabel);

            index++;
        }

        return true;
    }

public:
    void onClose(CCObject*) {
        CCDirector::sharedDirector()->popScene();
    }

    void onCopyID(CCObject* sender) {
        auto boton = static_cast<CCNode*>(sender);
        int index = boton->getTag();

        auto it = m_levelIDs.find(index);

        if (it != m_levelIDs.end()) {
            if (utils::clipboard::write(it->second)) {
                FLAlertLayer::create(
                    "WeonList",
                    "ID copiado al portapapeles!",
                    "OK"
                )->show();
            }
        }
    }

    static WeonListLayer* create() {
        auto ret = new WeonListLayer();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(WeonListMenuLayer, MenuLayer) {
    void onWeonList(CCObject*) {
        auto escena = CCScene::create();
        escena->addChild(WeonListLayer::create());

        CCDirector::sharedDirector()->pushScene(
            CCTransitionFade::create(0.3f, escena)
        );
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = CCSprite::create("WL-Boton-lmao.png"_spr);
        if (!sprite) return true;

        sprite->setScale(2.0f);

        auto boton = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(WeonListMenuLayer::onWeonList)
        );

        auto menu = this->getChildByID("bottom-menu");

        if (menu) {
            menu->addChild(boton);
            menu->updateLayout();
        }

        return true;
    }
};
