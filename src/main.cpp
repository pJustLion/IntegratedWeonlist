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
    CCMenu* m_backMenu = nullptr;

    int m_currentTab = 0;

    const ccColor3B GOLD = {235, 190, 75};
    const ccColor3B LIGHT_GOLD = {255, 220, 125};
    const ccColor3B GRAY = {145, 145, 145};
    const ccColor3B WHITE = {235, 235, 235};
    const ccColor3B CYAN = {100, 195, 220};

    bool init() {
        if (!CCLayer::init()) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto bg = CCLayerColor::create({5, 5, 7, 255});
        addChild(bg, -10);

        auto topLine = CCLayerColor::create(
            {GOLD.r, GOLD.g, GOLD.b, 255},
            win.width, 2
        );
        topLine->setPosition({0, win.height - 76});
        addChild(topLine, -1);

        auto title = CCLabelBMFont::create(
            "WEONLIST", "bigFont.fnt"
        );
        title->setPosition({win.width / 2, win.height - 31});
        title->setScale(0.85f);
        title->setColor(LIGHT_GOLD);
        title->setOpacity(0);
        addChild(title);

        title->runAction(CCSpawn::create(
            CCEaseBackOut::create(
                CCScaleTo::create(0.45f, 0.85f)
            ),
            CCFadeTo::create(0.35f, 255),
            nullptr
        ));

        auto credits = CCLabelBMFont::create(
            "UNA DEMONLIST DE LA COMUNIDAD",
            "chatFont.fnt"
        );
        credits->setColor(GRAY);
        credits->setPosition({win.width / 2, win.height - 52});
        credits->setScale(0.48f);
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
            closeMenu->setPosition({
                win.width - 25, win.height - 25
            });
            addChild(closeMenu, 20);
        }

        loadPlayers();
        createTabs();
        showLevels();

        return true;
    }

    void createTabs() {
        auto win = CCDirector::sharedDirector()->getWinSize();

        auto levelsLabel = CCLabelBMFont::create(
            "NIVELES", "goldFont.fnt"
        );

        auto playersLabel = CCLabelBMFont::create(
            "CLASIFICACION", "goldFont.fnt"
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

        m_tabs = CCMenu::create(
            levelsBtn, playersBtn, nullptr
        );

        m_tabs->alignItemsHorizontallyWithPadding(30);
        m_tabs->setPosition({
            win.width / 2, win.height - 96
        });

        addChild(m_tabs, 5);
    }

    void clearContent() {
        if (m_content) {
            m_content->removeFromParentAndCleanup(true);
            m_content = nullptr;
        }

        if (m_backMenu) {
            m_backMenu->removeFromParentAndCleanup(true);
            m_backMenu = nullptr;
        }
    }

    void addAccent(
        CCNode* parent,
        float x,
        float y,
        float width,
        float height,
        ccColor3B color
    ) {
        auto accent = CCLayerColor::create(
            {color.r, color.g, color.b, 255},
            width, height
        );
        accent->setPosition({x, y});
        parent->addChild(accent, 2);
    }

    CCLabelBMFont* makeLabel(
        std::string const& text,
        char const* font,
        float scale,
        ccColor3B color
    ) {
        auto label = CCLabelBMFont::create(text.c_str(), font);
        label->setScale(scale);
        label->setColor(color);
        return label;
    }

    void loadPlayers() {
        m_players.clear();

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

            player.name =
                item["name"].asString().unwrapOr("Desconocido");

            player.country =
                item["country"].asString().unwrapOr("Desconocido");

            player.points =
                item["points"].asInt().unwrapOr(0);

            player.rank =
                item["rank"].asInt().unwrapOr(0);

            player.completed = item["completed"];
            player.verified = item["verified"];

            m_players.push_back(player);
        }

        std::sort(
            m_players.begin(), m_players.end(),
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
        addChild(scroll, 1);
        m_content = scroll;

        int total = static_cast<int>(levels.size());
        float cardHeight = 72;
        float gap = 9;
        float contentHeight = total * (cardHeight + gap) + 10;

        auto content = scroll->m_contentLayer;
        content->setContentSize({
            win.width - 50,
            contentHeight
        });

        int index = 0;

        for (auto const& level : levels) {
            int rank = level["rank"].asInt().unwrapOr(0);
            std::string name =
                level["name"].asString().unwrapOr("Sin nombre");
            std::string creator =
                level["creator"].asString().unwrapOr("Desconocido");
            int id = level["id"].asInt().unwrapOr(0);
            std::string verifier =
                level["verifier"].asString().unwrapOr("Desconocido");

            m_levelIDs[id] = name;

            auto card = CCLayerColor::create(
                {20, 20, 24, 255},
                win.width - 70,
                cardHeight
            );

            float y = contentHeight - (index + 1) * (cardHeight + gap);
            card->setPosition({10, y});
            content->addChild(card);

            ccColor3B rankColor = WHITE;
            if (rank == 1) rankColor = LIGHT_GOLD;
            else if (rank == 2) rankColor = {190, 195, 205};
            else if (rank == 3) rankColor = {190, 125, 75};

            auto rankLabel = makeLabel(
                "#" + std::to_string(rank),
                "goldFont.fnt",
                0.65f,
                rankColor
            );
            rankLabel->setAnchorPoint({0, 0.5f});
            rankLabel->setPosition({12, cardHeight / 2});
            card->addChild(rankLabel);

            auto nameLabel = makeLabel(
                name,
                "bigFont.fnt",
                0.48f,
                WHITE
            );
            nameLabel->setAnchorPoint({0, 0.5f});
            nameLabel->setPosition({65, 48});
            card->addChild(nameLabel);

            auto creatorLabel = makeLabel(
                "Creado por: " + creator,
                "chatFont.fnt",
                0.48f,
                GRAY
            );
            creatorLabel->setAnchorPoint({0, 0.5f});
            creatorLabel->setPosition({65, 27});
            card->addChild(creatorLabel);

            auto verifierLabel = makeLabel(
                "Verificado por: " + verifier,
                "chatFont.fnt",
                0.42f,
                CYAN
            );
            verifierLabel->setAnchorPoint({0, 0.5f});
            verifierLabel->setPosition({65, 10});
            card->addChild(verifierLabel);

            auto idLabel = makeLabel(
                "ID: " + std::to_string(id),
                "chatFont.fnt",
                0.43f,
                GRAY
            );
            idLabel->setAnchorPoint({1, 0.5f});
            idLabel->setPosition({
                card->getContentSize().width - 12,
                49
            });
            card->addChild(idLabel);

            auto copyLabel = makeLabel(
                "COPIAR ID",
                "goldFont.fnt",
                0.48f,
                GOLD
            );

            auto copyBtn = CCMenuItemLabel::create(
                copyLabel, this,
                menu_selector(WeonListLayer::onCopyID)
            );

            copyBtn->setTag(id);

            auto copyMenu = CCMenu::createWithItem(copyBtn);
            copyMenu->setPosition({
                card->getContentSize().width - 55,
                20
            });

            card->addChild(copyMenu, 5);

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
        addChild(scroll, 1);
        m_content = scroll;

        float cardHeight = 65;
        float gap = 9;
        float contentHeight =
            m_players.size() * (cardHeight + gap) + 10;

        auto content = scroll->m_contentLayer;
        content->setContentSize({
            win.width - 50,
            contentHeight
        });

        int index = 0;

        for (auto const& player : m_players) {
            auto card = CCLayerColor::create(
                {20, 20, 24, 255},
                win.width - 70,
                cardHeight
            );

            float y = contentHeight - (index + 1) * (cardHeight + gap);
            card->setPosition({10, y});
            content->addChild(card);

            ccColor3B rankColor = WHITE;
            if (player.rank == 1) rankColor = LIGHT_GOLD;
            else if (player.rank == 2) rankColor = {190, 195, 205};
            else if (player.rank == 3) rankColor = {190, 125, 75};

            auto rankLabel = makeLabel(
                "#" + std::to_string(player.rank),
                "goldFont.fnt",
                0.65f,
                rankColor
            );
            rankLabel->setAnchorPoint({0, 0.5f});
            rankLabel->setPosition({12, cardHeight / 2});
            card->addChild(rankLabel);

            auto nameLabel = makeLabel(
                player.name,
                "bigFont.fnt",
                0.52f,
                WHITE
            );
            nameLabel->setAnchorPoint({0, 0.5f});
            nameLabel->setPosition({65, 42});
            card->addChild(nameLabel);

            auto pointsLabel = makeLabel(
                std::to_string(player.points) + " puntos",
                "chatFont.fnt",
                0.5f,
                GOLD
            );
            pointsLabel->setAnchorPoint({0, 0.5f});
            pointsLabel->setPosition({65, 18});
            card->addChild(pointsLabel);

            auto profileLabel = makeLabel(
                "VER PERFIL",
                "goldFont.fnt",
                0.48f,
                CYAN
            );

            auto profileBtn = CCMenuItemLabel::create(
                profileLabel, this,
                menu_selector(WeonListLayer::onViewProfile)
            );

            profileBtn->setTag(index);

            auto profileMenu = CCMenu::createWithItem(profileBtn);
            profileMenu->setPosition({
                card->getContentSize().width - 60,
                cardHeight / 2
            });

            card->addChild(profileMenu, 5);

            index++;
        }
    }

    void showPlayerProfile(int playerIndex) {
        if (playerIndex < 0 ||
            playerIndex >= static_cast<int>(m_players.size())) {
            return;
        }

        clearContent();

        auto win = CCDirector::sharedDirector()->getWinSize();
        auto const& player = m_players[playerIndex];

        auto scroll = ScrollLayer::create({
            win.width - 50,
            win.height - 190
        });

        if (!scroll) return;

        scroll->setPosition({25, 65});
        addChild(scroll, 1);
        m_content = scroll;

        float width = win.width - 50;
        float y = 0;

        int completedCount = player.completed.isArray()
            ? static_cast<int>(player.completed.size()) : 0;

        int verifiedCount = player.verified.isArray()
            ? static_cast<int>(player.verified.size()) : 0;

        float contentHeight =
            190 + completedCount * 30 + verifiedCount * 30;

        auto content = scroll->m_contentLayer;
        content->setContentSize({width, contentHeight});

        auto profileTitle = makeLabel(
            "PERFIL DEL JUGADOR",
            "goldFont.fnt",
            0.7f,
            LIGHT_GOLD
        );
        profileTitle->setPosition({
            width / 2,
            contentHeight - 25
        });
        content->addChild(profileTitle);

        auto rankLabel = makeLabel(
            "Puesto #" + std::to_string(player.rank),
            "bigFont.fnt",
            0.65f,
            GOLD
        );
        rankLabel->setPosition({
            width / 2,
            contentHeight - 60
        });
        content->addChild(rankLabel);

        auto nameLabel = makeLabel(
            player.name,
            "bigFont.fnt",
            0.75f,
            WHITE
        );
        nameLabel->setPosition({
            width / 2,
            contentHeight - 95
        });
        content->addChild(nameLabel);

        auto countryLabel = makeLabel(
            "Pais: " + player.country,
            "chatFont.fnt",
            0.55f,
            GRAY
        );
        countryLabel->setPosition({
            width / 2,
            contentHeight - 125
        });
        content->addChild(countryLabel);

        auto pointsLabel = makeLabel(
            "Puntos: " + std::to_string(player.points),
            "goldFont.fnt",
            0.58f,
            GOLD
        );
        pointsLabel->setPosition({
            width / 2,
            contentHeight - 155
        });
        content->addChild(pointsLabel);

        y = contentHeight - 195;

        auto completedTitle = makeLabel(
            "NIVELES COMPLETADOS",
            "goldFont.fnt",
            0.55f,
            LIGHT_GOLD
        );
        completedTitle->setAnchorPoint({0, 0.5f});
        completedTitle->setPosition({15, y});
        content->addChild(completedTitle);

        y -= 30;

        if (player.completed.isArray()) {
            for (auto const& item : player.completed) {
                std::string level =
                    item["level"].asString().unwrapOr("Desconocido");

                std::string record =
                    item["record"].asString().unwrapOr("Sin record");

                auto label = makeLabel(
                    level + " - " + record,
                    "chatFont.fnt",
                    0.48f,
                    WHITE
                );
                label->setAnchorPoint({0, 0.5f});
                label->setPosition({20, y});
                content->addChild(label);

                y -= 30;
            }
        }

        y -= 10;

        auto verifiedTitle = makeLabel(
            "NIVELES VERIFICADOS",
            "goldFont.fnt",
            0.55f,
            LIGHT_GOLD
        );
        verifiedTitle->setAnchorPoint({0, 0.5f});
        verifiedTitle->setPosition({15, y});
        content->addChild(verifiedTitle);

        y -= 30;

        if (player.verified.isArray()) {
            for (auto const& item : player.verified) {
                std::string level =
                    item.asString().unwrapOr("Desconocido");

                auto label = makeLabel(
                    level,
                    "chatFont.fnt",
                    0.48f,
                    CYAN
                );
                label->setAnchorPoint({0, 0.5f});
                label->setPosition({20, y});
                content->addChild(label);

                y -= 30;
            }
        }

        auto backLabel = makeLabel(
            "VOLVER",
            "goldFont.fnt",
            0.65f,
            LIGHT_GOLD
        );

        auto backBtn = CCMenuItemLabel::create(
            backLabel, this,
            menu_selector(WeonListLayer::onBack)
        );

        m_backMenu = CCMenu::createWithItem(backBtn);
        m_backMenu->setPosition({
            win.width / 2,
            35
        });

        addChild(m_backMenu, 10);
    }

    void onClose(CCObject*) {
        removeFromParentAndCleanup(true);
    }

    void onLevels(CCObject*) {
        showLevels();
    }

    void onLeaderboard(CCObject*) {
        showLeaderboard();
    }

    void onViewProfile(CCObject* sender) {
        auto item = static_cast<CCMenuItem*>(sender);
        showPlayerProfile(item->getTag());
    }

    void onBack(CCObject*) {
        showLeaderboard();
    }

    void onCopyID(CCObject* sender) {
        auto item = static_cast<CCMenuItem*>(sender);
        int id = item->getTag();

        utils::clipboard::write(std::to_string(id));

        Notification::create(
            "ID copiado al portapapeles",
            NotificationIcon::Success,
            1.5f
        )->show();
    }

public:
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

class WeonListMenuLayer : public CCNode {
public:
    void onWeonList(CCObject*) {
        auto layer = WeonListLayer::create();

        if (layer) {
            auto scene = CCScene::create();
            scene->addChild(layer);
            CCDirector::sharedDirector()->pushScene(
                CCTransitionFade::create(0.3f, scene)
            );
        }
    }
};

class $modify(WeonListMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = CCSprite::create(
            "WL-Boton-lmao.png"
        );

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
};o copyLabel = makeLabel(
                "COPIAR ID", "goldFont.fnt", 0.48f, GOLD
            );

            auto copyBtn = CCMenuItemLabel::create(
                copyLabel, this,
                menu_selector(WeonListLayer::onCopyID)
            );

            copyBtn->setTag(index);

            auto copyMenu = CCMenu::createWithItem(copyBtn);
            copyMenu->setPosition({
                card->getContentSize().width - 55, 22
            });
            card->addChild(copyMenu, 5);

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
        addChild(scroll, 1);
        m_content = scroll;

        float cardHeight = 58.0f;
        float gap = 8.0f;
        float totalHeight =
            m_players.size() * (cardHeight + gap);

        totalHeight = std::max(
            totalHeight, win.height - 155
        );

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(), totalHeight
        });

        int index = 0;

        for (auto const& player : m_players) {
            auto card = CCLayerColor::create(
                {15, 15, 18, 255},
                scroll->getContentWidth() - 4,
                cardHeight
            );

            float y = totalHeight -
                ((index + 1) * (cardHeight + gap));

            card->setPosition({0, y});
            scroll->m_contentLayer->addChild(card);

            ccColor3B rankColor = WHITE;

            if (player.rank == 1) rankColor = LIGHT_GOLD;
            else if (player.rank == 2) rankColor = {190, 200, 215};
            else if (player.rank == 3) rankColor = {205, 145, 95};

            addAccent(card, 0, 0, 3, cardHeight, rankColor);

            auto rank = makeLabel(
                "#" + std::to_string(player.rank),
                "goldFont.fnt", 0.58f, rankColor
            );
            rank->setAnchorPoint({0, 0.5f});
            rank->setPosition({12, 36});
            card->addChild(rank);

            auto name = makeLabel(
                player.name, "bigFont.fnt", 0.47f, WHITE
            );
            name->setAnchorPoint({0, 0.5f});
            name->setPosition({48, 36});
            card->addChild(name);

            auto country = makeLabel(
                player.country, "chatFont.fnt", 0.37f, GRAY
            );
            country->setAnchorPoint({0, 0.5f});
            country->setPosition({48, 15});
            card->addChild(country);

            auto points = makeLabel(
                std::to_string(player.points) + " PTS",
                "goldFont.fnt", 0.48f, GOLD
            );
            points->setAnchorPoint({1, 0.5f});
            points->setPosition({
                card->getContentSize().width - 12, 38
            });
            card->addChild(points);

            auto openLabel = makeLabel(
                "VER PERFIL", "chatFont.fnt", 0.40f, CYAN
            );

            auto openBtn = CCMenuItemLabel::create(
                openLabel, this,
                menu_selector(WeonListLayer::onOpenPlayer)
            );

            openBtn->setTag(index);

            auto menu = CCMenu::createWithItem(openBtn);
            menu->setPosition({
                card->getContentSize().width - 55, 14
            });
            card->addChild(menu, 5);

            index++;
        }
    }

    void showPlayerProfile(int index) {
        if (index < 0 ||
            index >= static_cast<int>(m_players.size())) {
            return;
        }

        clearContent();

        auto const& player = m_players[index];
        auto win = CCDirector::sharedDirector()->getWinSize();

        float panelWidth = win.width - 50;
        float panelHeight = win.height - 155;

        auto scroll = ScrollLayer::create({
            panelWidth, panelHeight
        });

        if (!scroll) return;

        scroll->setPosition({25, 30});
        addChild(scroll, 2);
        m_content = scroll;

        // Altura calculada según la cantidad de registros
        int completedCount = player.completed.isArray()
            ? static_cast<int>(player.completed.size()) : 0;

        int verifiedCount = player.verified.isArray()
            ? static_cast<int>(player.verified.size()) : 0;

        float totalHeight = 360.0f +
            completedCount * 22.0f +
            verifiedCount * 22.0f;

        totalHeight = std::max(totalHeight, panelHeight);

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(), totalHeight
        });

        auto content = scroll->m_contentLayer;

        auto addText = [&](std::string const& text,
                           float x, float y,
                           float scale, ccColor3B color) {
            auto label = CCLabelBMFont::create(
                text.c_str(), "chatFont.fnt"
            );

            label->setAnchorPoint({0, 0.5f});
            label->setPosition({x, y});
            label->setScale(scale);
            label->setColor(color);
            content->addChild(label);
        };

        auto header = makeLabel(
            "PERFIL DEL JUGADOR",
            "goldFont.fnt", 0.68f, LIGHT_GOLD
        );
        header->setAnchorPoint({0, 0.5f});
        header->setPosition({16, totalHeight - 35});
        content->addChild(header);

        addAccent(
            content, 15, totalHeight - 57,
            panelWidth - 30, 2, GOLD
        );

        addText(
            "PUESTO: #" + std::to_string(player.rank),
            16, totalHeight - 82, 0.52f, LIGHT_GOLD
        );

        addText(
            "JUGADOR: " + player.name,
            16, totalHeight - 108, 0.48f, WHITE
        );

        addText(
            "PAIS: " + player.country,
            16, totalHeight - 134, 0.48f, WHITE
        );

        addText(
            "PUNTOS: " + std::to_string(player.points),
            16, totalHeight - 160, 0.48f, GOLD
        );

        float y = totalHeight - 202;

        addText(
            "NIVELES COMPLETADOS",
            16, y, 0.52f, CYAN
        );
        y -= 25;

        if (player.completed.isArray() &&
            player.completed.size() > 0) {

            for (auto const& item : player.completed) {
                std::string level;
                std::string record;

                if (item.isObject()) {
                    level = item["level"].asString()
                        .unwrapOr("Desconocido");

                    record = item["record"].asString()
                        .unwrapOr("");
                } else {
                    level = item.asString()
                        .unwrapOr("Desconocido");
                }

                addText(
                    "- " + level +
                    (record.empty() ? "" : " (" + record + ")"),
                    20, y, 0.39f, WHITE
                );

                y -= 22;
            }
        } else {
            addText(
                "Sin niveles registrados",
                20, y, 0.39f, GRAY
            );
            y -= 22;
        }

        y -= 12;

        addText(
            "NIVELES VERIFICADOS",
            16, y, 0.52f, CYAN
        );
        y -= 25;

        if (player.verified.isArray() &&
            player.verified.size() > 0) {

            for (auto const& item : player.verified) {
                std::string level =
                    item.asString().unwrapOr("Desconocido");

                addText(
                    "- " + level,
                    20, y, 0.39f, WHITE
                );

                y -= 22;
            }
        } else {
            addText(
                "Sin niveles verificados",
                20, y, 0.39f, GRAY
            );
        }

        // Botón fijo fuera del ScrollLayer
        auto backLabel = makeLabel(
            "VOLVER", "goldFont.fnt", 0.65f, LIGHT_GOLD
        );

        auto backBtn = CCMenuItemLabel::create(
            backLabel, this,
            menu_selector(WeonListLayer::onBackProfile)
        );

        m_backMenu = CCMenu::createWithItem(backBtn);
        m_backMenu->setPosition({
            win.width / 2, 13
        });

        addChild(m_backMenu, 20);
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

        auto sprite = CCSprite::create(
            "WL-Boton-lmao.png"_spr
        );

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
o copyLabel = makeLabel(
                "COPIAR ID", "goldFont.fnt", 0.48f, GOLD
            );

            auto copyBtn = CCMenuItemLabel::create(
                copyLabel, this,
                menu_selector(WeonListLayer::onCopyID)
            );

            copyBtn->setTag(index);

            auto copyMenu = CCMenu::createWithItem(copyBtn);
            copyMenu->setPosition({
                card->getContentSize().width - 55, 22
            });
            card->addChild(copyMenu, 5);

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
        addChild(scroll, 1);
        m_content = scroll;

        float cardHeight = 58.0f;
        float gap = 8.0f;
        float totalHeight =
            m_players.size() * (cardHeight + gap);

        totalHeight = std::max(
            totalHeight, win.height - 155
        );

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(), totalHeight
        });

        int index = 0;

        for (auto const& player : m_players) {
            auto card = CCLayerColor::create(
                {15, 15, 18, 255},
                scroll->getContentWidth() - 4,
                cardHeight
            );

            float y = totalHeight -
                ((index + 1) * (cardHeight + gap));

            card->setPosition({0, y});
            scroll->m_contentLayer->addChild(card);

            ccColor3B rankColor = WHITE;

            if (player.rank == 1) rankColor = LIGHT_GOLD;
            else if (player.rank == 2) rankColor = {190, 200, 215};
            else if (player.rank == 3) rankColor = {205, 145, 95};

            addAccent(card, 0, 0, 3, cardHeight, rankColor);

            auto rank = makeLabel(
                "#" + std::to_string(player.rank),
                "goldFont.fnt", 0.58f, rankColor
            );
            rank->setAnchorPoint({0, 0.5f});
            rank->setPosition({12, 36});
            card->addChild(rank);

            auto name = makeLabel(
                player.name, "bigFont.fnt", 0.47f, WHITE
            );
            name->setAnchorPoint({0, 0.5f});
            name->setPosition({48, 36});
            card->addChild(name);

            auto country = makeLabel(
                player.country, "chatFont.fnt", 0.37f, GRAY
            );
            country->setAnchorPoint({0, 0.5f});
            country->setPosition({48, 15});
            card->addChild(country);

            auto points = makeLabel(
                std::to_string(player.points) + " PTS",
                "goldFont.fnt", 0.48f, GOLD
            );
            points->setAnchorPoint({1, 0.5f});
            points->setPosition({
                card->getContentSize().width - 12, 38
            });
            card->addChild(points);

            auto openLabel = makeLabel(
                "VER PERFIL", "chatFont.fnt", 0.40f, CYAN
            );

            auto openBtn = CCMenuItemLabel::create(
                openLabel, this,
                menu_selector(WeonListLayer::onOpenPlayer)
            );

            openBtn->setTag(index);

            auto menu = CCMenu::createWithItem(openBtn);
            menu->setPosition({
                card->getContentSize().width - 55, 14
            });
            card->addChild(menu, 5);

            index++;
        }
    }

    void showPlayerProfile(int index) {
        if (index < 0 ||
            index >= static_cast<int>(m_players.size())) {
            return;
        }

        clearContent();

        auto const& player = m_players[index];
        auto win = CCDirector::sharedDirector()->getWinSize();

        float panelWidth = win.width - 50;
        float panelHeight = win.height - 155;

        auto scroll = ScrollLayer::create({
            panelWidth, panelHeight
        });

        if (!scroll) return;

        scroll->setPosition({25, 30});
        addChild(scroll, 2);
        m_content = scroll;

        // Altura calculada según la cantidad de registros
        int completedCount = player.completed.isArray()
            ? static_cast<int>(player.completed.size()) : 0;

        int verifiedCount = player.verified.isArray()
            ? static_cast<int>(player.verified.size()) : 0;

        float totalHeight = 360.0f +
            completedCount * 22.0f +
            verifiedCount * 22.0f;

        totalHeight = std::max(totalHeight, panelHeight);

        scroll->m_contentLayer->setContentSize({
            scroll->getContentWidth(), totalHeight
        });

        auto content = scroll->m_contentLayer;

        auto addText = [&](std::string const& text,
                           float x, float y,
                           float scale, ccColor3B color) {
            auto label = CCLabelBMFont::create(
                text.c_str(), "chatFont.fnt"
            );

            label->setAnchorPoint({0, 0.5f});
            label->setPosition({x, y});
            label->setScale(scale);
            label->setColor(color);
            content->addChild(label);
        };

        auto header = makeLabel(
            "PERFIL DEL JUGADOR",
            "goldFont.fnt", 0.68f, LIGHT_GOLD
        );
        header->setAnchorPoint({0, 0.5f});
        header->setPosition({16, totalHeight - 35});
        content->addChild(header);

        addAccent(
            content, 15, totalHeight - 57,
            panelWidth - 30, 2, GOLD
        );

        addText(
            "PUESTO: #" + std::to_string(player.rank),
            16, totalHeight - 82, 0.52f, LIGHT_GOLD
        );

        addText(
            "JUGADOR: " + player.name,
            16, totalHeight - 108, 0.48f, WHITE
        );

        addText(
            "PAIS: " + player.country,
            16, totalHeight - 134, 0.48f, WHITE
        );

        addText(
            "PUNTOS: " + std::to_string(player.points),
            16, totalHeight - 160, 0.48f, GOLD
        );

        float y = totalHeight - 202;

        addText(
            "NIVELES COMPLETADOS",
            16, y, 0.52f, CYAN
        );
        y -= 25;

        if (player.completed.isArray() &&
            player.completed.size() > 0) {

            for (auto const& item : player.completed) {
                std::string level;
                std::string record;

                if (item.isObject()) {
                    level = item["level"].asString()
                        .unwrapOr("Desconocido");

                    record = item["record"].asString()
                        .unwrapOr("");
                } else {
                    level = item.asString()
                        .unwrapOr("Desconocido");
                }

                addText(
                    "- " + level +
                    (record.empty() ? "" : " (" + record + ")"),
                    20, y, 0.39f, WHITE
                );

                y -= 22;
            }
        } else {
            addText(
                "Sin niveles registrados",
                20, y, 0.39f, GRAY
            );
            y -= 22;
        }

        y -= 12;

        addText(
            "NIVELES VERIFICADOS",
            16, y, 0.52f, CYAN
        );
        y -= 25;

        if (player.verified.isArray() &&
            player.verified.size() > 0) {

            for (auto const& item : player.verified) {
                std::string level =
                    item.asString().unwrapOr("Desconocido");

                addText(
                    "- " + level,
                    20, y, 0.39f, WHITE
                );

                y -= 22;
            }
        } else {
            addText(
                "Sin niveles verificados",
                20, y, 0.39f, GRAY
            );
        }

        // Botón fijo fuera del ScrollLayer
        auto backLabel = makeLabel(
            "VOLVER", "goldFont.fnt", 0.65f, LIGHT_GOLD
        );

        auto backBtn = CCMenuItemLabel::create(
            backLabel, this,
            menu_selector(WeonListLayer::onBackProfile)
        );

        m_backMenu = CCMenu::createWithItem(backBtn);
        m_backMenu->setPosition({
            win.width / 2, 13
        });

        addChild(m_backMenu, 20);
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

        auto sprite = CCSprite::create(
            "WL-Boton-lmao.png"_spr
        );

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
e->setPosition({12, 30});
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
