#include "iGraphics.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

std::string playerName = "";
int highScore = 0;
bool nameEntered = false;

using namespace std;

int loadingPhase = 1;
int secondLoadingImage, loadingState = 0;
int transitionImage;
int bossMovementState = 0;
int currentLevel = 1;
bool gameOver = false;
int gameOverImage;

int LoadingImage, SkyboundVengeance, gameBackground, howToPlayImage, optionImage, AboutUs, planeImage, firstWaveImage, secondWaveImage, thirdWaveImage, hiddenPackageImage, bossPlaneImage;
int victoryImage;

int backgroundX = 0, currentImage = 1, timerCount = 0, dotCount = 0;
char loadingText[20] = "Loading";

int blinkState = 0, blinkTimer = 0, selectedIcon = 1, gameBackgroundY = 0, imageHeight = 742;

int planeX = 475, planeY = 50, playerScore = 0, playerHealth = 100;
double angle = 0, wingOffset = 52.5;

int consecutiveKills = 0, strongFireTimer = 0;
bool strongFireActive = false;

int enemyCount = 0, resetTimerSecond = 0, resetTimerThird = 0, gameTimer = 0;
bool allSecondEnemiesExited = false, allThirdEnemiesExited = false, secondCodeActive = false, thirdWaveActive = false;

int packageSpawnTimer = 0, shieldTimer = 0, randomEnemyIndex = -1;
bool shieldActive = false;

// Add a new state variable to track the transition
bool showTransitionImage = false;
int transitionTimer = 0; // Timer for the 4-second delay

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 742
#define MAX_ENEMIES 7
#define ENEMY_WIDTH 110
#define ENEMY_HEIGHT 123
#define MIN_ENEMY_GAP 50
#define ENEMY_COUNT 5
#define ENEMY_SPACING 220
#define RESET_DELAY 10000

struct Bullet {
	double x, y, dx, dy;
};
vector<Bullet> bullets;

struct Enemy {
	double x, y, speed;
	bool active, isSecondType, isThirdType;
	int shootTimer;
	double angle;
};
Enemy enemies[MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT];

struct EnemyBullet {
	double x, y;
	double dx, dy;
};
vector<EnemyBullet> enemyBullets;

struct Package {
	double x, y, dx, dy;
	bool active, revealed, collected;
};
Package hiddenPackage = { 0, 0, 0, 0, false, false, false };

struct BossPlane {
	double x, y;
	int health;
	bool active;
	double speed;
	int movementState; // 0 = Zigzag Left, 1 = Zigzag Right
	double zigzagAmplitude; // How wide the zigzag movement is
	double zigzagSpeed; // How fast the zigzag movement is
};

BossPlane bossPlane = { SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT + 200, 200, false, 1.5, 0, 40, 0.01 };
bool bossDefeated = false;

struct BossBullet {
	double x, y;
	double dx, dy;
};
vector<BossBullet> bossBullets; // Vector to store boss bullets

void updateDots() {
	dotCount = (dotCount + 1) % 4;
}

void updateBlinkEffect() {
	blinkState = !blinkState;
}

void drawCornerLines(int x, int y, int size, int margin) {
	iLine(x - margin, y + size + margin, x + 10, y + size + margin);
	iLine(x - margin, y + size + margin, x - margin, y + size - 10);
	iLine(x + size - 10, y + size + margin, x + size + margin, y + size + margin);
	iLine(x + size + margin, y + size - 10, x + size + margin, y + size + margin);
	iLine(x - margin, y - margin, x + 10, y - margin);
	iLine(x - margin, y - margin, x - margin, y + 10);
	iLine(x + size - 10, y - margin, x + size + margin, y - margin);
	iLine(x + size + margin, y + 10, x + size + margin, y - margin);
}

void changeImage() {
	timerCount += 1;
	if (timerCount >= 4) {
		if (loadingState == 1) { // If transitioning to the game
			currentImage = 3; // Switch to game screen
			loadingState = 0; // Reset loading state
		}
		else {
			currentImage = 2; // Main menu after first loading
			PlaySound("D:\\GameFolder\\backgroundSong.wav", NULL, SND_LOOP | SND_ASYNC);
		}
		iPauseTimer(0);
	}
}

void bulletMove() {
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i].x += bullets[i].dx;
		bullets[i].y += bullets[i].dy;

		if (bullets[i].y > 800) {
			bullets.erase(bullets.begin() + i);
			i--;
		}
	}
}

void spawnEnemies() {
	enemyCount = 1 + rand() % 4;

	for (int i = 0; i < enemyCount; i++) {
		bool overlap;
		do {
			overlap = false;
			enemies[i].x = 30 + rand() % (SCREEN_WIDTH - 60 - ENEMY_WIDTH);
			for (int j = 0; j < i; j++) {
				if (abs(enemies[i].x - enemies[j].x) < ENEMY_WIDTH + MIN_ENEMY_GAP) {
					overlap = true;
					break;
				}
			}
		} while (overlap);

		enemies[i].y = SCREEN_HEIGHT;
		enemies[i].speed = 3;
		enemies[i].active = true;
		enemies[i].isSecondType = false;
		enemies[i].isThirdType = false;
	}
	cout << "First Wave Started" << endl;
}

