#include "DrawableEnvironment.h"
#include "myAudioUtilities.h"
#include "generators.h"


#include "std_lib_facilities.h"
#include "AnimationWindow.h"
#include "widgets/TextInput.h"


void optionalCallback(const std::string& str){
    std::cout<<"new value: "<<str<<"\n";
}


float fetchY(const StringSegment<float>& seg){
    return seg.y;
}

int main() {
    DrawableEnvironment env;
    AudioStream austr(44100, 4000);
    String<float> stringsim;
    //stringsim.substeps = 2;
    grapher op;
    op.miny = -1;
    op.maxy = 1;
    op.resize(300);
    op.load(stringsim.string, fetchY);
    env.bind(op);

    while(!env.getwin().should_close()){
        uint numtoQueue = std::min(austr.numQueuedIn(env.getFrameTime()), 10000u);
        for(uint i = 0; i<numtoQueue; ++i){
            austr.queueSample(stringsim.step());
        }
        op.load(stringsim.string, fetchY);
        env.control();
        env.render();
    }
    return 0;

    /*AudioStream austr(44100, 4000);
    sinusoidalGenerator sinsim;
    String<float> stringsim;
    stringsim.substeps = 2;

    uint frame = 0;
    for(int i = 0; i<200 && (!env.getwin().should_close()); ++i){
        env.control();
        env.render();
        sinsim.frequency = exp(double(env.getWorldMousePos().y)/100.)*100.;
        uint samplesToQueue = austr.numQueuedIn(env.getFrameTime());
        samplesToQueue = std::min(10000u, samplesToQueue);
        for(uint i = 0; i<samplesToQueue; ++i){
            austr.queueSample(sinsim.step()*0.9);
        }
    }

    while(!env.getwin().should_close()){
        frame++;
        env.control();
        env.render();
        sinsim.frequency = exp(double(env.getWorldMousePos().y)/100.)*100.;
        uint samplesToQueue = austr.numQueuedIn(env.getFrameTime());
        samplesToQueue = std::min(10000u, samplesToQueue);
        if(frame%20==0){
            std::cout<<"frameTime "<<env.getFrameTime()<<"\n";
            std::cout<<"generating " <<samplesToQueue<<"\n";
            std::cout<<"from "<<austr.getSampleRate()<<"\n";
        }
        for(uint i = 0; i<samplesToQueue; ++i){
            austr.queueSample(sinsim.step()*0.9);
        }
    }
    
    return 0;*/
}