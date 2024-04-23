#include "DrawableEnvironment.h"
#include "myAudioUtilities.h"
#include "generators.h"


#include "std_lib_facilities.h"
#include "AnimationWindow.h"

//helper function for the grapher window to retrive offsetts from a String
float fetchY(const StringSegment<float>& seg){
    return seg.y;
}

int main() {
    DrawableEnvironment env;
    AudioStream austr(44100, 4000);

    String<float> stringsim;
    //change the stepSize to make the simulation faster/slower
    //stringsim.stepSize /= 512.f;

    grapher gra(150, -1, 1, {{-70, -70},{70, 70}});
    env.bind(gra);

    Pick lastpick = {0,0,0}; //used to interpolate between pick positions

    while(!env.getwin().should_close()){
        //generate new pick
        Pick thispick;
        thispick.active = gra.localHeld;
        if(thispick.active){
            thispick.pos = gra.localMousePosition(env);
            thispick.pos.y = thispick.pos.y*(gra.maxy-gra.miny)+gra.miny;
            thispick.radius = 0.015f;
        }

        //simulate and queue audio samples, interpolating picks
        uint numtoQueue = std::min(austr.numQueuedIn(env.getFrameTime()), 10000u);
        uint substeps = 1;
        vec2 pickstep = (thispick.pos-lastpick.pos)*(1.f/float(numtoQueue*substeps));
        for(uint i = 0; i<numtoQueue; ++i){
            float samp = 0;
            for(int s = 0; s<substeps; ++s){
                lastpick.pos += pickstep;
                samp = stringsim.stepStroked(lastpick);
            }
            austr.queueSample(samp);
        }
        lastpick = thispick;

        //user communication
        gra.load(stringsim.string, fetchY);
        env.control();
        env.render();
    }
    return 0;
}