void spawnSpiralEnemies() {
	enemyCount = 1 + rand() % 4;

	for (int i = 0; i < enemyCount; i++) {
		bool overlap;
		do {
			overlap = false;
			enemies[i].x = 30 + rand() % (SCREEN_WIDTH - 60 - ENEMY_WIDTH);
			for (int j = 0; j < i; j++) {
				if (abs(enemies[i].x - enemies[j].x) < ENEMY_WIDTH + MIN_ENEMY_GAP) {
					overlap = true;
					break;
				}
			}
		} while (overlap);

		enemies[i].y = SCREEN_HEIGHT;
		enemies[i].speed = 2; // Adjust speed as needed
		enemies[i].active = true;
		enemies[i].isSecondType = false;
		enemies[i].isThirdType = false;
		enemies[i].angle = 0; // Initialize angle for spiral movement
	}
	cout << "Spiral Wave Started" << endl;
}

void moveSpiralEnemies() {
	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].active) {
			
			enemies[i].angle += 0.05; 
			
			enemies[i].x += cos(enemies[i].angle) * 5; 
			enemies[i].y -= enemies[i].speed; 

			if (enemies[i].y < -ENEMY_HEIGHT) {
				enemies[i].active = false;
			}
		}
	}
}

void startNewLevel() {
	
	if (currentLevel == 1) {
		playerHealth = 100; 
		playerScore = 0;
	}

	bossPlane.active = false;
	bossDefeated = false;
	bullets.clear();
	enemyBullets.clear();
	hiddenPackage.active = false;
	hiddenPackage.revealed = false;
	hiddenPackage.collected = false;
	shieldActive = false;
	shieldTimer = 0;
	packageSpawnTimer = 0;
	gameTimer = 0;
	transitionTimer = 0;

	for (int i = 0; i < MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT; i++) {
		enemies[i].active = false;
	}

	secondCodeActive = false;
	thirdWaveActive = false;
	allSecondEnemiesExited = false;
	allThirdEnemiesExited = false;

	
	if (currentLevel == 1) {
		
		spawnEnemies(); 
		cout << "Level 1: Top-Down Wave Started" << endl;
	}
	else if (currentLevel == 2) {
		
		spawnSpiralEnemies(); 
		cout << "Level 2: Spiral Wave Started" << endl;
	}

	
}

void moveEnemies() {
	bool allGone = true;
	int activeEnemies = 0;

	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].active) {
			enemies[i].y -= enemies[i].speed;
			activeEnemies++;
		}

		if (enemies[i].y > -ENEMY_HEIGHT) {
			allGone = false;
		}
	}

	if (allGone || activeEnemies == 0) {
		spawnEnemies();
	}
}

bool allFirstWaveEnemiesGone() {
	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].y > -ENEMY_HEIGHT) {
			return false;
		}
	}
	return true;
}

void initSecondEnemies() {
	for (int i = 0; i < ENEMY_COUNT; i++) {
		enemies[MAX_ENEMIES + i].x = SCREEN_WIDTH + (i * ENEMY_SPACING);
		enemies[MAX_ENEMIES + i].y = 900;
		enemies[MAX_ENEMIES + i].speed = 2.0;
		enemies[MAX_ENEMIES + i].active = true;
		enemies[MAX_ENEMIES + i].isSecondType = true;
		enemies[MAX_ENEMIES + i].isThirdType = false;
		enemies[MAX_ENEMIES + i].shootTimer = 0;
	}
	allSecondEnemiesExited = false;
	resetTimerSecond = 0;
	cout << "Second Wave Started" << endl;
}

bool allSecondEnemiesGone() {
	for (int i = 0; i < ENEMY_COUNT; i++) {
		if (enemies[MAX_ENEMIES + i].x > -ENEMY_WIDTH && enemies[MAX_ENEMIES + i].y > -ENEMY_HEIGHT) {
			return false;
		}
	}
	return true;
}

void updateSecondEnemies() {
	if (allSecondEnemiesExited) {
		resetTimerSecond += 5;
		if (resetTimerSecond >= RESET_DELAY) {
			initSecondEnemies();
		}
		return;
	}

	for (int i = 0; i < ENEMY_COUNT; i++) {
		if (enemies[MAX_ENEMIES + i].active) {
			enemies[MAX_ENEMIES + i].x -= enemies[MAX_ENEMIES + i].speed;
			enemies[MAX_ENEMIES + i].y -= enemies[MAX_ENEMIES + i].speed * 0.55;

			
			enemies[MAX_ENEMIES + i].shootTimer += 5; 

			
			if (enemies[MAX_ENEMIES + i].shootTimer >= 1000) { 
			
				double bulletX1 = enemies[MAX_ENEMIES + i].x + ENEMY_WIDTH / 2 - 20; 
				double bulletX2 = enemies[MAX_ENEMIES + i].x + ENEMY_WIDTH / 2 + 20; 
				double bulletY = enemies[MAX_ENEMIES + i].y;

				EnemyBullet bullet1 = { bulletX1, bulletY, 0, -5 };
				EnemyBullet bullet2 = { bulletX2, bulletY, 0, -5 };

				enemyBullets.push_back(bullet1);
				enemyBullets.push_back(bullet2);

				enemies[MAX_ENEMIES + i].shootTimer = 0; // Reset shoot timer
			}
		}
	}

	if (allSecondEnemiesGone()) {
		allSecondEnemiesExited = true;
	}
}

