#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    
    ofSetFrameRate(20);
    
    receiver.setup(PORT_0);
    sender_1.setup(HOST_1, PORT_1);
    sender_2.setup(HOST_2, PORT_2);
    sender_3.setup(HOST_3, PORT_3);
    
    gui.setup();
    font.loadFont("Akkurat-Mono.ttf", 10);
    gui.add(indice.setup( "indice", 0, 0, 100 ));
    
    gui.add(thermal_delta_y.setup( "thermal_delta_y", 0, 0, 300 ));
    gui.add(thermal_target_x.setup( "thermal_target_x", 0.5, 0., 1. ));
    gui.add(thermal_target_y.setup( "thermal_target_y", 0.5, 0., 1. ));
    gui.add(heart_threshold.setup( "heart_threshold", 400, 100, 600));
    gui.add(bFake.setup("fake heart", false));
    
    thermal_target_delta_x = 0;
    thermal_target_delta_x = 0;
    
    stress = 0;
    
    lastBeat = ofGetElapsedTimeMillis();
    beatN  = 0;
    beatAvg = 0;
    
    ofBuffer buffer = ofBufferFromFile("beat.txt");
    
    while(!buffer.isLastLine())
        fake_beats.push_back(ofToInt(buffer. getNextLine()));
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if(ofGetFrameNum() % 3 == 0){
        ofxOscMessage m;
        m.setAddress("/thermal");
        m.addIntArg(thermal_delta_y);
        m.addFloatArg(thermal_target_x + thermal_target_delta_x);
        m.addFloatArg(thermal_target_y + thermal_target_delta_y);
        sender_1.sendMessage(m);
    
        thermal_target_delta_x = ofMap( ofNoise(0., ofGetFrameNum() / 100.), 0., 1., -0.1, 0.1);
        thermal_target_delta_y = ofMap( ofNoise(1., ofGetFrameNum() / 100.), 0., 1., -0.1, 0.1);
    }
    
    stress *= 0.95;
    
    if(bStress)
        stress += 5;
    if(bHStress)
        stress += 20;
    
    stress = ofClamp(stress, 0, 100);
    
    if(ofGetFrameNum() % 30 == 0){
        stress += ofRandom(10);
    }
    
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        if(m.getAddress() == "/heart"){
            
                beats.push_back(m.getArgAsInt32(0));
            
            if(!bFake){
                sender_2.sendMessage(m);
            }
            else{
                ofxOscMessage fake_m;
                fake_m.setAddress("/heart");
                fake_m.addIntArg(fake_beats[fake_beats_index]);
                sender_2.sendMessage(fake_m);
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
                        ofxJSONElement response;
                        response.open("http://192.168.1.42:3000/heartRate.json?v=" + ofToString(value));
                        beatAvg += value;
                        beatN ++;
                    }
                    lastBeat = now;
                }

        }
    }


}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(20);

    if(ofGetFrameNum() % 60 == 0){
        user.open("http://192.168.1.42:3000/last.json");
    }
    
    if(ofGetFrameNum() % 5 == 0){
         ofxJSONElement response;
        response.open("http://192.168.1.42:3000/stress.json?v=" + ofToString(int(stress)));
        response.open("http://192.168.1.42:3000/indice.json?v="+ofToString(int(indice)));
    }
    
    int avg = 60;
    if(beatN > 0)
        avg = beatAvg / beatN;
    
    ofPushMatrix();
    string msg = user["name"].asString() +  " " + user["active"].asString()  +  " " + user["indice"].asString()  +
                "\n" + ofToString(avg) +
                "\n[M/F] male/female \n[I/E] activate/deactivate";
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
    
    
    ofxJSONElement response;
    switch (key) {
        case 'M':
            response.open("http://192.168.1.42:3000/last.json?male=1");
            break;
        case 'F':
            response.open("http://192.168.1.42:3000/last.json?male=0");
            break;
        case 'E':
            response.open("http://192.168.1.42:3000/active.json?active=0");
            break;
        case 'I':
            response.open("http://192.168.1.42:3000/active.json?active=1");
            indice = 0;
            break;
            
        case '1':
            bStress = true;
            break;
        case '2':
            bHStress = true;
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
