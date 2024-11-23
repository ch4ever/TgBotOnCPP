#include <stdio.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <tgbot/tgbot.h>
#include <string>
#include <sstream>
#include <random>
#include <map>

#include "TgBot.h"


using std::cin;
using namespace TgBot;


int main() {
    TgBot::Bot bot("TOKEN");
    int upperborder = 100;
    int lowerborder = 1;
    std::map<std::string, bool> status = {
        {"set_upperborder" ,false},
        {"ready_for_game", false},
        { "ready_for_gpt" ,false},
        {"set_lowerborder", false}
    };
    std::random_device rd; 
    std::mt19937 gen(rd()); 
    int random_number{};

    vector<BotCommand::Ptr> commands;
    auto cmdStart = std::make_shared<TgBot::BotCommand>();
    cmdStart->command = "start";
    cmdStart->description = "Restart/start bot";
    commands.push_back(cmdStart);

    auto cmdLower = std::make_shared<TgBot::BotCommand>();
    cmdLower->command = "lower";
    cmdLower->description = "Set lowerborder value";
    commands.push_back(cmdLower);

    auto cmdUpper = std::make_shared<TgBot::BotCommand>();
    cmdUpper->command = "upper";
    cmdUpper->description = "Set upperborder value";
    commands.push_back(cmdUpper);

    auto cmdGpt = std::make_shared<TgBot::BotCommand>();
    cmdGpt->command = "gpt";
    cmdGpt->description = "Send GPTAPI request and get a response";
    commands.push_back(cmdGpt);

    auto cmdStartGame = std::make_shared<TgBot::BotCommand>();
    cmdStartGame->command = "startgame";
    cmdStartGame->description = "Generate random value between 2 borders and start game";
    commands.push_back(cmdStartGame);

    bot.getApi().setMyCommands(commands);



    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi, this is a bot for guess number game! \tEnter lower and upper borders by commands and start game!");
        });

    bot.getEvents().onCommand("upper", [&bot, &upperborder, &status](TgBot::Message::Ptr message) {
        if (message->text.length() > 7) {
            string arg = message->text.substr(7);
            try {
                upperborder = stoi(arg);
                bot.getApi().sendMessage(message->chat->id, "You set upperborder to " + to_string(upperborder));

            }
            catch (const std::invalid_argument&) {
                bot.getApi().sendMessage(message->chat->id, "Invalid arg of border");
            }
            catch (const out_of_range&) {
                bot.getApi().sendMessage(message->chat->id, "Out of range");
            }
        }
        else {
            bot.getApi().sendMessage(message->chat->id, "Enter the upperborder number: ");
            status["set_upperborder"] = true;
        }
    });

    bot.getEvents().onCommand("lower", [&bot, &lowerborder,&status](TgBot::Message::Ptr message) {
        if (message->text.length() > 7) {
            string arg2 = message->text.substr(7);
            try {
                lowerborder = stoi(arg2);
                bot.getApi().sendMessage(message->chat->id, "You set lowerborder to " + to_string(lowerborder));

            }
            catch (const std::invalid_argument&) {
                bot.getApi().sendMessage(message->chat->id, "Invalid arg of border");
            }
            catch (const out_of_range&) {
                bot.getApi().sendMessage(message->chat->id, "Out of range");
            }
        }
        else {
            bot.getApi().sendMessage(message->chat->id, "Enter the upperborder number: ");
            status["set_lowerborder"] = true;
        
        }
        });


    bot.getEvents().onCommand("gpt", [&bot, &status](TgBot::Message::Ptr message) {
        SwitchMode(status);
        status["ready_for_gpt"] = true;
        bot.getApi().sendMessage(message->chat->id, "Enter your GPT prompt");
       
    });


    bot.getEvents().onCommand("startgame", [&bot, &upperborder, &lowerborder, &gen, &random_number, &status](TgBot::Message::Ptr message) {
        SwitchMode(status);
        uniform_int_distribution<> distrib(lowerborder, upperborder);
        status["ready_for_game"] = true;
        random_number = distrib(gen);
        bot.getApi().sendMessage(message->chat->id, "Game has began, \tStart guessing the num with bots hints");


        TgBot::ReplyKeyboardMarkup::Ptr keyboard = std::make_shared<TgBot::ReplyKeyboardMarkup>();
        keyboard->resizeKeyboard = true;

        TgBot::KeyboardButton::Ptr button1 = std::make_shared<TgBot::KeyboardButton>();
        button1->text = "Restart Game";

        std::vector<KeyboardButton::Ptr> row1 = { button1 };
        keyboard->keyboard.push_back(row1);

        bot.getApi().sendMessage(message->chat->id, "Added actionbutton to Restart game", false, 0, keyboard);
     });

    bot.getEvents().onAnyMessage([&bot, &random_number, &status, &gen, &lowerborder, &upperborder](TgBot::Message::Ptr message) {

        if (message->text == "Restart Game") {
            uniform_int_distribution<> distrib(lowerborder, upperborder);
            random_number = distrib(gen);
            bot.getApi().sendMessage(message->chat->id, "Number has been regenerated");
            status["ready_for_game"] = true;
            return;
        }

        if (!message->text.empty() && message->text[0] == '/') {
            return;
        }
       if (IsModeInactive(status)) {
         bot.getApi().sendMessage(message->chat->id, "Choose mode by command");
            return;
       }

        if (status["set_upperborder"]) {
            string arg3 = message->text.substr();
            try {
                upperborder = stoi(message->text);
                bot.getApi().sendMessage(message->chat->id, "You set upperborder to " + to_string(upperborder));
                status["set_upperborder"] = false;
            }
            catch (const std::invalid_argument&) {
                bot.getApi().sendMessage(message->chat->id, "Invalid number");
            }
            catch (const out_of_range&) {
                bot.getApi().sendMessage(message->chat->id, "Out of range");
            }
            
        }

        if (status["set_lowerborder"]) {
            string arg4 = message->text.substr();
            try {
                lowerborder = stoi(message->text);
                bot.getApi().sendMessage(message->chat->id, "You set lowerborder to " + to_string(lowerborder));
                status["set_lowerborder"] = false;
            }
            catch (const std::invalid_argument&) {
                bot.getApi().sendMessage(message->chat->id, "Invalid number");
            }
            catch (const out_of_range&) {
                bot.getApi().sendMessage(message->chat->id, "Out of range");
            }

        }

        if (status["ready_for_game"]) {
            try {
                int guess = stoi(message->text);


                if (guess > random_number) {
                    bot.getApi().sendMessage(message->chat->id, "Too high!");
                }
                if (guess < random_number) {
                    bot.getApi().sendMessage(message->chat->id, "Too low!");
                }
                if (guess == random_number) {
                    bot.getApi().sendMessage(message->chat->id, "Thats it! \tRestart game for other number.");
                    status["ready_for_game"] = false;
                }
            }
            catch (invalid_argument&) {
                bot.getApi().sendMessage(message->chat->id, "Please Enter a valid number");
            }

        }
        
        if (status["ready_for_gpt"]) {
            string prompt = message->text;
            cout << "Received prompt: " << prompt << endl; 

            string response = getCompletion(prompt, "gpt-3.5-turbo");  
            cout << "GPT response: " << response << endl; 

            bot.getApi().sendMessage(message->chat->id, "Your response is: " + response);

            status["ready_for_gpt"] = false;  
        }
        });
        

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
        return 0;

}