void initThirdEnemies() {
	for (int i = 0; i < ENEMY_COUNT; i++) {
		enemies[MAX_ENEMIES + ENEMY_COUNT + i].x = -100 - (i * ENEMY_SPACING);
		enemies[MAX_ENEMIES + ENEMY_COUNT + i].y = 900;
		enemies[MAX_ENEMIES + ENEMY_COUNT + i].speed = 2.0;
		enemies[MAX_ENEMIES + ENEMY_COUNT + i].active = true;
		enemies[MAX_ENEMIES + ENEMY_COUNT + i].isSecondType = false;
		enemies[MAX_ENEMIES + ENEMY_COUNT + i].isThirdType = true;
	}
	allThirdEnemiesExited = false;
	resetTimerThird = 0;
	cout << "Third Wave Started" << endl;
}

bool allThirdEnemiesGone() {
	for (int i = 0; i < ENEMY_COUNT; i++) {
		if (enemies[MAX_ENEMIES + ENEMY_COUNT + i].x < SCREEN_WIDTH + 110 && enemies[MAX_ENEMIES + ENEMY_COUNT + i].y > -123) {
			return false;
		}
	}
	return true;
}

void updateThirdEnemies() {
	if (allThirdEnemiesExited) {
		resetTimerThird += 5;
		if (resetTimerThird >= RESET_DELAY) {
			initThirdEnemies();
		}
		return;
	}

	for (int i = 0; i < ENEMY_COUNT; i++) {
		if (enemies[MAX_ENEMIES + ENEMY_COUNT + i].active) {
			enemies[MAX_ENEMIES + ENEMY_COUNT + i].x += enemies[MAX_ENEMIES + ENEMY_COUNT + i].speed;
			enemies[MAX_ENEMIES + ENEMY_COUNT + i].y += enemies[MAX_ENEMIES + ENEMY_COUNT + i].speed * (-0.55);
		}
	}

	if (allThirdEnemiesGone()) {
		allThirdEnemiesExited = true;
	}
}

void shootEnemyBullets() {
	for (int i = 0; i < MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT; i++) {
		if (enemies[i].active) {
			if (!enemies[i].isSecondType && !enemies[i].isThirdType) {
				
				if (!secondCodeActive) { // Only fire if the second wave is not active
					double bulletX1 = enemies[i].x + ENEMY_WIDTH / 2 - 20;
					double bulletX2 = enemies[i].x + ENEMY_WIDTH / 2 + 20;
					double bulletY = enemies[i].y;

					EnemyBullet bullet1 = { bulletX1, bulletY, 0, -5 };
					EnemyBullet bullet2 = { bulletX2, bulletY, 0, -5 };

					enemyBullets.push_back(bullet1);
					enemyBullets.push_back(bullet2);
				}
			}
			else if (enemies[i].isSecondType) {
				double angle = -(90 + 28.1) * (3.1416 / 180); 

			
				double leftBulletX = enemies[i].x + ENEMY_WIDTH / 2 - 20; 
				double leftBulletY = enemies[i].y;

			
				double rightBulletX = enemies[i].x + ENEMY_WIDTH / 2 + 20; 
				double rightBulletY = enemies[i].y;

				EnemyBullet leftBullet = { leftBulletX, leftBulletY, cos(angle) * 5, sin(angle) * 5 };
				EnemyBullet rightBullet = { rightBulletX, rightBulletY, cos(angle) * 5, sin(angle) * 5 };

				enemyBullets.push_back(leftBullet);
				enemyBullets.push_back(rightBullet);
			}
		}
	}
}

void updateEnemyBullets() {
	for (int i = enemyBullets.size() - 1; i >= 0; i--) {
		enemyBullets[i].x += enemyBullets[i].dx; 
		enemyBullets[i].y += enemyBullets[i].dy; // Update y position

		
		if (enemyBullets[i].y < 0 || enemyBullets[i].x < 0 || enemyBullets[i].x > SCREEN_WIDTH) {
			enemyBullets.erase(enemyBullets.begin() + i);
		}
	}
}

void spawnHiddenPackage() {
	if (!shieldActive && packageSpawnTimer >= 5000) {
		randomEnemyIndex = rand() % enemyCount;
		hiddenPackage.x = enemies[randomEnemyIndex].x + ENEMY_WIDTH / 2 - 30;
		hiddenPackage.y = enemies[randomEnemyIndex].y - 56;
		hiddenPackage.active = true;
		hiddenPackage.revealed = false;
		hiddenPackage.collected = false;
		packageSpawnTimer = 0;
	}
}

void revealPackage(int enemyIndex) {
	if (hiddenPackage.active && !hiddenPackage.revealed && enemyIndex == randomEnemyIndex) {
		hiddenPackage.revealed = true;
		hiddenPackage.dx = 0;
		hiddenPackage.dy = -3;
	}
}

