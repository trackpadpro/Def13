#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>

#include <curl/curl.h>
#include <dpp/dpp.h>

#ifdef MATLAB
    #include <complex>
    #include <atomic>
    #include <thread>
    #include <mutex>    

    #include <MatlabEngine.hpp>
    #include <MatlabDataArray.hpp>

    void threadFFT(const dpp::snowflake channelID, const dpp::voiceconn* voicePtr);
    matlab::engine::FutureResult<std::unique_ptr<matlab::engine::MATLABEngine>> initializeMATLAB();
    std::unique_ptr<matlab::engine::MATLABEngine> apiMATLAB;
    matlab::engine::FutureResult<std::unique_ptr<matlab::engine::MATLABEngine>> apiMATfut;
    std::unique_ptr<matlab::data::ArrayFactory> apiMATrix;
    std::atomic<bool> activeMATLAB = false, activeFFT = false;
    std::mutex audioMut;
    std::vector<double> audioData;
#endif

bool authenticate();
std::unique_ptr<dpp::cluster> apiDPP;
std::vector<dpp::snowflake> serverList;
dpp::snowflake userID;

int main(){
    if(!authenticate()){
        return 1;
    }

#ifdef MATLAB
    apiMATfut = initializeMATLAB();
#endif

    apiDPP->on_log(dpp::utility::cout_logger());

    apiDPP->on_slashcommand([](const dpp::slashcommand_t& event){
        if(event.command.get_command_name()=="ping"){
            if(event.command.get_issuing_user().id==116429793532182537){
                dpp::snowflake serverCurrent = event.command.guild_id;

                std::ofstream serverFile("./data/auth/servers.txt", std::ios::app);
                if(!serverFile.good()){
                    std::cout<<"Server file failed to open"<<std::endl;
                }
                else if(std::find(serverList.begin(), serverList.end(), serverCurrent)==serverList.end()){
                    serverList.push_back(serverCurrent);
                    serverFile<<serverCurrent<<'\n';
                }

                serverFile.close();
            }

            event.reply("pong");
        }
        else if(event.command.get_command_name()=="repo"){
            event.reply("https://github.com/trackpadpro/Def13");
        }
#ifdef MATLAB
        else if(event.command.get_command_name()=="eig"){
            if(!activeMATLAB){
                try{
                    apiMATLAB = apiMATfut.get();
                }
                catch(matlab::engine::EngineException){
                    event.reply("MATLAB initialization failed");

                    return;
                }

                apiMATrix = std::make_unique<matlab::data::ArrayFactory>();
                activeMATLAB = true;

                apiMATLAB->evalAsync(u"addpath('scripts/')");
            }

            srand(static_cast<u_int>(std::chrono::system_clock::now().time_since_epoch().count()));

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
            if(!activeMATLAB){
                try{
                    apiMATLAB = apiMATfut.get();
                }
                catch(matlab::engine::EngineException){
                    event.reply("MATLAB initialization failed");

                    return;
                }

                apiMATrix = std::make_unique<matlab::data::ArrayFactory>();
                activeMATLAB = true;

                apiMATLAB->eval(u"addpath('scripts/')");
            }

            if(std::find(serverList.begin(), serverList.end(), event.command.guild_id)!=serverList.end()){
                dpp::guild* g = dpp::find_guild(event.command.guild_id);
                userID = event.command.get_issuing_user().id;

                if(!g->connect_member_voice(userID)){
                    event.reply("unable to connect to user's voice channel");

                    return;
                }

                dpp::voiceconn* v = event.from->get_voice(event.command.guild_id);

                event.reply("beginning fft");

                std::thread t(threadFFT, event.command.channel_id, v);
                t.detach();
            }
            else{
                event.reply("server not authorized");
            }
        }
        else if(event.command.get_command_name()=="end"){
            activeFFT = false;

            std::this_thread::sleep_for(std::chrono::milliseconds(777)); //allow thread processes to end

            event.from->disconnect_voice(event.command.guild_id);

            event.reply(":saluting_face:");
        }
#endif
    });

#ifdef MATLAB
    apiDPP->on_voice_receive([](const dpp::voice_receive_t& event){
        if(activeFFT&&event.user_id==userID){
            static std::basic_string<uint8_t> packet;
            static size_t audioSize;
            static int8_t clip;
            
            packet = event.audio_data;

            audioMut.lock();
            audioSize = audioData.size();
            audioData.resize(audioSize+packet.size());
            for(size_t i = 0; i<packet.size(); i++){
                clip = *(int8_t*)&packet[i]; //clip = packet[i] works too
                audioData[audioSize+i] = clip;
            }
            audioMut.unlock();
        }
    });
#endif

    apiDPP->on_message_create([](const dpp::message_create_t& event){
        if(std::find(serverList.begin(), serverList.end(), event.msg.guild_id)!=serverList.end()){
            std::string msgTxt = event.msg.content;
            dpp::message msgOrigin = event.msg;

            size_t strPos = msgTxt.find("https://www.instagram.");
            if(strPos!=std::string::npos){
                bool send = true;

                size_t strPos2 = msgTxt.find("/share/");
                if(strPos2!=std::string::npos){
                    send = false;

                    strPos2 = msgTxt.find(' ', strPos2);
                    if(strPos2!=std::string::npos){
                        strPos2 = strPos2-strPos; //set strPos2 to length of URL
                    }
                    
                    CURL* curl = curl_easy_init();
                    if(curl){
                        CURLcode res;
                        curl_easy_setopt(curl, CURLOPT_URL, (msgTxt.substr(strPos, strPos2)+'/').data());
                        //mimic browser to avoid https://www.facebook.com/unsupportedbrowser redirect
                        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Chrome/51.0.2704.64 Safari/537.36");
                        res = curl_easy_perform(curl);
                        if(res==CURLE_OK){
                            char* url = NULL;
                            curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &url);
                            if(url){
                                msgTxt.replace(strPos, strPos2, url);

                                strPos2 = msgTxt.find("/p/");
                                if(strPos2!=std::string::npos){
                                    send = true;
                                }
                                else{
                                    strPos2 = msgTxt.find("/reel");
                                    if(strPos2!=std::string::npos){
                                        send = true;
                                    }
                                }
                            }
                        }
                        else{
                            std::cout<<msgTxt.substr(strPos, strPos2)+"/ "<<res<<std::endl;
                        }
                        curl_easy_cleanup(curl);
                    }
                }

                msgTxt.insert(strPos+12, "kk");
                //msgTxt.insert(strPos+12, "dd");
                //msgTxt.insert(strPos+21, "ez");

                strPos = msgTxt.find("/?");
                if(strPos!=std::string::npos){
                    strPos2 = msgTxt.find(' ', strPos);
                    if(strPos2!=std::string::npos){
                        strPos2 = strPos2-strPos; //set strPos2 to length of igsh code
                    }

                    msgTxt.erase(strPos+1, strPos2);
                }

                strPos = msgTxt.find("/reel/");
                if(strPos!=std::string::npos){
                    msgTxt.insert(strPos+5, "s");
                }

                strPos = msgTxt.find("/p/");
                if(strPos!=std::string::npos){
                    //check if post is public
                    strPos2 = msgTxt.find(' ', strPos);
                    if(strPos2!=std::string::npos){
                        strPos2 = strPos2-strPos; //set strPos2 to length of post_id
                    }
                    else{
                        strPos2 = msgTxt.length(); //set strPos2 to length of string
                    }

                    if(strPos2>=strPos+20){
                        send = false;
                    }
                }

                if(send){
                    event.reply(msgTxt, true);

                    apiDPP->message_edit_flags(msgOrigin.suppress_embeds(true));
                }
            }
            else{
                strPos = msgTxt.find("https://www.reddit");

                if(strPos!=std::string::npos){
                    msgTxt.replace(strPos+13, 1, "x");

                    event.reply(msgTxt, true);

                    apiDPP->message_edit_flags(msgOrigin.suppress_embeds(true));
                }
                else{
                    strPos = msgTxt.find("https://www.tiktok");

                    if(strPos!=std::string::npos){
                        msgTxt.replace(strPos+13, 1, "fx");

                        event.reply(msgTxt, true);

                        apiDPP->message_edit_flags(msgOrigin.suppress_embeds(true));
                    }
                }
            }
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
#ifdef MATLAB
                dpp::slashcommand("eig", "eigenvalue of matrix", apiDPP->me.id),
                dpp::slashcommand("fft", "conducts a fast Fourier transform on your voice", apiDPP->me.id),
                dpp::slashcommand("end", "leave voice channel", apiDPP->me.id)
#endif
            }); 
        }
    });

    //await events
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

    apiDPP = std::make_unique<dpp::cluster>(tokenDiscord, dpp::i_default_intents|dpp::i_message_content);
    dpp::snowflake server;

    auth.open("./data/auth/servers.txt", std::ios::in);
    while(auth.good()){
        auth>>server;
        serverList.push_back(server);
    }
    auth.close();

    return true;
}

