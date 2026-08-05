#include "Toolbar.h"
#include "../Config/GameConfig.h"
#include "../Core/Game.h"
#include <fstream>
#include "../Entities/Animal.h"


ToolbarIcon::ToolbarIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : Drawable(r_pGame, r_point, r_width, r_height)
{
	image_path = img_path;
}

void ToolbarIcon::draw() const
{
	window* pWind = pGame->getWind();
	pWind->DrawImage(image_path, RefPoint.x, RefPoint.y, width, height);
}

RestartIcon::RestartIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void RestartIcon::onClick()
{
	// Clear chickens
	for (int i = 0; i < pGame->chickCount; i++) {
		delete pGame->chickList[i];
		pGame->chickList[i] = nullptr;
	}
	pGame->chickCount = 0;

	// Clear cows
	for (int i = 0; i < pGame->cowCount; i++) {
		delete pGame->cowList[i];
		pGame->cowList[i] = nullptr;
	}
	pGame->cowCount = 0;

	// Clear wolves
	for (int i = 0; i < pGame->wolfCount; i++) {
		delete pGame->wolfList[i];
		pGame->wolfList[i] = nullptr;
	}
	pGame->wolfCount = 0;

	// Clear items
	for (int i = 0; i < pGame->ItemCount; i++) {
		delete pGame->ItemList[i];
		pGame->ItemList[i] = nullptr;
	}
	pGame->ItemCount = 0;

	// Reset game state
	pGame->budget = 30000;
	pGame->remainingTime = 300;   // FIX: reset to starting time
	pGame->eggInWarehouse = 0;
	pGame->milkInWarehouse = 0;
	pGame->animalID = 0;
	pGame->foodAreas.clear();

	// Redraw game area
	pGame->getWind()->SetPen(ORANGE, 2);
	pGame->getWind()->SetBrush(ORANGE);
	pGame->getWind()->DrawRectangle(0, 2 * config.toolBarHeight,
		config.windWidth,
		config.windHeight - config.statusBarHeight);
	pGame->drawWarehouse();
	pGame->printBudget("BUDGET = $" + to_string(pGame->budget));
	pGame->printMessage("Game Restarted!");
}

ExitIcon::ExitIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void ExitIcon::onClick()
{
	pGame->stopBackgroundMusic();
	pGame->saveScore();
	SaveIcon s(pGame, { 0,0 }, 0, 0, "");
	s.onClick();
	exit(0);
}

PauseIcon::PauseIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void PauseIcon::onClick()
{
	pGame->isPaused = true;
	pGame->printMessage("Game Paused - Click Resume to continue");

	window* pWind = pGame->getWind();
	int x, y;
	while (pGame->isPaused)
	{
		if (pWind->GetButtonState(LEFT_BUTTON, x, y))
		{
			if (y >= 0 && y < config.toolBarHeight)
			{
				int clickedIndex = x / config.iconWidth;
				if (clickedIndex == ICON_RESUME)
				{
					pGame->isPaused = false;
					pGame->printMessage("Game Resumed!");
				}
			}
		}
		Sleep(50);
	}
}

ResumeIcon::ResumeIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path) : ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void ResumeIcon::onClick()
{
	pGame->isPaused = false;
	pGame->printMessage("Game Resumed!");
}

SaveIcon::SaveIcon(Game* r_pGame, point r_point, int r_width, int r_height, string img_path)
	: ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void SaveIcon::onClick()
{
	// FIX: removed ios::app — overwrite the file cleanly each save
	ofstream file("save_slot_1.txt");
	if (!file) {
		pGame->printMessage("Save Failed!");
		return;
	}

	file << "PlayerName: " << pGame->playerName << "\n";
	file << "Level: " << pGame->currentLevel << "\n";
	file << "Budget: " << pGame->budget << "\n";
	file << "Timer: " << pGame->remainingTime << "\n";
	file << "WarehouseEggs: " << pGame->eggInWarehouse << "\n";
	file << "WarehouseMilk: " << pGame->milkInWarehouse << "\n";
	file << "ChickCount: " << pGame->chickCount << "\n";
	file << "CowCount: " << pGame->cowCount << "\n";

	for (int i = 0; i < pGame->chickCount; i++) {
		if (pGame->chickList[i])
			file << pGame->chickList[i]->getPos().x << " "
			<< pGame->chickList[i]->getPos().y << "\n";
	}

	for (int i = 0; i < pGame->cowCount; i++) {
		if (pGame->cowList[i])
			file << pGame->cowList[i]->getPos().x << " "
			<< pGame->cowList[i]->getPos().y << "\n";
	}

	int realWolfCount = 0;
	for (int i = 0; i < pGame->wolfCount; i++)
		if (pGame->wolfList[i] != nullptr)
			realWolfCount++;

	file << "WolfCount: " << realWolfCount << "\n";
	for (int i = 0; i < pGame->wolfCount; i++) {
		if (pGame->wolfList[i])
			file << pGame->wolfList[i]->getPos().x << " "
			<< pGame->wolfList[i]->getPos().y << " "
			<< pGame->currentLevel << "\n";
	}

	file << "FoodCount: " << pGame->foodAreas.size() << "\n";
	for (auto& f : pGame->foodAreas)
		file << f.x << " " << f.y << " " << f.amount << "\n";

	file.close();
	pGame->printMessage("Game Saved!");
}

LoadIcon1::LoadIcon1(Game* r_pGame, point r_point, int r_width, int r_height, string img_path)
	: ToolbarIcon(r_pGame, r_point, r_width, r_height, img_path)
{
}

