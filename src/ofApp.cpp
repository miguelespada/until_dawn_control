#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    font.loadFont("Akkurat-Mono.ttf", 10);
    
    ofSetFrameRate(20);
    
    receiver.setup(PORT_0);
    sender_1.setup(HOST_1, PORT_1);
    sender_2.setup(HOST_2, PORT_2);
    sender_3.setup(HOST_3, PORT_3);
    
    local_sender_1.setup(HOST_1, PORT_1);
    local_sender_2.setup(HOST_2, PORT_2);
    local_sender_3.setup(HOST_3, PORT_3);
    
    gui.setup();
    
    
    gui.add(indice.setup( "indice", 0, 0, 100 ));
    
    gui.add(thermal_target_x.setup( "thermal_target_x", 0.5, 0., 1. ));
    gui.add(thermal_target_y.setup( "thermal_target_y", 0.5, 0., 1. ));
    gui.add(heart_threshold.setup( "heart_threshold", 400, 100, 600));
    gui.add(bFake.setup("fake heart", false));
    
    
    //
    thermal_target_delta_x = 0;
    thermal_target_delta_x = 0;
    
    //
    stress = 0;
    
    //
    lastBeat = ofGetElapsedTimeMillis();
    beatN  = 0;
    beatAvg = 0;
    
    ofBuffer buffer = ofBufferFromFile("beat.txt");
    
    while(!buffer.isLastLine())
        fake_beats.push_back(ofToInt(buffer. getNextLine()));
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    runningTime = ofGetElapsedTimef() - initTime;
    
    updatePositions();
    updateStress();
    
    parseOsc();
    
    saveData();
}

//--------------------------------------------------------------
void ofApp::draw(){
    if(bSave)
        ofClear(50, 20, 20);
    else
        ofClear(20);
    
    ofPushMatrix();
    string msg = "Name: " + user["name"].asString() +
    " " +"isMale? [M/F]: " + ofToString(user["male"]) +
    "Running time: " + ofToString(user["runningTime"]) +
    "Stress: " + ofToString(user["stress"][user["stress"].size() - 1].asInt()) +  " " +
                "Indice: " + user["indice"].asString()  +
    "\n" + "Last beat: " + ofToString(user["heartRate"][user["heartRate"].size() - 1].asInt());
    ofTranslate(10, 300);
    font.drawString(msg, 0, 0);
    ofPopMatrix();

    int x = ofGetFrameNum()  % ofGetWidth();
    ofLine(x, ofGetHeight(), x, ofGetHeight() - stress);
    
    ofPushMatrix();
    ofTranslate(0, -200);
    ofLine(0, heart_threshold, ofGetWidth(), heart_threshold);
    if(beats.size() > 0){
        for(int i = 1; i < beats.size(); i ++){
            ofLine(i, 0, i, beats[i]);
        }
    }
    ofPopMatrix();
    
    
    gui.draw();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch (key) {
        case 'M':
            user.open("http://192.168.1.42:3000/last.json?male=1");
            break;
        case 'F':
            user.open("http://192.168.1.42:3000/last.json?male=0");
            break;
        case 'r':
            reset();
            break;
        case 'i':
            idle();
            break;
        case 'n':
            next();
            break;
            
        case '=':
            index();
            break;
            
            
        case '0':
            calculandoIndex();
            break;
            
            
        case '1':
            bStress = true;
            break;
        case '2':
            bHStress = true;
            break;
            
            
        case 'S':
            bSave = !bSave;
            saveFrames();
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch (key) {
        case '1':
            bStress = false;
            break;
        case '2':
            bHStress = false;
            break;
            
        default:
            break;
    }

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::reset(){
    
    user.open("http://192.168.1.42:3000/reset");
    
    initTime = ofGetElapsedTimef();
    
    //
    thermal_target_delta_x = 0;
    thermal_target_delta_x = 0;
    
    //
    stress = 0;
    
    //
    lastBeat = ofGetElapsedTimeMillis();
    beatN  = 0;
    beatAvg = 0;
    beats.clear();
}

void ofApp::idle(){
    ofxOscMessage m;
    m.setAddress("/standby");
    sendAll(m);
}

void ofApp::next(){
    ofxOscMessage m;
    m.setAddress("/next");
    sendAll(m);
}

void ofApp::index(){
    ofxOscMessage m;
    m.setAddress("/index");
    sendAll(m);
}

void ofApp::calculandoIndex(){
    ofxOscMessage m;
    m.setAddress("/calculandoIndex");
    sendAll(m);
}

void ofApp::sendAll(ofxOscMessage m){
    sender_1.sendMessage(m);
    sender_2.sendMessage(m);
    sender_3.sendMessage(m);
    
    if(LOCAL){
        local_sender_1.sendMessage(m);
        local_sender_2.sendMessage(m);
        local_sender_3.sendMessage(m);
    }
}

void ofApp::updateBeat(ofxOscMessage m){
    beats.push_back(m.getArgAsInt32(0));
    
    if(!bFake){
        sender_2.sendMessage(m);
        local_sender_2.sendMessage(m);
    }
    else{
        ofxOscMessage fake_m;
        fake_m.setAddress("/heart");
        fake_m.addIntArg(fake_beats[fake_beats_index]);
        sender_2.sendMessage(fake_m);
        if(LOCAL)
            local_sender_2.sendMessage(m);
        
        fake_beats_index = (fake_beats_index + 1) % fake_beats.size();
    }
    
    if(beats.size() > 400){
        beats.erase(beats.begin());
    }
    if(beats[beats.size() - 1] > heart_threshold && beats[beats.size() - 2] < heart_threshold){
        int value;
        float now = ofGetElapsedTimeMillis();
        value = int(60000. / (now - lastBeat));
        if(value > 50 && value < 100){
            user.open("http://192.168.1.42:3000/heartRate.json?v=" + ofToString(value));
            beatAvg += value;
            beatN ++;
        }
        lastBeat = now;
    }

}

void ofApp::updateStress(){
    stress *= 0.95;
    
    if(bStress)
        stress += 5;
    if(bHStress)
        stress += 20;
    
    stress = ofClamp(stress, 0, 100);
    
    if(ofGetFrameNum() % 30 == 0){
        stress += ofRandom(10);
    }
}

void ofApp::saveData(){
    if(ofGetFrameNum() % 10 == 0){
        user.open("http://192.168.1.42:3000/updateControl.json?stress=" + ofToString(stress)
                  + "&indice=" + ofToString(int(indice))
                  + "&runningTime=" + ofToString(int(runningTime))
                  );
    }
}

void ofApp::updatePositions(){
    if(ofGetFrameNum() % 3 == 0){
        ofxOscMessage m;
        m.setAddress("/thermal");
        m.addFloatArg(thermal_target_x + thermal_target_delta_x);
        m.addFloatArg(thermal_target_y + thermal_target_delta_y);
        sender_3.sendMessage(m);
        
        thermal_target_delta_x = ofMap( ofNoise(0., ofGetFrameNum() / 100.), 0., 1., -0.1, 0.1);
        thermal_target_delta_y = ofMap( ofNoise(1., ofGetFrameNum() / 100.), 0., 1., -0.1, 0.1);
        
    }
}

void ofApp::parseOsc(){
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        if(m.getAddress() == "/heart"){
            updateBeat(m);
        }
        // touch osc
        else if(m.getAddress() == "/2/push1"){
            float val = m.getArgAsFloat(0);
            bStress = (val == 1.) ? true : false;
        }
    }
}

void ofApp::saveFrames(){
    ofxOscMessage m;
    m.setAddress("/save");
    m.addIntArg(int(bSave));
    sendAll(m);
}