void checkPackageHit() {
	if (hiddenPackage.revealed && !hiddenPackage.collected) {
		for (int i = bullets.size() - 1; i >= 0; i--) {
			if (bullets[i].x >= hiddenPackage.x && bullets[i].x <= hiddenPackage.x + 60 &&
				bullets[i].y >= hiddenPackage.y && bullets[i].y <= hiddenPackage.y + 56) {
				hiddenPackage.collected = true;
				shieldActive = true;
				shieldTimer = 8000;
				bullets.erase(bullets.begin() + i);
				break;
			}
		}
	}
}

void updateShield() {
	if (shieldActive) {
		shieldTimer -= 5;
		if (shieldTimer <= 0) {
			shieldActive = false;
			packageSpawnTimer = 0;
		}
	}
}

void checkPlayerCollision() {

	if (shieldActive) {
		return; 
	}

	for (int j = 0; j < MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT; j++) {
		if (enemies[j].active) {
			
			if (planeX + 125 >= enemies[j].x && planeX + 125 <= enemies[j].x + ENEMY_WIDTH &&
				planeY + 69.5 >= enemies[j].y && planeY + 69.5 <= enemies[j].y + ENEMY_HEIGHT) {
				playerHealth -= 3; 
				enemies[j].active = false; 
				cout << "Player hit by enemy plane! Health: " << playerHealth << endl;

				if (playerHealth <= 0) {
					gameOver = true; 
					cout << "Game Over!" << endl;
				}
				enemies[j].x = 30 + rand() % (SCREEN_WIDTH - 60 - ENEMY_WIDTH);
				enemies[j].y = SCREEN_HEIGHT;
				enemies[j].speed = 3;
				enemies[j].active = true;
				enemies[j].isSecondType = false;
				enemies[j].isThirdType = false;
				break;
				break;
			}
		}
	}
}

void checkBulletCollisions() {
	for (int i = bullets.size() - 1; i >= 0; i--) {
		for (int j = 0; j < MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT; j++) {
			if (enemies[j].active) {
				if (bullets[i].x >= enemies[j].x && bullets[i].x <= enemies[j].x + ENEMY_WIDTH &&
					bullets[i].y >= enemies[j].y && bullets[i].y <= enemies[j].y + ENEMY_HEIGHT) {
					enemies[j].active = false;
					bullets.erase(bullets.begin() + i);

					playerScore += 6;

					if (hiddenPackage.active && !hiddenPackage.revealed && j == randomEnemyIndex) {
						revealPackage(j); // Add this line
					}

					enemies[j].x = 30 + rand() % (SCREEN_WIDTH - 60 - ENEMY_WIDTH);
					enemies[j].y = SCREEN_HEIGHT;
					enemies[j].speed = 3;
					enemies[j].active = true;

					break;
				}
			}
		}
	}
	checkPackageHit();
}

void checkEnemyBulletCollisions() {

	
	if (shieldActive) {
		return; 
	}

	for (int i = enemyBullets.size() - 1; i >= 0; i--) {
		
		if (enemyBullets[i].x >= planeX && enemyBullets[i].x <= planeX + 250 &&
			enemyBullets[i].y >= planeY && enemyBullets[i].y <= planeY + 139) {

			
			playerHealth -= 2; 
			enemyBullets.erase(enemyBullets.begin() + i); 

			cout << "Player hit by enemy bullet! Health: " << playerHealth << endl;

			
			if (playerHealth <= 0) {
				gameOver = true; 
				cout << "Game Over!" << endl;
			}
		}
	}
}

void checkBossSpawn() {
	
	if (currentLevel == 1 && playerScore >= 250 && !bossPlane.active) {
	
		for (int i = 0; i < MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT; i++) {
			enemies[i].active = false;
		}

		bossPlane.active = true;
		bossPlane.x = SCREEN_WIDTH / 2 - 150; 
		bossPlane.y = SCREEN_HEIGHT + 200; 
		bossPlane.health = 300; 

		
		bossMovementState = 0; 
		cout << "Zigzag Boss Plane Arrived!" << endl;
	}

	
	if (currentLevel == 2 && playerScore >= 800 && !bossPlane.active) {
	
		for (int i = 0; i < MAX_ENEMIES + ENEMY_COUNT + ENEMY_COUNT; i++) {
			enemies[i].active = false;
		}

	
		bossPlane.active = true;
		bossPlane.x = SCREEN_WIDTH / 2 - 150; 
		bossPlane.y = SCREEN_HEIGHT + 200; 
		bossPlane.health = 600; 

		
		bossMovementState = 1; 
		cout << "Circular Boss Plane Arrived!" << endl;
	}
}

void updateBossZigzag(BossPlane &boss) {
	static double zigzagAngle = 0; 
	zigzagAngle += boss.zigzagSpeed; 

	if (boss.movementState == 0) { 
		boss.x -= (sin(zigzagAngle) *  boss.zigzagAmplitude); 
	}
	else if (boss.movementState == 1) { 
		boss.x += (sin(zigzagAngle) * boss.zigzagAmplitude); 
	}

	boss.y -= boss.speed;

	if (boss.x <= 250) {
		boss.movementState = 1;
		zigzagAngle = 0;
	}

	if (boss.x >= 950) {
		boss.movementState = 0;
		zigzagAngle = 0;
	}

	if (boss.x < 0) {
		boss.x = 0;
	}
	else if (boss.x > SCREEN_WIDTH - 300) {
		boss.x = SCREEN_WIDTH - 300;
	}
}

