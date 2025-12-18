#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex(); // use normalized coords for GLSL sampling
	NDIlib_initialize();
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofBackground(0);

	ensureDataFolder();
	computeSimRes();
	loadSavedSource();
	finder_.watchSources();
	ofLogNotice("NEXT2VISUALS") << "data path: " << ofToDataPath("", true);
	setupCascade();
}

//--------------------------------------------------------------
void ofApp::update(){
	auto sources = finder_.getSources();

	if(!receiver_.isConnected()) {
		bool connected = false;

		if(!savedSourceName_.empty()) {
			auto it = std::find_if(sources.begin(), sources.end(), [&](const ofxNDI::Source &src){
				return src.p_ndi_name == savedSourceName_;
			});
			if(it != sources.end()) {
				connected = connectToSource(*it);
			}
		}

		if(!connected && !sources.empty()) {
			connected = connectToSource(sources.front());
		}

		if(connected) {
			autoConnected_ = true;
		} else if(sources.empty()) {
			// log once in a while
			static uint64_t lastLogMs = 0;
			uint64_t now = ofGetElapsedTimeMillis();
			if(now - lastLogMs > 2000) {
				ofLogNotice("NEXT2VISUALS") << "No NDI sources found yet...";
				lastLogMs = now;
			}
		}
	}

	if(receiver_.isConnected()) {
		video_.update();
		if(video_.isFrameNew()) {
			video_.decodeTo(pixels_);
			if(pixels_.isAllocated()) {
				if(!texture_.isAllocated() || texture_.getWidth() != pixels_.getWidth() || texture_.getHeight() != pixels_.getHeight()) {
					texture_.allocate(pixels_);
				}
				texture_.loadData(pixels_);
				hasFrame_ = true;
				// draw occupies full window
				maskDrawRect_.set(0, 0, ofGetWidth(), ofGetHeight());
				if(!particlesReady_) {
					initParticles();
				}
			}
		}
	}

	if(particlesReady_) {
		updateParticles(ofGetLastFrameTime());
	}
	}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	if(hasFrame_ && texture_.isAllocated() && showMask_) {
		if(maskDrawRect_.isEmpty()) {
			texture_.draw(0, 0, ofGetWidth(), ofGetHeight());
		} else {
			texture_.draw(maskDrawRect_);
		}
	} else {
		ofSetColor(255);
		std::string status = "Buscando fuente NDI...\nPulsa 1-9 para conectar a una fuente listada.";
		if(!currentSourceName_.empty()) {
			status = "Fuente actual: " + currentSourceName_ + "\nSi no ves imagen, pulsa 1-9 para reconectar.";
			if(!showMask_) {
				status += "\n(Máscara oculta: tecla M para mostrar)";
			}
		}
		ofDrawBitmapStringHighlight(status, 20, 30);
		ofDrawBitmapStringHighlight("Sources detectados: " + ofToString(finder_.getSources().size()), 20, 60);
		ofDrawBitmapStringHighlight("Shaders: " + string(shadersLoaded_ ? "OK" : "NO"), 20, 80);
		ofDrawBitmapStringHighlight("Particles: " + string(particlesReady_ ? "READY" : "NO"), 20, 100);
		auto sources = finder_.getSources();
		for(size_t i = 0; i < sources.size(); ++i) {
			const auto &src = sources[i];
			ofDrawBitmapStringHighlight(ofToString(i+1) + ": " + src.p_ndi_name, 20, 140 + i * 20);
		}
	}

	if(particlesReady_) {
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		drawCascade();
		ofDisableBlendMode();
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
	if(receiver_.isConnected()) {
		receiver_.disconnect();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key >= '1' && key <= '9') {
		connectToSourceIndex(key - '1');
	}
	if(key == 'm' || key == 'M') {
		showMask_ = !showMask_;
		ofLogNotice("NEXT2VISUALS") << "Mostrar máscara: " << (showMask_ ? "ON" : "OFF");
	}
	if(key == 'c' || key == 'C') {
		collide_ = !collide_;
		ofLogNotice("NEXT2VISUALS") << "Colisiones máscara: " << (collide_ ? "ON" : "OFF");
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	computeSimRes();
	rebuildCascade();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::connectToSourceIndex(int index){
	auto sources = finder_.getSources();
	if(index < 0 || index >= static_cast<int>(sources.size())) return;

	connectToSource(sources[index]);
}

//--------------------------------------------------------------
bool ofApp::connectToSource(const ofxNDI::Source &src){
	bool connected = false;
	bool wasSetup = receiver_.isSetup();

	if(wasSetup) {
		receiver_.changeConnection(src);
		connected = true;
	} else {
		connected = receiver_.setup(src);
	}

	if(connected) {
		ofLogNotice("NEXT2VISUALS") << "Conectado a " << src.p_ndi_name << " (" << src.p_url_address << ")";
		if(!videoSetup_) {
			video_.setup(receiver_);
			videoSetup_ = true;
		} else {
			// ensure frame sync uses current receiver after changeConnection
			video_.setup(receiver_);
		}
		hasFrame_ = false;
		autoConnected_ = true;
		savedSourceName_ = src.p_ndi_name;
		currentSourceName_ = src.p_ndi_name;
		saveSelectedSource(src);
	}

	return connected;
}

//--------------------------------------------------------------
void ofApp::saveSelectedSource(const ofxNDI::Source &src){
	ofJson j;
	j["ndi_name"] = src.p_ndi_name;
	j["ndi_url"] = src.p_url_address;
	ofSavePrettyJson("ndi_source.json", j); // relative to data folder
}

//--------------------------------------------------------------
void ofApp::loadSavedSource(){
	auto path = ofToDataPath("ndi_source.json", true);
	if(!ofFile::doesFileExist(path)) {
		ofLogNotice("NEXT2VISUALS") << "No saved source found at " << path;
		return;
	}
	ofJson j = ofLoadJson("ndi_source.json");
	if(j.contains("ndi_name") && j["ndi_name"].is_string()) {
		savedSourceName_ = j["ndi_name"].get<std::string>();
		currentSourceName_ = savedSourceName_;
		ofLogNotice("NEXT2VISUALS") << "Saved source: " << savedSourceName_;
	}
}

//--------------------------------------------------------------
void ofApp::ensureDataFolder(){
	auto dataPath = ofToDataPath("", true);
	ofDirectory dir(dataPath);
	if(!dir.exists()) {
		dir.create(true);
	}
}

//--------------------------------------------------------------
void ofApp::setupCascade(){
	computeSimRes();

	// load shaders once
	if(!shadersLoaded_) {
		bool updOk = updateShader_.load("shaders/update");
		bool rndOk = renderShader_.load("shaders/render");
		shadersLoaded_ = updOk && rndOk;
		if(!shadersLoaded_) {
			ofLogError("NEXT2VISUALS") << "Shaders failed to load.";
			return;
		} else {
			ofLogNotice("NEXT2VISUALS") << "Shaders loaded.";
		}
	}

	rebuildCascade();
}

//--------------------------------------------------------------
void ofApp::computeSimRes(){
	int w = std::max(200, std::min(1000, int(ofGetWidth() * simDensity_)));
	int h = std::max(300, int(float(w) * (float(ofGetHeight())/float(ofGetWidth()))));
	simRes_.x = w;
	simRes_.y = h;
}

//--------------------------------------------------------------
void ofApp::rebuildCascade(){
	ofFbo::Settings s;
	s.width = simRes_.x;
	s.height = simRes_.y;
	s.internalformat = GL_RGBA32F;
	s.useDepth = false;
	s.useStencil = false;
	s.textureTarget = GL_TEXTURE_2D;
	s.minFilter = GL_NEAREST;
	s.maxFilter = GL_NEAREST;
	s.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
	s.wrapModeVertical = GL_CLAMP_TO_EDGE;

	for(auto &fbo : ping_) {
		fbo.allocate(s);
		fbo.begin(); ofClear(0,0,0,0); fbo.end();
	}

	particleMesh_.setMode(OF_PRIMITIVE_POINTS);
	particleMesh_.getVertices().reserve(simRes_.x * simRes_.y);
	particleMesh_.getTexCoords().reserve(simRes_.x * simRes_.y);
	particleMesh_.clear();
	for(int y=0; y<simRes_.y; ++y){
		for(int x=0; x<simRes_.x; ++x){
			particleMesh_.addVertex(glm::vec3(0,0,0)); // actual position set in shader
			particleMesh_.addTexCoord(glm::vec2((x + 0.5f) / simRes_.x, (y + 0.5f) / simRes_.y));
		}
	}

	initParticles(); // start particles even if mask no frame yet
	cascadeAllocated_ = true;
}

//--------------------------------------------------------------
void ofApp::initParticles(){
	ofFloatPixels pix;
	pix.allocate(simRes_.x, simRes_.y, 4);
	for(int y=0; y<simRes_.y; ++y){
		for(int x=0; x<simRes_.x; ++x){
			float fx = ofRandom(1.0f);
			float fy = ofRandom(-0.05f, 0.05f); // start near top
			float vx = ofRandomf() * 0.005f;
			float vy = 0.0f;
			int idx = (y * simRes_.x + x) * 4;
			pix[idx + 0] = fx;
			pix[idx + 1] = fy;
			pix[idx + 2] = vx;
			pix[idx + 3] = vy;
		}
	}
	ping_[0].getTexture().loadData(pix);
	ping_[1].getTexture().loadData(pix);
	particlesReady_ = true;
	if(simRes_ != lastInitRes_) {
		ofLogNotice("NEXT2VISUALS") << "Particles initialized: " << simRes_.x << " x " << simRes_.y;
		lastInitRes_ = simRes_;
	}
}

//--------------------------------------------------------------
void ofApp::updateParticles(float dt){
	if(!shadersLoaded_) return;

	bool maskReady = texture_.isAllocated();
	bool useMask = collide_ && maskReady;

	ping_[1 - curPing_].begin();
	ofClear(0,0,0,0);
	updateShader_.begin();
	updateShader_.setUniformTexture("posTex", ping_[curPing_].getTexture(), 0);
	if(maskReady) {
		updateShader_.setUniformTexture("maskTex", texture_, 1);
	} else {
		// fallback dummy binding to avoid undefined sampler
		updateShader_.setUniformTexture("maskTex", ping_[curPing_].getTexture(), 1);
	}
	updateShader_.setUniform2f("posRes", simRes_.x, simRes_.y);
	updateShader_.setUniform2f("screenRes", ofGetWidth(), ofGetHeight());
	updateShader_.setUniform1f("dt", dt);
	updateShader_.setUniform1f("gravity", gravity_);
	updateShader_.setUniform1f("threshold", threshold_);
	updateShader_.setUniform1f("noiseStrength", noiseStrength_);
	updateShader_.setUniform1i("collide", useMask ? 1 : 0);
	updateShader_.setUniform1i("invertMask", invertMask_ ? 1 : 0);
	updateShader_.setUniform1f("time", ofGetElapsedTimef());
	ping_[curPing_].draw(0,0);
	updateShader_.end();
	ping_[1 - curPing_].end();

	curPing_ = 1 - curPing_;
}

//--------------------------------------------------------------
void ofApp::drawCascade(){
	if(!shadersLoaded_) return;

	ofPushStyle();
	ofEnablePointSprites();
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	renderShader_.begin();
	renderShader_.setUniformTexture("posTex", ping_[curPing_].getTexture(), 0);
	renderShader_.setUniform2f("posRes", simRes_.x, simRes_.y);
	renderShader_.setUniform2f("screenRes", ofGetWidth(), ofGetHeight());
	renderShader_.setUniform1f("pointSize", pointSize_);
	renderShader_.setUniform1f("time", ofGetElapsedTimef());
	particleMesh_.draw();
	renderShader_.end();
	glDisable(GL_PROGRAM_POINT_SIZE);
	ofDisablePointSprites();
	ofPopStyle();
}
