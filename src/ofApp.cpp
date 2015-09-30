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
    
    local_sender_1.setup("localhost", PORT_1);
    local_sender_2.setup("localhost", PORT_2);
    local_sender_3.setup("localhost", PORT_3);
    
    arduino_sender.setup(ARDUINO_HOST, ARDUINO_PORT);
    
    gui.setup();
    
    
    gui.add(indice.setup( "indice", 50, 0, 100 ));
    
    gui.add(thermal_target_x.setup( "thermal_target_x", 0.5, 0., 1. ));
    gui.add(thermal_target_y.setup( "thermal_target_y", 0.5, 0., 1. ));
    gui.add(heart_threshold.setup( "heart_threshold", 400, 100, 600));
    
    gui.add(bFake.setup("fake heart", false));
    gui.add(heartBeat.setup( "fake heart beat", 70, 50, 100));
    
    
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
    
    temperature = 0;
    conductance = 0;
    galvanicVoltage = 0;
    
    avgFlow = 0;
    
    arduino_input = "ARD. NOT CONNTECTED";
    
    
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    runningTime = ofGetElapsedTimef() - initTime;
    
    updatePositions();
    updateFlow();
    updateStress();
    
    parseOsc();
    
    saveData();
    
    if(bFake){
        ofxOscMessage fake_m;
        fake_m.setAddress("/heart");
        int value = fake_beats[fake_beats_index];
        if(ofRandom(10) > 5)
            value += ofRandom(-50, 50);
        fake_m.addIntArg(value);
        sender_2.sendMessage(fake_m);
        if(LOCAL)
            local_sender_2.sendMessage(fake_m);
        
        fake_beats_index = (fake_beats_index + 1) % fake_beats.size();
        
        if(ofGetFrameNum() % 3 == 0)
            user.open("http://192.168.1.42:3000/heartRate.json?v=" + ofToString(int(heartBeat)));
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    if(bSave)
        ofClear(50, 20, 20);
    else
        ofClear(20);
    
    if(user["active"].asBool())
        ofSetColor(0, 255, 0);
    else
        ofSetColor(255, 0, 0);
    ofEllipse(ofGetWidth() - 50, 50, 20, 20);
    
    ofSetColor(255);
    
    ofPushMatrix();
    string msg = "Name: " + user["name"].asString() +
    "\n" + "Running time: " + ofToString(user["runningTime"]) +
    "" + "isMale? [M/F]: " + ofToString(user["male"]) +
    "" +"active? [a/A]: " + ofToString(user["active"]) +
    "Stress: " + ofToString(user["stress"][user["stress"].size() - 1].asInt()) +  "\n" +
    "Indice: " + user["indice"].asString()  + " (" + computeIndice() + ")" +
    "\n" + "Last beat: " + ofToString(user["heartRate"][user["heartRate"].size() - 1].asInt()) +
    "\n" + "pump [4/5/6]";
    ofTranslate(10, 250);
    font.drawString(msg, 0, 0);
    ofPopMatrix();

    
    font.drawString(arduino_input, 500, 20);
    
    if(bPumping){
        ofSetColor(255, 0, 0);
        font.drawString("PUMP: " + ofToString(ofGetElapsedTimef() - pumpingTime), 500, 70);
    }
    
    else if(bPumpHold){
        ofSetColor(255, 255, 0);
        font.drawString("PUMP: HOLD", 500, 70);
    }
    else {
        ofSetColor(0, 255, 0);
        font.drawString("PUMP: OPEN " + ofToString(int(ofGetElapsedTimef() - pumpingTime)), 500, 70);
    }
    ofSetColor(255);
    
    
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
    
    
    drawUserStress();
    
    
    ofSetColor(255, 0, 255);
    int x = ofGetFrameNum()  % ofGetWidth();
    ofLine(x, ofGetHeight(), x, ofGetHeight() - stress);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    ofxOscMessage m;
    switch (key) {
        case 'M':
            user.open("http://192.168.1.42:3000/last.json?male=1");
            break;
        case 'F':
            user.open("http://192.168.1.42:3000/last.json?male=0");
            break;
            
        case 'A':
            user.open("http://192.168.1.42:3000/active.json?v=0");
            break;
        case 'a':
            user.open("http://192.168.1.42:3000/active.json?v=1");
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
            
        case '4':
            m.setAddress("/pump");
            m.addIntArg(0);
            arduino_sender.sendMessage(m);
            bPumpHold = false;
            bPumping = true;
            pumpingTime = ofGetElapsedTimef();
            break;
            
        case '5':
            m.setAddress("/pump");
            m.addIntArg(1);
            arduino_sender.sendMessage(m);
            bPumpHold = true;
            bPumping = false;
            break;
            
        case '6':
            m.setAddress("/pump");
            m.addIntArg(2);
            arduino_sender.sendMessage(m);
            bPumpHold = false;
            bPumping = false;
            pumpingTime = ofGetElapsedTimef();
            break;
            
        case '=':
            index();
            break;
            
        case OF_KEY_RETURN:
            newUser();
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

            
//        case ' ':
//            bSave = !bSave;
//            saveFrames();
//            break;

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
    thermal_target_delta_x = 0.5;
    thermal_target_delta_x = 0.5;
    
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
    beats.push_back(m.getArgAsInt32(3));
    
    if(!bFake){
        ofxOscMessage real_m;
        real_m.setAddress("/heart");
        real_m.addIntArg(m.getArgAsInt32(3));
        
        sender_2.sendMessage(real_m);;
        if(LOCAL)
            local_sender_2.sendMessage(real_m);
    }
    else{
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
                  + "&temperature=" + ofToString(temperature)
                  + "&conductance=" + ofToString(galvanicVoltage)
                  + "&galvanicVoltage=" + ofToString(galvanicVoltage)
                  + "&flow=" + ofToString(avgFlow)
                  + "&thermal=" + ofToString(avgThermal)
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
        if(LOCAL)
            local_sender_3.sendMessage(m);
            
        thermal_target_delta_x = ofMap( ofNoise(0., ofGetFrameNum() / 100.), 0., 1., -0.1, 0.1);
        thermal_target_delta_y = ofMap( ofNoise(1., ofGetFrameNum() / 100.), 0., 1., -0.1, 0.1);
        
    }
}

void ofApp::updateFlow(){
    if(ofGetFrameNum() % 3 == 0){
        ofxOscMessage m;
        m.setAddress("/flow");
        m.addFloatArg(flow_threshold);
        sender_3.sendMessage(m);
        if(LOCAL)
            local_sender_3.sendMessage(m);
    }
}

void ofApp::parseOsc(){
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        if(m.getAddress() == "/data"){
            updateBeat(m);
            temperature = m.getArgAsInt32(0);
            conductance =  ofMap(m.getArgAsInt32(1), 0, 5500, 0, 100, true);
            galvanicVoltage =  ofMap(m.getArgAsInt32(2), 50, 330, 0, 100, true);
            arduino_input = "C: " + ofToString(conductance) + "\nV: " + ofToString(galvanicVoltage) + "\nT: " + ofToString(temperature / 100.);
            
        }
        if(m.getAddress() == "/flow"){
            avgFlow = m.getArgAsFloat(0);
            
            sender_2.sendMessage(m);;
            if(LOCAL)
                local_sender_2.sendMessage(m);
        }
        
        if(m.getAddress() == "/thermal"){
            avgThermal = m.getArgAsFloat(0);
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
    
    sender_3.sendMessage(m);
    if(LOCAL)
        local_sender_3.sendMessage(m);
}

string ofApp::computeIndice(){
    if(user["indice"].asInt() > 90)
        return "PANICO";
    else if (user["indice"].asInt() > 70)
        return "ESPANTO";
    else if (user["indice"].asInt() > 50)
        return "MIEDO";
    else if (user["indice"].asInt() > 25)
        return "INQUIETUD";
    else
        return "INDIFERENCIA";
}

void ofApp::newUser(){
    user.open("http://192.168.1.42:3000/newUser.json");
    reset();
    indice = 0;
    calculandoIndex();
    
}

void ofApp::drawUserStress(){
    ofPushMatrix();
    ofxJSONElement stress = user["stress"];
    ofSetColor(255, 0, 0);
    ofTranslate(00, 400);
    ofScale(800./1200, 1);
    for(int i = 0; i < stress.size(); i++){
        float v = stress[i].asFloat() / 2;
        ofLine(i, 0, i, -v);
    }
    ofSetColor(255);
    ofPopMatrix();
}