void updateBossMovement(BossPlane &boss, double playerX, double playerY) {
	if (currentLevel == 2) {

		static double angle = 0; 
		angle += 0.05; 

		boss.x = SCREEN_WIDTH / 2 + 100 * cos(angle); 
		boss.y = SCREEN_HEIGHT / 2 + 100 * sin(angle); 
	}
	else {
		updateBossZigzag(boss);

		if (boss.y <= 55) {
			boss.speed = -boss.speed;
			boss.y = 55;
		}

		if (boss.y >= SCREEN_HEIGHT - 100) {
			boss.speed = -boss.speed;
			boss.y = SCREEN_HEIGHT - 200;
		}
	}


	if (boss.x < 0) {
		boss.x = 0;
	}
	else if (boss.x > SCREEN_WIDTH - 300) {
		boss.x = SCREEN_WIDTH - 300;
	}
}

void moveBossPlane() {
	if (bossPlane.active) {
		updateBossMovement(bossPlane, planeX + 125, planeY + 69.5); 
	}
}

void shootBossBullets() {
	if (bossPlane.active) {
		
		static int shootTimer = 0;
		shootTimer += 15; 

		if (shootTimer >= 1000) { 
			
			double bulletX1 = bossPlane.x + 100; 
			double bulletX2 = bossPlane.x + 200; 
			double bulletY = bossPlane.y;

			BossBullet bullet1 = { bulletX1, bulletY, 0, -5 }; 
			BossBullet bullet2 = { bulletX2, bulletY, 0, -5 }; 

			bossBullets.push_back(bullet1);
			bossBullets.push_back(bullet2);

			shootTimer = 0; 
		}
	}
}

void updateBossBullets() {
	for (int i = bossBullets.size() - 1; i >= 0; i--) {
		bossBullets[i].y += bossBullets[i].dy; 

		
		if (bossBullets[i].y < 0) {
			bossBullets.erase(bossBullets.begin() + i);
		}
	}
}

void checkBulletCollisionsWithBoss() {
	for (int i = bullets.size() - 1; i >= 0; i--) {
		if (bossPlane.active) {
			if (bullets[i].x >= bossPlane.x && bullets[i].x <= bossPlane.x + 300 &&
				bullets[i].y >= bossPlane.y && bullets[i].y <= bossPlane.y + 200) {

				bossPlane.health -= 10;
				bullets.erase(bullets.begin() + i);

				if (bossPlane.health <= 0) {
					bossPlane.active = false;
					bossDefeated = true;

					if (currentLevel == 1) {
						playerHealth += 50;
						if (playerHealth > 100) {
							playerHealth = 100;
						}
					}
					else if (currentLevel == 2) {
						gameOver = true; // Set game over if boss is defeated in level 2
					}

					cout << "Boss Defeated!" << endl;
				}
			}
		}
	}
}


void drawHealthBar() {
	float healthRatio = static_cast<float>(playerHealth) / 100.0f;
	int barWidth = 200; 
	int barHeight = 15; 
	int x = (SCREEN_WIDTH - barWidth) - 900; 
	int y = SCREEN_HEIGHT - barHeight - 82; 

	
	if (healthRatio > 0.5f) {
		iSetColor(0, 255, 0); 
	}
	else if (healthRatio > 0.2f) {
		iSetColor(255, 255, 0);
	}
	else {
		iSetColor(255, 0, 0); 
	}

	
	iFilledRectangle(x, y, barWidth * healthRatio, barHeight);

	
	iSetColor(255, 255, 255); 
	iRectangle(x, y, barWidth, barHeight);
}

void drawBossHealthBar(double bossHealth) {
	if (!bossPlane.active) return; 

	float healthRatio = static_cast<float>(bossHealth) / (currentLevel == 1 ? 300.0f : 600.0f); 
	int barWidth = 200; 
	int barHeight = 15; 

	
	if (healthRatio > 0.5f) {
		iSetColor(0, 255, 0); 
	}
	else if (healthRatio > 0.2f) {
		iSetColor(255, 255, 0); 
	}
	else {
		iSetColor(255, 0, 0); 
	}

	iFilledRectangle(1000, 700, barWidth * healthRatio, barHeight);

	iSetColor(255, 255, 255); 
	iRectangle(1000, 700, barWidth, barHeight); 
}


