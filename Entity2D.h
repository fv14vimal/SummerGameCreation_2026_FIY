#pragma once
#include "DxPlus/DxPlus.h"


struct Entity2D
{
    DxPlus::Vec2 position;  
    DxPlus::Vec2 velocity;  
    int spriteID;           
    bool isActive;          

    
    int animStepIndex;      
    int animTimer;          
    int spriteIndex;        
};