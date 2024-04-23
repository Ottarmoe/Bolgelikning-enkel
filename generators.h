#pragma once

#include <math.h>
#include <vector>
#include <string>
#define _USE_MATH_DEFINES

#include "myvecs.h"

struct sinusoidalGenerator{
    double phase = 0;
    double sampleRate = 44100.;
    double frequency = 440;

    double step(){
        phase += frequency*2.*M_PI/double(sampleRate);
        if(phase > 2.*M_PI) phase -= 2.*M_PI;
        return sin(phase);
    }
};


struct Pick{
    bool active;
    vec2 pos;
    float radius;
};

template<typename T>
struct StringSegment{
    T y, dy;
    StringSegment(T ny = 0, T nv = 0) : y(ny), dy(nv){}
};

template<typename T>
struct String{
    std::vector<StringSegment<T>> string;
    //T mass; //in kg
    //T tension; //in N
    //T length; //in m

    //T airResistance; //in N/(m*(m/s))
    //T nonElasticFct; //m^-1 ?
    //T edgeConductance;


    T stepSize = 1.f/44100.f;

    //these variables were derived through trial and error
    //each assumes the length of a segment is 1, 
    //so they do not scale to give the same behaviour for different string sizes

    T segmentStiffness = 1600000000.f; 
    //stiffness factor in wave equation
    //equal to the square of the speed of sound.

    T edgeResistance = 400.f; 
    //degree to which the edges are pulled toward zero. Simulates losses to the instrument body
    
    T airResistance = 0.2f; 
    //amount of air resistance proportional to velocity. 

    T elasticFriction = 0.02f; 
    //simulates the heat equation alongside the wave equation
    //to emulate loss of high-frequency oscilations to elastic friction.
    //essentially, the curve of a segment, besides contributing to its acceleration
    //also causes an inwards offsett proportional to elasticFriction.
    //this deletes energy, and should  disproportionally effect high frequencies

    String(){
        uint sc = 300;
        string.resize(sc);
        string[0] = 0;
        string[sc-1] = 0;
        for(size_t i = 1; i<sc-1; ++i){ //a function to generate a nice initial stroke shape
            string[i] = StringSegment<T>(string[i-1].y*0.90f + randomUnitFloat()*0.01f + 
                (0.5f-abs(0.5f-pow(float(i)/float(sc), 2.f)))*0.4f*((float(sc-i)/float(sc))), 0);
        }
    }

    T stepNoFriction(){
        //update velocities from curvatures
        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            T dylw = me.y-string[i-1].y;
            T dyhg = string[i+1].y-me.y;
            T ddy = dylw-dyhg;
            me.dy -= ddy*segmentStiffness*stepSize;
        }
        //update positions based on velocities
        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            me.y += me.dy * stepSize;
        }

        return (string[1].y-string[string.size()-2].y)*10;
    }


    T stepStroked(const Pick& pick){

        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            T dylw = me.y-string[i-1].y;
            T dyhg = string[i+1].y-me.y;
            T ddy = dylw-dyhg;
            me.dy -= ddy*segmentStiffness*stepSize;
            me.dy -= me.dy*airResistance*stepSize;
            me.y -= ddy*stepSize*elasticFriction;
        }


        if(pick.active){
            uint pickx = (uint)round(pick.pos.x*(float)(string.size()-1));

            if(pickx > 0 && pickx < string.size()-1) 
            if(abs(string[pickx].y-pick.pos.y)<=pick.radius){
                if(abs(string[pickx].dy)>pick.radius*3) string[pickx].dy=0;
                string[pickx].dy *= 0.5;
                string[pickx].y = pick.pos.y+pick.radius*((pick.pos.y>string[pickx].y)? -0.99 : 0.99);
                //changing these out for -1 : 1 gets rid of fun energy preserving behaviour when holding the string at a point
            }
        }


        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            me.y += me.dy * stepSize;
        }
        string[1].y                 -= string[1].y*edgeResistance*stepSize;
        string[string.size()-2].y   -= string[string.size()-2].y*edgeResistance*stepSize;
        return (string[1].y-string[string.size()-2].y)*10;
    }
};