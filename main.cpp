#include <iostream>
#include <fstream>
#include <string>

#include <dpp/dpp.h>

bool authenticate();
dpp::cluster* apiDPP;

int main(){
    if(!authenticate()){
        return 1;
    }

    apiDPP->on_slashcommand([](auto event){
        if (event.command.get_command_name() == "ping"){
            event.reply("Pong!");
        }
    });

    apiDPP->on_ready([](auto event){
        if (dpp::run_once<struct register_bot_commands>()){
            apiDPP->global_command_create(
                dpp::slashcommand("ping", "Ping pong!", apiDPP->me.id)
            );
        }
    });

    //Await events
    apiDPP->start(dpp::st_wait);

    //Free memory
    delete apiDPP;

    return 0;
}

bool authenticate(){
    std::fstream auth;
    std::string tokenDiscord;

    auth.open("./data/auth/imin.txt", std::ios::in); //"I'm in"
    if(auth.good()){
        std::getline(auth, tokenDiscord);

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
    auth.close();

    apiDPP = new dpp::cluster(tokenDiscord);

    return true;
}
