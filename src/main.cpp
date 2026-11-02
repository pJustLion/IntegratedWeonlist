#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/utils/file.hpp>
#include <matjson.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

using namespace geode::prelude;

class WeonListLayer : public CCLayer {
protected:
    struct Player {
        std::string name;
        std::string country;
        int points = 0;
        int rank = 0;
        matjson::Value completed;
        matjson::Value verified;
    };

    std::unordered_map<int, std::string> m_levelIDs;
    std::vector<Player> m_players;

    CCNode* m_content = nullptr;
    CCMenu* m_tabs = nullptr;

    int m_currentTab = 0;

    bool init() {
        if (!CCLayer::init()) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto bg = CCLayerColor::create({0, 0, 0, 255});
        addChild(bg, -1);

        auto title = CCLabelBMFont::create("WEONLIST", "bigFont.fnt");
        title->setPosition({win.width / 2, win.height - 35});
        title->setScale(0.8f);
        title->setOpacity(0);
        addChild(title);

        title->runAction(CCSpawn::create(
            CCEaseBackOut::create(CCScaleTo::create(0.45f, 0.8f)),
            CCFadeTo::create(0.35f, 255),
            nullptr
        ));

        auto credits = CCLabelBMFont::create(
            "Creado por: pJustLion", "chatFont.fnt"
        );
        credits->setColor({119, 119, 119});
        credits->setPosition({win.width / 2, win.height - 58});
        credits->setScale(0.7f);
        addChild(credits);

        auto closeSprite = CCSprite::createWithSpriteFrameName(
            "GJ_closeBtn_001.png"
        );

        if (closeSprite) {
            auto closeBtn = CCMenuItemSpriteExtra::create(
                closeSprite, this,
                menu_selector(WeonListLayer::onClose)
            );

            auto closeMenu = CCMenu::createWithItem(closeBtn);
            closeMenu->setPosition({win.width - 25, win.height - 25});
            addChild(closeMenu, 10);
        }

        loadPlayers();

        createTabs();
        showLevels();

        return true;
    }

    void createTabs() {
        auto win = CCDirector::sharedDirector()->getWinSize();

        auto levelsLabel = CCLabelBMFont::create(
            "LEVELS", "goldFont.fnt"
        );

        auto playersLabel = CCLabelBMFont::create(
            "LEADERBOARD", "goldFont.fnt"
        );

        levelsLabel->setScale(0.65f);
        playersLabel->setScale(0.65f);

        auto levelsBtn = CCMenuItemLabel::create(
            levelsLabel, this,
            menu_selector(WeonListLayer::onLevels)
        );

        auto playersBtn = CCMenuItemLabel::create(
            playersLabel, this,
            menu_selector(WeonListLayer::onLeaderboard)
        );

        m_tabs = CCMenu::create(levelsBtn, playersBtn, nullptr);
        m_tabs->alignItemsHorizontallyWithPadding(35);
        m_tabs->setPosition({win.width / 2, win.height - 88});

        addChild(m_tabs, 5);
    }

    void clearContent() {
        if (m_content) {
            m_content->removeFromParentAndCleanup(true);
            m_content = nullptr;
        }
    }

    void loadPlayers() {
        auto path = Mod::get()->getResourcesDir() / "players.json";
        auto data = file::readString(path);

        if (!data) {
            log::error("No se pudo leer players.json");
            return;
        }

        auto parsed = matjson::parse(data.unwrap());

        if (!parsed) {
            log::error("Error leyendo players.json");
            return;
        }

        auto players = parsed.unwrap()["players"];

        if (!players.isArray()) {
            log::error("La lista de jugadores no es un array");
            return;
        }

        for (auto const& item : players) {
            Player player;

            player.name = item["name"].asString().unwrapOr("Unknown");
            player.country = item["country"].asString().unwrapOr("Unknown");
            player.points = item["points"].asInt().unwrapOr(0);
            player.rank = item["rank"].asInt().unwrapOr(0);
            player.completed = item["completed"];
            player.verified = item["verified"];

            m_players.push_back(player);
        }

        std::sort(m_players.begin(), m_players.end(),
            [](Player const& a, Player const& b) {
                return a.rank < b.rank;
            }
        );
    }