void iDraw() {
	iClear();

	if (currentImage == 1) { 
		if (loadingPhase == 1) {
			iShowImage(0, 0, 1200, 742, LoadingImage); 
		}
		else if (loadingPhase == 2) {
			iShowImage(0, 0, 1200, 742, secondLoadingImage); 
		}

		strcpy_s(loadingText, "Loading");
		for (int i = 0; i < dotCount; i++) {
			strcat_s(loadingText, ".");
		}
		iText(549, 50, loadingText, GLUT_BITMAP_HELVETICA_18);
	}
	else if (currentImage == 2) {
		iShowImage(0, 0, 1200, 742, SkyboundVengeance);
		if (blinkState) {
			iSetColor(175, 165, 0);
		}
		int x, y, size = 58, margin = 4;

		if (selectedIcon == 1) {
			x = 602; y = 128;
			iSetColor(255, 255, 255);
			iText(612, 102, "Play", GLUT_BITMAP_HELVETICA_18);
		}
		else if (selectedIcon == 2) {
			x = 714.8; y = 128;
			iSetColor(255, 255, 255);
			iText(692, 102, "How To Play", GLUT_BITMAP_HELVETICA_18);
		}
		else if (selectedIcon == 3) {
			x = 819; y = 128;
			iSetColor(255, 255, 255);
			iText(818, 102, "Option", GLUT_BITMAP_HELVETICA_18);
		}
		else if (selectedIcon == 4) {
			x = 930; y = 128;
			iSetColor(255, 255, 255);
			iText(924, 102, "About Us", GLUT_BITMAP_HELVETICA_18);
		}
		else if (selectedIcon == 5) {
			x = 1040; y = 128;
			iSetColor(255, 255, 255);
			iText(1055, 102, "Exit", GLUT_BITMAP_HELVETICA_18);
		}
		drawCornerLines(x, y, size, margin);
	}
	else if (currentImage == 3) {
		PlaySound(NULL, NULL, 0);
		if (gameOver) {
			
			iShowImage(0, 0, 1200, 742, gameOverImage);

			if (!nameEntered) {
				iSetColor(255, 255, 255);
				iText(750, 185, "Name : ", GLUT_BITMAP_TIMES_ROMAN_24);
				char nameBuffer[100];
				strncpy_s(nameBuffer, playerName.c_str(), sizeof(nameBuffer));
				iText(850, 185, nameBuffer, GLUT_BITMAP_TIMES_ROMAN_24);
			}
			else {
				char scoreText[100];
				sprintf_s(scoreText, "%s, your score is %d", playerName.c_str(), playerScore);
				iText(750, 170, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);

				char highScoreText[100];
				sprintf_s(highScoreText, "Highscore: %d", highScore);
				iText(750, 100, highScoreText, GLUT_BITMAP_TIMES_ROMAN_24);
			}
		}
		else if (bossDefeated) {
			if (showTransitionImage) {
				
				iShowImage(0, 0, 1200, 742, transitionImage);
			}
			else {
				
				iShowImage(0, 0, 1200, 742, victoryImage);
			}

		}
		else {
			int offsetY = gameBackgroundY % imageHeight;
			int overlap = 2;

			iShowImage(backgroundX, offsetY, 1200, imageHeight, gameBackground);
			if (offsetY < 0) {
				iShowImage(backgroundX, offsetY + imageHeight - overlap, 1200, imageHeight, gameBackground);
			}

			iRotate(planeX + 125, planeY + 69.5, angle);
			iShowImage(planeX, planeY, 250, 139, planeImage);
			iUnRotate();

			for (auto &b : bullets) {
				iFilledCircle(b.x, b.y, 5);
			}

			drawHealthBar();
			drawBossHealthBar(bossPlane.health);

			for (int i = 0; i < enemyCount; i++) {
				if (enemies[i].active && !enemies[i].isSecondType && !enemies[i].isThirdType) {
					iShowImage(enemies[i].x, enemies[i].y, ENEMY_WIDTH, ENEMY_HEIGHT, firstWaveImage);

					for (const auto& bullet : enemyBullets) {
						iFilledCircle(bullet.x, bullet.y, 5);
					}
				}
			}

			if (secondCodeActive) {
				for (int i = 0; i < ENEMY_COUNT; i++) {
					if (enemies[MAX_ENEMIES + i].active) {
						iShowImage(enemies[MAX_ENEMIES + i].x, enemies[MAX_ENEMIES + i].y, 110, 113, secondWaveImage);
					
					for (const auto& bullet : enemyBullets) {
						iFilledCircle(bullet.x, bullet.y, 5);
						}
					}
				}
			}

			if (thirdWaveActive) {
				for (int i = 0; i < ENEMY_COUNT; i++) {
					if (enemies[MAX_ENEMIES + ENEMY_COUNT + i].active) {
						iShowImage(enemies[MAX_ENEMIES + ENEMY_COUNT + i].x, enemies[MAX_ENEMIES + ENEMY_COUNT + i].y, 110, 123, thirdWaveImage);
					
						for (const auto& bullet : enemyBullets) {
							iFilledCircle(bullet.x, bullet.y, 5);
							}
					}
				}
			}

			if (hiddenPackage.revealed && !hiddenPackage.collected) {
				iShowImage(hiddenPackage.x, hiddenPackage.y, 60, 56, hiddenPackageImage);
			}

			if (shieldActive) {
				iSetColor(255, 255, 255);
				iCircle(planeX + 125, planeY + 69.5, 100);
			}

			if (bossPlane.active) {
				iShowImage(bossPlane.x, bossPlane.y, 300, 200, bossPlaneImage);

				for (const auto& bullet : bossBullets) {
					iFilledCircle(bullet.x, bullet.y, 5); 
				}
			}

			iSetColor(255, 255, 255);
			char scoreText[50];
			sprintf_s(scoreText, "Score: %d", playerScore);
			iText(10, 700, scoreText, GLUT_BITMAP_HELVETICA_18);

			iText(10, 645, "Health", GLUT_BITMAP_TIMES_ROMAN_24);

			
		}
	}
	else if (currentImage == 4) {
		iShowImage(0, 0, 1200, 742, howToPlayImage);
	}
	else if (currentImage == 5) {
		iShowImage(0, 0, 1200, 742, optionImage);
	}
	else if (currentImage == 6) {
		iShowImage(0, 0, 1200, 742, AboutUs);
	}
	
}