#ifdef MATLAB
    matlab::engine::FutureResult<std::unique_ptr<matlab::engine::MATLABEngine>> initializeMATLAB(){
        std::vector<matlab::engine::String> MATLABsessions = matlab::engine::findMATLAB();
        matlab::engine::FutureResult<std::unique_ptr<matlab::engine::MATLABEngine>> result;

        if(MATLABsessions.empty()){
            result = matlab::engine::startMATLABAsync();
        }
        else{
            result = matlab::engine::connectMATLABAsync();
        }

        return result;
    }

    void threadFFT(const dpp::snowflake channelID, const dpp::voiceconn* voicePtr){
        static const size_t blankSize = 10000;
        static uint8_t blankData[blankSize];
        activeFFT = true;
        std::string buf = "-";
        dpp::message m(channelID, buf);
        m = apiDPP->message_create_sync(m);

        auto tmr = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        while(activeFFT){
            std::this_thread::sleep_for(std::chrono::milliseconds(250));

            if(!voicePtr||!voicePtr->voiceclient||!voicePtr->voiceclient->is_ready()){
                activeFFT = false;

                break;
            }

            //send blank audio through the bot in order to receive audio from the call
            voicePtr->voiceclient->send_audio_raw((uint16_t*)blankData, blankSize);

            //prevent rate-limiting on message edits
            if((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-tmr)).count()>4080){
                buf += '-';
                m.set_content(buf);
                apiDPP->message_edit(m);

                /*audioMut.lock();
                matlab::data::TypedArray<double> audio = apiMATrix->createArray({1, audioData.size()}, audioData.begin(), audioData.end());
                audioData.clear();
                audioMut.unlock();

                apiMATLAB->setVariable(u"audio", audio);

                apiMATLAB->evalAsync(u"audioFFT(audio);");*/

                apiMATLAB->evalAsync(u"audioFFT();");

                tmr = std::chrono::high_resolution_clock::now();
            }
        }
    }
#endif
