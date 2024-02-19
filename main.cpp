#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <complex>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>

#include <dpp/dpp.h>
#include <MatlabEngine.hpp>
#include <MatlabDataArray.hpp>

bool authenticate();
bool initializeMATLAB();
void threadFFT(const dpp::snowflake channelID, const dpp::voiceconn* voicePtr);
std::unique_ptr<dpp::cluster> apiDPP;
std::unique_ptr<matlab::engine::MATLABEngine> apiMATLAB;
std::unique_ptr<matlab::data::ArrayFactory> apiMATrix;
std::atomic<bool> activeFFT = false;

int main(){
    if(!authenticate()||!initializeMATLAB()){
        return 1;
    }

    apiDPP->on_log(dpp::utility::cout_logger());

    apiDPP->on_slashcommand([](const dpp::slashcommand_t& event){
        if(event.command.get_command_name()=="ping"){
            event.reply("pong");
        }
        else if(event.command.get_command_name()=="repo"){
            event.reply("https://github.com/trackpadpro/Def13");
        }
        else if(event.command.get_command_name()=="eig"){
            std::string buffer = "For the matrix\n";
            unsigned short n = rand()%3+2;
            matlab::data::TypedArray<float> A = apiMATrix->createArray<float>({n, n});
            std::stringstream ss;
            short i, j;

            ss<<"```";
            for(i = 0; i<n; i++){
                ss<<'|';
                for(j = 0; j<n-1; j++){
                    A[i][j] = rand()%40-20;

                    ss<<std::setw(3)<<A[i][j]<<' ';
                }
                A[i][n-1] = rand()%40-20;

                ss<<std::setw(3)<<A[i][n-1]<<" |\n";
            }
            
            ss<<"```the eigenvalues are\n```";
            buffer += ss.str();

            matlab::data::Array eT = apiMATLAB->feval(u"eig", A);
            
            try{
                matlab::data::TypedArray<float> e = eT;

                i = 0;
                for(auto& it:e){
                    buffer += std::to_string(it);
                    if(++i<n)
                    {
                        buffer += ", ";
                    }
                }
            }
            catch(matlab::data::InvalidArrayTypeException){
                //matlab::data::InvalidArrayTypeException not thrown with future result created with fevalAsync()
                matlab::data::TypedArray<std::complex<float>> e = eT;

                i = 0;
                for(auto& it:e){
                    buffer += std::to_string(real(it));
                    if(imag(it)<0){
                        buffer += "-j"+std::to_string(abs(imag(it)));
                    }
                    else if(imag(it)>0){
                        buffer += "+j"+std::to_string(imag(it));
                    }
                    if(++i<n)
                    {
                        buffer += ", ";
                    }
                }
            }

            buffer += "```";

            event.reply(buffer);
        }
        else if(event.command.get_command_name()=="fft"){
            dpp::guild* g = dpp::find_guild(event.command.guild_id);
            if(!g->connect_member_voice(event.command.get_issuing_user().id)){
                event.reply("unable to connect to user's voice channel");

                return;
            }

            dpp::voiceconn* v = event.from->get_voice(event.command.guild_id);

            event.reply("beginning fft");

            std::thread t(threadFFT, event.command.channel_id, v);
            t.detach();
        }
        else if(event.command.get_command_name()=="end"){
            activeFFT = false;

            std::this_thread::sleep_for(std::chrono::milliseconds(777)); //Allow thread processes to end

            event.from->disconnect_voice(event.command.guild_id);

            event.reply(":saluting_face:");
        }
    });

    apiDPP->on_voice_receive([](const dpp::voice_receive_t& event){
        if(activeFFT){
            std::basic_string<uint8_t> audio = event.audio_data;

            std::cout<<audio.data()<<std::endl;
        }
    });

    apiDPP->on_ready([](const dpp::ready_t& event){
        if(dpp::run_once<struct setStatus>()){
            apiDPP->set_presence(dpp::presence(dpp::ps_online, dpp::at_custom, "Lounging"));
        }

        if(dpp::run_once<struct registerBotCommands>()){
            apiDPP->global_bulk_command_create({
                dpp::slashcommand("ping", "ping pong", apiDPP->me.id),
                dpp::slashcommand("repo", "source code", apiDPP->me.id),
                dpp::slashcommand("eig", "eigenvalue of matrix", apiDPP->me.id),
                dpp::slashcommand("fft", "conducts a fast Fourier transform on your voice", apiDPP->me.id),
                dpp::slashcommand("end", "leave voice channel", apiDPP->me.id)
            }); 
        }
    });

    //Await events
    apiDPP->start(dpp::st_wait);

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

    apiDPP = std::make_unique<dpp::cluster>(tokenDiscord);

    return true;
}

bool initializeMATLAB(){
    try{
        std::vector<matlab::engine::String> MATLABsessions = matlab::engine::findMATLAB();

        if(MATLABsessions.empty()){
            apiMATLAB = matlab::engine::startMATLAB();
        }
        else{
            apiMATLAB = matlab::engine::connectMATLAB();
        }

        apiMATrix = std::make_unique<matlab::data::ArrayFactory>();
    }
    catch(matlab::engine::EngineException){
        std::cout<<"MATLAB initialization failed"<<std::endl;

        return false;
    }

    return true;
}

void threadFFT(const dpp::snowflake channelID, const dpp::voiceconn* voicePtr){
    static const size_t blankSize = 10000;
    static uint8_t blankData[blankSize];
    activeFFT = true;
    std::string buf = "-";
    dpp::message m(channelID, buf);
    m = apiDPP->message_create_sync(m);
    auto tmr = std::chrono::high_resolution_clock::now();

    while(activeFFT){
        if(!voicePtr||!voicePtr->voiceclient||!voicePtr->voiceclient->is_ready()){
            activeFFT = false;

            break;
        }

        //Send blank audio through the bot in order to receive audio from the call
        voicePtr->voiceclient->send_audio_raw((uint16_t*)blankData, blankSize);

        //Prevent rate-limiting on message edits
        if((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-tmr)).count()>1080){
            buf += '-';
            m.set_content(buf);
            apiDPP->message_edit(m);

            tmr = std::chrono::high_resolution_clock::now();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}