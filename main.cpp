#include<Gorgon/Main.h>
#include<Gorgon/Window.h>
#include<Gorgon/Graphics.h>
#include<Gorgon/Graphics/Layer.h>
#include<Gorgon/Geometry.h>
#include"GameClass.h"
#include"AudioManager.h"





int main(){ 
    Initialize("1");
    auto& aud_manager=AudioManager::Get();
    Game game(1600,900,"Tank Shooter");

    bool isGameRunning=true;
    float Time=0.0f;
    while (isGameRunning)
    {
        float deltaTime=static_cast<float>(Gorgon::Time::DeltaTime())/1000.0f;
        Time+=deltaTime;
        
        game.UpdateGame(deltaTime);
        game.ClearInputMap();

        Gorgon::NextFrame();
    }
    


    return 0;
}