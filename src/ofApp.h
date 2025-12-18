#pragma once

#include "ofMain.h"
#include "ofxNDIFinder.h"
#include "ofxNDIReceiver.h"
#include "ofxNDIRecvStream.h"
#include "ofxGui.h"

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

	private:
		void connectToSourceIndex(int index);
		bool connectToSource(const ofxNDI::Source &src);
		void saveSelectedSource(const ofxNDI::Source &src);
		void loadSavedSource();
		void ensureDataFolder();
		void computeSimRes();
		void setupCascade();
		void rebuildCascade();
		void initParticles();
		void updateParticles(float dt);
		void drawCascade();

		ofxNDIFinder finder_;
		ofxNDIReceiver receiver_;
		ofxNDIRecvVideoFrameSync video_;

		ofPixels pixels_;
		ofTexture texture_;
		bool autoConnected_ = false;
		bool hasFrame_ = false;
		bool videoSetup_ = false;

		std::string savedSourceName_;
		std::string currentSourceName_;
		bool showMask_ = true;

		// Cascade simulation
		ofFbo ping_[2];
		int curPing_ = 0;
		glm::ivec2 simRes_{160, 480};
		bool cascadeAllocated_ = false;
		ofVboMesh particleMesh_;
		ofShader updateShader_;
		ofShader renderShader_;
		bool particlesReady_ = false;
		bool shadersLoaded_ = false;
		bool collide_ = true; // collisions against mask on by default

		float gravity_ = 3.5f;
		float threshold_ = 0.45f;
		float noiseStrength_ = 0.8f;
		bool invertMask_ = false;
		float pointSize_ = 5.0f;
		float topBias_ = 0.0f;
		float bounceDampen_ = 0.5f;
		bool renderSquares_ = false;

		ofRectangle maskDrawRect_;
		float simDensity_ = 0.15f; // particles per pixel on width (lighter)
		glm::ivec2 lastInitRes_{0,0};

		// GUI
		ofxPanel gui_;
		bool showGui_ = true;
		ofParameter<float> pGravity_;
		ofParameter<float> pNoise_;
		ofParameter<float> pThreshold_;
		ofParameter<float> pPointSize_;
		ofParameter<float> pSimDensity_;
		ofParameter<float> pTopBias_;
		ofParameter<float> pBounceDampen_;
		ofParameter<bool> pCollide_;
		ofParameter<bool> pInvertMask_;
		ofParameter<bool> pShowMask_;
		ofParameter<bool> pRenderSquares_;
		
};
