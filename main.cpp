#include <iostream>
#include <conio.h>
#include <fstream>
#include <string>

bool authenticate();

int main(){
    if(!authenticate()){
        return 1;
    }

    return 0;
}

bool authenticate(){
    std::fstream auth;
    std::string tokenDiscord;

    auth.open("./data/auth/imin.txt", std::ios::in); //"I'm in"
    if(auth.is_open()){
        std::getline(auth, tokenDiscord);

        std::cout<<tokenDiscord;

        auth.close();
    }
    else{
        auth.clear();

        auth.open("./data/auth/imin.txt", std::ios::out);
        
        if(!auth.good()){
            std::cout<<"Authentication file failed to open"<<std::endl;

            return false;
        }

        std::cin.sync();

        std::cout<<"Please provide your Discord bot's token: ";

        std::cin>>tokenDiscord;
        std::cin.sync();

        auth<<tokenDiscord<<'\n';
    }

    return true;
}
