#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace geode::prelude;

// ---------------------------------------------------------------
// Replay: запись и воспроизведение нажатий по номеру физического шага
// ---------------------------------------------------------------

struct ReplayInput {
	int frame;
	int button;
	bool down;
	bool player1;
};

static std::vector<ReplayInput> g_macro;
static size_t g_index = 0;      // следующее нажатие для воспроизведения
static int g_frame = 0;         // номер физического шага с начала попытки
static bool g_recording = false;
static bool g_playing = false;
static bool g_injecting = false;

static bool isMine(GJBaseGameLayer* layer) {
	auto pl = PlayLayer::get();
	return pl && static_cast<GJBaseGameLayer*>(pl) == layer;
}

static std::string replayMode() {
	return Mod::get()->getSettingValue<std::string>("replay-mode");
}

static std::filesystem::path replayPath(GJGameLevel* level) {
	int id = level ? level->m_levelID.value() : 0;
	return Mod::get()->getSaveDir() / ("replay_" + std::to_string(id) + ".txt");
}

static void saveReplay(GJGameLevel* level) {
	std::ofstream file(replayPath(level));
	if (!file) {
		Notification::create("Replay: не удалось сохранить файл", NotificationIcon::Error)->show();
		return;
	}
	for (auto const& e : g_macro) {
		file << e.frame << ' ' << e.button << ' ' << (e.down ? 1 : 0) << ' ' << (e.player1 ? 1 : 0) << '\n';
	}
	Notification::create("Replay сохранён: " + std::to_string(g_macro.size()) + " нажатий", NotificationIcon::Success)->show();
}

static void loadReplay(GJGameLevel* level) {
	g_macro.clear();
	std::ifstream file(replayPath(level));
	if (!file) return;
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream in(line);
		int frame, button, down, p1;
		if (in >> frame >> button >> down >> p1) {
			g_macro.push_back({frame, button, down != 0, p1 != 0});
		}
	}
}

// ---------------------------------------------------------------
// PlayLayer: noclip + начало/конец попытки для replay
// ---------------------------------------------------------------

class $modify(CheatsPlayLayer, PlayLayer) {
	// Noclip: игнорируем смерть игрока.
	// Античит-шип (m_anticheatSpike) оставляем смертельным, как в самой игре.
	void destroyPlayer(PlayerObject* player, GameObject* object) {
		if (Mod::get()->getSettingValue<bool>("noclip") && object != m_anticheatSpike) {
			return;
		}
		PlayLayer::destroyPlayer(player, object);
	}

	// Начало каждой попытки: сбрасываем счётчик и выбираем режим replay.
	void resetLevel() {
		PlayLayer::resetLevel();

		g_frame = 0;
		g_index = 0;
		g_recording = false;
		g_playing = false;
		g_injecting = false;

		// в практике с чекпоинтами номера шагов не совпадают, поэтому replay там выключен
		if (m_isPracticeMode) return;

		auto mode = replayMode();
		if (mode == "Record") {
			g_macro.clear();
			g_recording = true;
		} else if (mode == "Play") {
			loadReplay(m_level);
			g_playing = !g_macro.empty();
		}
	}

	// Запись сохраняется, когда уровень пройден.
	void levelComplete() {
		if (g_recording && !g_macro.empty()) {
			saveReplay(m_level);
		}
		g_recording = false;
		g_playing = false;
		PlayLayer::levelComplete();
	}
};

// ---------------------------------------------------------------
// GJBaseGameLayer: запись нажатий и подача записанных нажатий
// ---------------------------------------------------------------

class $modify(CheatsBaseGameLayer, GJBaseGameLayer) {
	void handleButton(bool down, int button, bool isPlayer1) {
		if (isMine(this) && !g_injecting) {
			// при воспроизведении свои нажатия игнорируем, чтобы не ломать запись
			if (g_playing) return;
			if (g_recording) {
				g_macro.push_back({g_frame, button, down, isPlayer1});
			}
		}
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}

	void processCommands(float dt) {
		bool mine = isMine(this);
		int shift = static_cast<int>(Mod::get()->getSettingValue<int64_t>("replay-offset"));

		if (mine && g_playing) {
			while (g_index < g_macro.size() && g_macro[g_index].frame + shift <= g_frame) {
				auto const& e = g_macro[g_index++];
				g_injecting = true;
				this->handleButton(e.down, e.button, e.player1);
				g_injecting = false;
			}
		}

		GJBaseGameLayer::processCommands(dt);

		if (mine && (g_playing || g_recording)) {
			g_frame++;
		}
	}
};

// ---------------------------------------------------------------
// Speedhack: умножаем время кадра на выбранный множитель.
// ---------------------------------------------------------------

class $modify(CheatsScheduler, CCScheduler) {
	void update(float dt) {
		auto mod = Mod::get();
		if (mod->getSettingValue<bool>("speedhack")) {
			float speed = static_cast<float>(mod->getSettingValue<double>("speed"));
			if (speed > 0.f) {
				dt *= speed;
			}
		}
		CCScheduler::update(dt);
	}
};