void LoadIcon1::onClick()
{
	// FIX: use relative path so it works on every machine
	string filename = "save_slot_1.txt";

	ifstream file(filename);
	if (!file) {
		pGame->printMessage("No Save File Found!");
		return;
	}

	// Clear existing animals and state
	for (int i = 0; i < pGame->chickCount; i++) {
		delete pGame->chickList[i];
		pGame->chickList[i] = nullptr;
	}
	pGame->chickCount = 0;

	for (int i = 0; i < pGame->cowCount; i++) {
		delete pGame->cowList[i];
		pGame->cowList[i] = nullptr;
	}
	pGame->cowCount = 0;

	for (int i = 0; i < pGame->wolfCount; i++) {
		delete pGame->wolfList[i];
		pGame->wolfList[i] = nullptr;
	}
	pGame->wolfCount = 0;

	pGame->foodAreas.clear();

	// FIX: read fields in the exact same order as SaveIcon writes them
	string label;
	file >> label >> pGame->playerName;
	file >> label >> pGame->currentLevel;
	file >> label >> pGame->budget;
	file >> label >> pGame->remainingTime;
	file >> label >> pGame->eggInWarehouse;
	file >> label >> pGame->milkInWarehouse;

	int savedChickCount, savedCowCount;
	file >> label >> savedChickCount;
	file >> label >> savedCowCount;

	// FIX: clear screen before redrawing loaded animals
	pGame->getWind()->SetPen(ORANGE, 2);
	pGame->getWind()->SetBrush(ORANGE);
	pGame->getWind()->DrawRectangle(0, 2 * config.toolBarHeight,
		config.windWidth,
		config.windHeight - config.statusBarHeight);

	for (int i = 0; i < savedChickCount; i++) {
		point p;
		file >> p.x >> p.y;
		pGame->animalID++;
		Chick* c = new Chick(pGame, p, 50, 50, "images\\chick.jpg", pGame->animalID);
		pGame->chickList[pGame->chickCount++] = c;
		c->draw();
	}

	for (int i = 0; i < savedCowCount; i++) {
		point p;
		file >> p.x >> p.y;
		pGame->animalID++;
		Cow* c = new Cow(pGame, p, 50, 50, "images\\cow.jpg", pGame->animalID);
		pGame->cowList[pGame->cowCount++] = c;
		c->draw();
	}

	int savedWolfCount;
	file >> label >> savedWolfCount;
	for (int i = 0; i < savedWolfCount; i++) {
		point p;
		int level;
		file >> p.x >> p.y >> level;
		pGame->animalID++;
		Wolf* w = new Wolf(pGame, p, 50, 50, "images\\wolf.jpg", pGame->animalID, level);
		pGame->wolfList[pGame->wolfCount++] = w;
		w->draw();
	}

	int savedFoodCount;
	file >> label >> savedFoodCount;
	for (int i = 0; i < savedFoodCount; i++) {
		Game::FoodArea f;
		file >> f.x >> f.y >> f.amount;
		pGame->foodAreas.push_back(f);
	}

	file.close();

	pGame->lastSecond = chrono::steady_clock::now();
	pGame->drawWarehouse();
	pGame->printBudget("BUDGET = $" + to_string(pGame->budget));
	pGame->printMessage("Game Loaded Successfully");
}

Toolbar::Toolbar(Game* r_pGame, point r_point, int r_width, int r_height) : Drawable(r_pGame, r_point, r_width, r_height)
{
	iconsImages[ICON_RESTART] = "images\\RESTART.jpg";
	iconsImages[ICON_EXIT] = "images\\EXIT.jpg";
	iconsImages[ICON_RESUME] = "images\\resume.jpg";
	iconsImages[ICON_SAVE] = "images\\save.jpg";
	iconsImages[ICON_LOAD] = "images\\load.jpg";
	iconsImages[ICON_PAUSE] = "images\\pause.jpg";

	point p;
	p.x = 0;
	p.y = 0;

	iconsList = new ToolbarIcon * [ICON_COUNT];

	iconsList[ICON_RESTART] = new RestartIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_RESTART]);
	p.x += config.iconWidth;
	iconsList[ICON_EXIT] = new ExitIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_EXIT]);
	p.x += config.iconWidth;
	iconsList[ICON_PAUSE] = new PauseIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_PAUSE]);
	p.x += config.iconWidth;
	iconsList[ICON_LOAD] = new LoadIcon1(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_LOAD]);
	p.x += config.iconWidth;
	iconsList[ICON_SAVE] = new SaveIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_SAVE]);
	p.x += config.iconWidth;
	iconsList[ICON_RESUME] = new ResumeIcon(pGame, p, config.iconWidth, config.toolBarHeight, iconsImages[ICON_RESUME]);
}

Toolbar::~Toolbar()
{
	for (int i = 0; i < ICON_COUNT; i++)
		delete iconsList[i];
	delete[] iconsList;
}

void Toolbar::draw() const
{
	for (int i = 0; i < ICON_COUNT; i++)
		iconsList[i]->draw();
	window* pWind = pGame->getWind();
	pWind->SetPen(BLACK, 3);
	pWind->DrawLine(0, config.toolBarHeight, pWind->GetWidth(), config.toolBarHeight);
}

bool Toolbar::handleClick(int x, int y)
{
	if (x > ICON_COUNT * config.iconWidth)
		return false;

	int clickedIconIndex = (x / config.iconWidth);
	iconsList[clickedIconIndex]->onClick();
	if (clickedIconIndex == ICON_EXIT) return true;

	return false;
}