#pragma once

#include "ofMain.h"
#include "Aquarium.h"


class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
	
		
		char moveDirection;
		int DEFAULT_SPEED = 5;


		AwaitFrames acuariumUpdate{5};

		ofTrueTypeFont gameOverTitle;
		GameEvent lastEvent;


		ofImage backgroundImage;

		std::unique_ptr<GameSceneManager> gameManager;
		std::shared_ptr<AquariumSpriteManager>spriteManager;
		
		// === New: background music player ===
        ofSoundPlayer backgroundMusic;
};