void iMouseMove(int mx, int my) {}

void iPassiveMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (currentImage == 3 && bossDefeated) {
			
			showTransitionImage = true;
			transitionTimer = 0; 
		}
	}
}

bool readyToReturnToMainMenu = false; 

void iKeyboard(unsigned char key) {
	if (key == 13 && currentImage == 2 && selectedIcon == 1) {
		currentImage = 1; 
		timerCount = 0;
		loadingPhase = 2; 
		loadingState = 1; 
		iResumeTimer(0); 
	}
	else if (key == 13 && currentImage == 2 && selectedIcon == 2) {
		currentImage = 4;
	}
	else if (key == 13 && currentImage == 2 && selectedIcon == 3) {
		currentImage = 5;
	}
	else if (key == 13 && currentImage == 2 && selectedIcon == 4) {
		currentImage = 6;
	}
	else if (key == 13 && currentImage == 2 && selectedIcon == 5) {
		exit(0);
	}
	else if (key == 27 && (currentImage == 3 || currentImage == 4 || currentImage == 5 || currentImage == 6)) {
		currentImage = 2;
	}

	if (gameOver && !nameEntered) {
		if (key == 13) { 
			nameEntered = true;

			if (playerScore > highScore) {
				highScore = playerScore;
				
				std::ofstream outFile("highscore.txt");
				outFile << highScore;
				outFile.close();
			}
			readyToReturnToMainMenu = true; 
		}
		else if (key == 8) { 
			if (!playerName.empty()) {
				playerName.pop_back();
			}
		}
		else {
			playerName += key;
		}
	}
	else if (gameOver && nameEntered && readyToReturnToMainMenu && key == 13) {
		
		gameOver = false;
		nameEntered = false;
		readyToReturnToMainMenu = false;
		playerName = "";
		playerScore = 0;
		playerHealth = 100;
		currentLevel = 1; 
		currentImage = 2; 
		PlaySound("D:\\GameFolder\\backgroundSong.wav", NULL, SND_LOOP | SND_ASYNC);
		startNewLevel(); 
	}

	if (key == 'r') {
		angle += 2;
	}
	else if (key == 'l') {
		angle -= 2;
	}
	else if (key == 'f') {
		double rad = (angle + 90) * (3.1416 / 180);
		double cosA = cos(rad);
		double sinA = sin(rad);

		double perpRad = angle * (3.1416 / 180);
		double perpCos = cos(perpRad);
		double perpSin = sin(perpRad);

		double wingX1 = planeX + 105 + wingOffset * perpCos;
		double wingY1 = planeY + 90 + wingOffset * perpSin;
		double wingX2 = planeX + 143 - wingOffset * perpCos;
		double wingY2 = planeY + 90 - wingOffset * perpSin;

		Bullet b1 = { wingX1, wingY1, cosA * 5, sinA * 5 };
		Bullet b2 = { wingX2, wingY2, cosA * 5, sinA * 5 };

		bullets.push_back(b1);
		bullets.push_back(b2);

		if (strongFireActive) {
			Bullet b3 = { wingX1, wingY1, cosA * 5 + 1, sinA * 5 + 1 };
			Bullet b4 = { wingX2, wingY2, cosA * 5 - 1, sinA * 5 - 1 };
			bullets.push_back(b3);
			bullets.push_back(b4);
		}
	}
}


void iSpecialKeyboard(unsigned char key) {
	if (key == GLUT_KEY_RIGHT) {
		selectedIcon++;
		if (selectedIcon > 5) {
			selectedIcon = 5;
		}
		planeX += 12;
		if (planeX > 1200 - 250) {
			planeX = 1200 - 250;
		}
	}
	else if (key == GLUT_KEY_LEFT) {
		selectedIcon--;
		if (selectedIcon < 1) {
			selectedIcon = 1;
		}
		planeX -= 12;
		if (planeX < -150) {
			planeX = -150;
		}
	}
	if (key == GLUT_KEY_UP) {
		planeY += 12;
		if (planeY > 742 - 250) {
			planeY = 742 - 250;
		}
	}
	if (key == GLUT_KEY_DOWN) {
		planeY -= 12;
		if (planeY < -150) {
			planeY = -150;
		}
	}
}

void gameUpdate() {
	gameBackgroundY -= 1;
}