    void showLevels() {
        m_currentTab = 0;
        clearContent();
        m_levelIDs.clear();

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto path = Mod::get()->getResourcesDir() / "levels.json";
        auto data = file::readString(path);

        if (!data) {
            log::error("No se pudo leer levels.json");
            return;
        }

        auto parsed = matjson::parse(data.unwrap());

        if (!parsed) {
            log::error("Error leyendo levels.json");
            return;
        }

        auto levels = parsed.unwrap()["levels"];

        if (!levels.isArray()) return;

        auto scroll = ScrollLayer::create({
            win.width - 50,
            win.height - 155
        });

        if (!scroll) return;

        scroll->setPosition({25, 30});
        addChild(scroll);

        m_content = scroll;

        int total = static_cast<int>(levels.size());
        float cardHeight = 58.0f;
        float gap = 8.0f;
        float totalHeight = total * (cardHeight + gap);

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(),
            totalHeight
        });

        int index = 0;

        for (auto const& level : levels) {
            int rank = level["rank"].asInt().unwrapOr(0);

            std::string name =
                level["name"].asString().unwrapOr("Sin nombre");

            std::string creator =
                level["creator"].asString().unwrapOr("Desconocido");

            std::string id = std::to_string(
                level["id"].asInt().unwrapOr(0)
            );

            std::string verifier =
                level["verifier"].asString().unwrapOr("Desconocido");

            m_levelIDs[index] = id;

            auto card = CCLayerColor::create(
                {10, 10, 10, 255},
                scroll->getContentWidth(),
                cardHeight
            );

            float y = totalHeight -
                ((index + 1) * (cardHeight + gap));

            card->setPosition({-35, y});
            card->setOpacity(0);
            scroll->m_contentLayer->addChild(card);

            card->runAction(CCSequence::create(
                CCDelayTime::create(index * 0.055f),
                CCSpawn::create(
                    CCEaseBackOut::create(
                        CCMoveTo::create(0.38f, {0, y})
                    ),
                    CCFadeTo::create(0.28f, 255),
                    nullptr
                ),
                nullptr
            ));

            auto nameLabel = CCLabelBMFont::create(
                (std::to_string(rank) + ". " + name).c_str(),
                "bigFont.fnt"
            );

            nameLabel->setAnchorPoint({0, 0.5f});
            nameLabel->setPosition({12, 43});
            nameLabel->setScale(0.45f);
            card->addChild(nameLabel);

            auto info = CCLabelBMFont::create(
                ("Creado por: " + creator + " | ID: " + id).c_str(),
                "chatFont.fnt"
            );

            info->setColor({119, 119, 119});
            info->setAnchorPoint({0, 0.5f});
            info->setPosition({12, 25});
            info->setScale(0.34f);
            card->addChild(info);

            auto copyLabel = CCLabelBMFont::create(
                "Copiar ID", "goldFont.fnt"
            );

            copyLabel->setScale(0.68f);

            auto copyBtn = CCMenuItemLabel::create(
                copyLabel, this,
                menu_selector(WeonListLayer::onCopyID)
            );

            copyBtn->setTag(index);

            auto copyMenu = CCMenu::createWithItem(copyBtn);
            copyMenu->setPosition({
                card->getContentSize().width - 52, 25
            });

            card->addChild(copyMenu, 5);

            auto verifierLabel = CCLabelBMFont::create(
                ("Verificado por: " + verifier).c_str(),
                "chatFont.fnt"
            );

            verifierLabel->setColor({119, 119, 119});
            verifierLabel->setAnchorPoint({0, 0.5f});
            verifierLabel->setPosition({12, 9});
            verifierLabel->setScale(0.36f);
            card->addChild(verifierLabel);

            index++;
        }
    }

    void showLeaderboard() {
        m_currentTab = 1;
        clearContent();

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto scroll = ScrollLayer::create({
            win.width - 50,
            win.height - 155
        });

        if (!scroll) return;

        scroll->setPosition({25, 30});
        addChild(scroll);

        m_content = scroll;

        float cardHeight = 48.0f;
        float gap = 7.0f;
        float totalHeight = m_players.size() * (cardHeight + gap);

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(),
            totalHeight
        });

        int index = 0;

        for (auto const& player : m_players) {
            auto card = CCLayerColor::create(
                {12, 12, 12, 255},
                scroll->getContentWidth(),
                cardHeight
            );

            float y = totalHeight -
                ((index + 1) * (cardHeight + gap));

            card->setPosition({0, y});
            scroll->m_contentLayer->addChild(card);

            auto name = CCLabelBMFont::create(
                (std::to_string(player.rank) + ". " +
                 player.name).c_str(),
                "bigFont.fnt"
            );

            name->setAnchorPoint({0, 0.5f});
            name->setPosition({12, 30});
            name->setScale(0.48f);
            card->addChild(name);

            auto points = CCLabelBMFont::create(
                (std::to_string(player.points) + " points").c_str(),
                "goldFont.fnt"
            );

            points->setAnchorPoint({1, 0.5f});
            points->setPosition({
                card->getContentSize().width - 12, 24
            });
            points->setScale(0.55f);
            card->addChild(points);

            auto openLabel = CCLabelBMFont::create(
                "VER PERFIL", "chatFont.fnt"
            );

            openLabel->setScale(0.42f);

            auto openBtn = CCMenuItemLabel::create(
                openLabel, this,
                menu_selector(WeonListLayer::onOpenPlayer)
            );

            openBtn->setTag(index);

            auto menu = CCMenu::createWithItem(openBtn);
            menu->setPosition({
                card->getContentSize().width - 55, 9
            });

            card->addChild(menu, 5);

            index++;
        }
    }

    void showPlayerProfile(int index) {
        if (index < 0 || index >= static_cast<int>(m_players.size()))
            return;

        auto const& player = m_players[index];

        clearContent();

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto profile = CCLayerColor::create(
            {10, 10, 10, 255},
            win.width - 50,
            win.height - 155
        );

        profile->setPosition({25, 30});
        addChild(profile);

        m_content = profile;

        auto addText = [&](std::string const& text, float y,
                           float scale, ccColor3B color) {
            auto label = CCLabelBMFont::create(
                text.c_str(), "chatFont.fnt"
            );

            label->setAnchorPoint({0, 0.5f});
            label->setPosition({15, y});
            label->setScale(scale);
            label->setColor(color);
            profile->addChild(label);
        };

        addText("PLAYER PROFILE", win.height - 185, 0.65f,
                {255, 210, 80});

        addText("Rank: #" + std::to_string(player.rank),
                win.height - 220, 0.55f, {255, 255, 255});

        addText("Player: " + player.name,
                win.height - 245, 0.55f, {255, 255, 255});

        addText("Country: " + player.country,
                win.height - 270, 0.55f, {255, 255, 255});

        addText("Points: " + std::to_string(player.points),
                win.height - 295, 0.55f, {255, 210, 80});

        float y = win.height - 335;

        addText("COMPLETED LEVELS", y, 0.55f,
                {100, 220, 255});

        y -= 22;

        if (player.completed.isArray()) {
            for (auto const& item : player.completed) {
                std::string level;
                std::string record;

                if (item.isObject()) {
                    level = item["level"].asString().unwrapOr("Unknown");
                    record = item["record"].asString().unwrapOr("");
                } else {
                    level = item.asString().unwrapOr("Unknown");
                }

                addText(
                    "- " + level +
                    (record.empty() ? "" : " (" + record + ")"),
                    y, 0.42f, {200, 200, 200}
                );

                y -= 19;
            }
        }

        y -= 8;

        addText("VERIFIED LEVELS", y, 0.55f,
                {100, 220, 255});

        y -= 22;

        if (player.verified.isArray()) {
            for (auto const& item : player.verified) {
                std::string level = item.asString().unwrapOr("Unknown");

                addText("- " + level, y, 0.42f,
                        {200, 200, 200});

                y -= 19;
            }
        }

        auto backLabel = CCLabelBMFont::create(
            "BACK", "goldFont.fnt"
        );

        backLabel->setScale(0.65f);

        auto backBtn = CCMenuItemLabel::create(
            backLabel, this,
            menu_selector(WeonListLayer::onBackProfile)
        );

        auto backMenu = CCMenu::createWithItem(backBtn);
        backMenu->setPosition({
            win.width / 2, 48
        });

        addChild(backMenu, 10);
    }

