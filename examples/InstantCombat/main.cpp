#include <iostream>
#include <vector>
#include <ctime>
#include "PhoenixApi/Api.h"
#include "Bot.h"
#include "Scene.h"
#include "Split_string.h"

int select_port();

int main()
{
    int port = select_port();

    if (port == -1)
        return 0;

    Phoenix::Api api(port);
    Scene scene;
    Bot bot(&api, &scene);
    std::vector<Module*> modules = { &scene, &bot };
    clock_t time_begin = clock();

    std::cout << "Bot is running..." << std::endl;

    while (true)
    {
        if (!api.empty())
        {
            std::string message = api.get_message();

            try
            {
                nlohmann::json json_msg = nlohmann::json::parse(message);

                if (json_msg["type"] == Phoenix::Type::packet_send)
                {
                    std::string packet = json_msg["packet"];
                    std::vector<std::string> packet_splitted = split_string(packet);

                    if (packet_splitted.size() > 0)
                    {
                        for (auto mod : modules)
                            mod->on_send(packet_splitted, packet);
                    }
                }

                if (json_msg["type"] == Phoenix::Type::packet_recv)
                {
                    std::string packet = json_msg["packet"];
                    std::vector<std::string> packet_splitted = split_string(packet);

                    if (packet_splitted.size() > 0)
                    {
                        for (auto mod : modules)
                            mod->on_recv(packet_splitted, packet);
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                return 1;
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        float diff_time = float(clock() - time_begin) / CLOCKS_PER_SEC;

        if (diff_time >= 1.0f)
        {
            bot.move();
            time_begin = clock();
        }
    }

}

int select_port()
{
    int option = -1;
    int port = -1;

    std::vector<int> ports = Phoenix::find_ports();

    std::cout << "Select the port to connect (-1 to exit):" << std::endl;

    for (size_t i = 0; i < ports.size(); ++i)
    {
        std::cout << i << ") " << ports[i] << std::endl;
    }

    while (true)
    {
        std::cin >> option;

        if (option == -1)
        {
            std::cout << "Exiting..." << std::endl;
            break;
        }

        else if (option < 0 || option >= (int)ports.size())
        {
            std::cout << "Selected option is not valid, try again." << std::endl;
            std::cout << "Options are: ";

            for (size_t i = 0; i < ports.size(); ++i)
            {
                std::cout << i << " ";
            }

            std::cout << std::endl;
        }

        else
        {
            port = ports[option];
            break;
        }
    }

    return port;
}