void iTimer() {
	static int lastMessageTime = 0;

	if (currentImage == 3) {
		if (playerHealth <= 0 && !bossDefeated) {
			gameOver = true;
			return;
		}

		gameTimer += 5; 
		packageSpawnTimer += 5; 

		if (gameTimer >= lastMessageTime + 5000) {
			cout << "Timer: " << gameTimer << endl;
			lastMessageTime = gameTimer;
		}

	
		spawnHiddenPackage();
		updateShield(); 
		checkPlayerCollision(); 
		checkBulletCollisionsWithBoss();
	}

	
	if (strongFireActive) {
		strongFireTimer -= 5; 
		if (strongFireTimer <= 0) {
			strongFireActive = false; 
			consecutiveKills = 0; 
		}
	}

	
	checkBossSpawn();

	if (bossPlane.active) {
		moveBossPlane(); 
		shootBossBullets();
		updateBossBullets();
	}


	else {
		if (!secondCodeActive && playerScore >=560 && currentImage == 3) { 
			
			if (allFirstWaveEnemiesGone()) {
				secondCodeActive = true;
				initSecondEnemies();
				cout << "Second Wave Started" << endl; 
			}
		}

		if (!thirdWaveActive && playerScore >= 680 && currentImage == 3) { 
			thirdWaveActive = true;
			initThirdEnemies();
			cout << "Third Wave Started" << endl;
		}

		if (secondCodeActive) {
			updateSecondEnemies();
		}

		if (thirdWaveActive) {
			updateThirdEnemies();
			if (gameTimer >= 45000 && allThirdEnemiesExited && currentImage == 3) {
				thirdWaveActive = false;
				secondCodeActive = false;

				currentLevel++;
				cout << "Level " << currentLevel - 1 << " Completed! Starting Level " << currentLevel << endl;

				startNewLevel();
			}
		}
		else if (!secondCodeActive) {
			if (currentLevel >= 2) {
				bool isZigZagWave = false;
				for (int i = 0; i < enemyCount; i++) {
					if (enemies[i].active && enemies[i].isSecondType == false && enemies[i].isThirdType == false) {
						isZigZagWave = true;
						break;
					}
				}
				if (secondCodeActive) {
					moveSpiralEnemies();
				}
				else {
					moveEnemies();
				}
			}
			else {
				moveEnemies();
			}
		}
	}

	static int shootTimer = 0;
	shootTimer += 15;
	if (shootTimer >= 800) {
		shootEnemyBullets();
		shootTimer = 0;
	}

	updateEnemyBullets();
	checkBulletCollisions();
	checkEnemyBulletCollisions();

	if (hiddenPackage.revealed && !hiddenPackage.collected) {
		hiddenPackage.x += hiddenPackage.dx;
		hiddenPackage.y += hiddenPackage.dy;

		if (hiddenPackage.x < 0 || hiddenPackage.x > SCREEN_WIDTH ||
			hiddenPackage.y < 0 || hiddenPackage.y > SCREEN_HEIGHT) {
			hiddenPackage.revealed = false;
			hiddenPackage.active = false;
		}
	}

	if (currentImage == 3 && bossDefeated) {
		if (showTransitionImage) {
			transitionTimer += 4;

			if (transitionTimer >= 4000) {
				showTransitionImage = false;
				bossDefeated = false;

				currentLevel++;
				cout << "Level " << currentLevel - 1 << " Completed! Starting Level " << currentLevel << endl;
				startNewLevel();
			}
		}
	}
}

int main() {
	std::ifstream inFile("highscore.txt");
	if (inFile) {
		inFile >> highScore;
		inFile.close();
	}

	iInitialize(1200, 742, "Game");

	LoadingImage = iLoadImage("E:\\GameFolder\\LoadingImage.jpg");
	SkyboundVengeance = iLoadImage("E:\\GameFolder\\SkyboundVengeance.jpg");
	secondLoadingImage = iLoadImage("E:\\GameFolder\\gameplayLoading.jpg");
	gameBackground = iLoadImage("E:\\GameFolder\\bg_02_h.jpg");
	howToPlayImage = iLoadImage("E:\\GameFolder\\Press.jpg");
	optionImage = iLoadImage("E:\\GameFolder\\optionImage.jpg");
	AboutUs = iLoadImage("E:\\GameFolder\\AboutUs.jpg");
	planeImage = iLoadImage("E:\\GameFolder\\aeroplane.png");
	firstWaveImage = iLoadImage("E:\\GameFolder\\enemyPlane.png");
	secondWaveImage = iLoadImage("E:\\GameFolder\\enemyAngle1.png");
	thirdWaveImage = iLoadImage("E:\\GameFolder\\enemyAngle2.png");
	hiddenPackageImage = iLoadImage("E:\\GameFolder\\hiddenPackage.png");
	bossPlaneImage = iLoadImage("E:\\GameFolder\\bossPlane.png");
	victoryImage = iLoadImage("E:\\GameFolder\\SUNDOWN1.jpg");
	transitionImage = iLoadImage("E:\\GameFolder\\SUNDOWN2.jpg");
	gameOverImage = iLoadImage("E:\\GameFolder\\SUNDOWN4.jpg");

	iSetTimer(1000, changeImage);
	iSetTimer(500, updateDots);
	iSetTimer(300, updateBlinkEffect);
	iSetTimer(30, gameUpdate);
	iSetTimer(10, bulletMove);
	iSetTimer(4, iTimer);

	iStart();
	return 0;
}