public:
    void onClose(CCObject*) {
        CCDirector::sharedDirector()->popScene();
    }

    void onLevels(CCObject*) {
        showLevels();
    }

    void onLeaderboard(CCObject*) {
        showLeaderboard();
    }

    void onOpenPlayer(CCObject* sender) {
        auto node = static_cast<CCNode*>(sender);
        showPlayerProfile(node->getTag());
    }

    void onBackProfile(CCObject*) {
        showLeaderboard();
    }

    void onCopyID(CCObject* sender) {
        auto button = static_cast<CCNode*>(sender);

        button->stopAllActions();
        button->setScale(1.0f);

        button->runAction(CCSequence::create(
            CCScaleTo::create(0.07f, 0.85f),
            CCScaleTo::create(0.12f, 1.0f),
            nullptr
        ));

        int index = button->getTag();
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
        auto scene = CCScene::create();
        scene->addChild(WeonListLayer::create());

        CCDirector::sharedDirector()->pushScene(
            CCTransitionFade::create(0.3f, scene)
        );
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = CCSprite::create("WL-Boton-lmao.png"_spr);
        if (!sprite) return true;

        sprite->setScale(2.0f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(WeonListMenuLayer::onWeonList)
        );

        auto menu = this->getChildByID("bottom-menu");

        if (menu) {
            menu->addChild(button);
            menu->updateLayout();
        }

        return true;
    }
};
