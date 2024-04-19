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

    String(){
        string.resize(300);
        string[0] = 0;
        string[string.size()-1] = 0;
        for(size_t i = 1; i<string.size()-1; ++i){
            string[i] = StringSegment<T>(string[i-1].y*0.90f + randomUnitFloat()*0.001f + 
                (0.5f-abs(0.5f-pow(float(i)/float(string.size()), 2.f)))*0.4f*((float(string.size()-i)/float(string.size()))), 0);
        }
    }

    T step(){
        //T nsegments = T(string.size());
        //T slength = length/nsegments;
        //T smass = mass*slength;
        //T snonel = nonElasticFct*snonel;
        //T saircon = 1.-1./airResistance; //the proportion of velocity that remains after linear air resistance
        //T segmentStiffness = tension/smass; //the general factor applied to each segment

        T segmentStiffness = 6400000000.f;
        T edgeConductance = 0.99f;
        T airTransferrance = 0.99999f;
        T elasticFriction = 0.01f;
        T stepSize = 1.f/80000.f;


        //edges bleed energy in that their displacement is always drawn toward 0
        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            T dylw = me.y-string[i-1].y;
            T dyhg = string[i+1].y-me.y;
            T ddy = dylw-dyhg;
            me.dy -= ddy*segmentStiffness*stepSize;
            me.dy *= airTransferrance;
            me.y -= ddy*stepSize*elasticFriction;
        }
        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            me.y += me.dy * stepSize;
        }
        string[1].y               *= edgeConductance;
        string[string.size()-2].y   *= edgeConductance;

        return (string[1].y-string[string.size()-2].y)*10;
    }

    T stepStroked(const Pick& pick){
        T segmentStiffness = 6400000000.f;
        T edgeConductance = 0.99f;
        T airTransferrance = 0.99999f;
        T elasticFriction = 0.01f;
        T stepSize = 1.f/80000.f;


        //edges bleed energy in that their displacement is always drawn toward 0
        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            T dylw = me.y-string[i-1].y;
            T dyhg = string[i+1].y-me.y;
            T ddy = dylw-dyhg;
            me.dy -= ddy*segmentStiffness*stepSize;
            me.dy *= airTransferrance;
            me.y -= ddy*stepSize*elasticFriction;
        }


        if(pick.active){
            uint pickx = (uint)round(pick.pos.x*(float)(string.size()-1));

            if(pickx > 0 && pickx < string.size()-1) 
            if(abs(string[pickx].y-pick.pos.y)<=pick.radius){
                if(abs(string[pickx].dy)>pick.radius*3) string[pickx].dy=0;
                string[pickx].dy *= 0.5;
                string[pickx].y = pick.pos.y+pick.radius*((pick.pos.y>string[pickx].y)? -0.99 : 0.99);
            }
        }




        for(size_t i = 1; i<string.size()-1; ++i){
            StringSegment<T>& me = string[i];
            me.y += me.dy * stepSize;
        }
        string[1].y               *= edgeConductance;
        string[string.size()-2].y   *= edgeConductance;
        return (string[1].y-string[string.size()-2].y)*10;
    }
};