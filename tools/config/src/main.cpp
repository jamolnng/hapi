#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>

namespace fs = std::experimental::filesystem;

inline bool has_sudo()
{
    return getuid() == 0 && geteuid() == 0;
}

std::string default_formatter(std::string line)
{
    return line;
}

std::string delay_formatter(std::string line)
{
    int t = std::stoi(line);
    if (t < 0)
        t = 0;
    if (t > 15)
        t = 15;
    return std::to_string(t);
}

std::string exp_formatter(std::string line)
{
    return delay_formatter(line);
}

std::string pulse_formatter(std::string line)
{
    int t = std::stoi(line);
    if (t < 0)
        t = 0;
    if (t > 310)
        t = 310;
    t /= 10;
    return std::to_string(t);
}

void prompt(std::string k, std::string s, std::ostringstream &sout,
            std::string d = "", std::function<std::string(std::string)> formatter = default_formatter)
{
    std::string line;
    std::cout << "Enter " << s << ": ";
    if (d.size() == 0)
    {
        while (line.size() == 0)
            std::getline(std::cin, line);
    }
    else
    {
        std::getline(std::cin, line);
        if (line.size() == 0)
            line = d;
    }
    try
    {
        line = formatter(line);
        sout << k << "=" << line << std::endl;
    }
    catch (...)
    {
        std::cout << "Invalid format" << std::endl;
        prompt(k, s, sout, d, formatter);
    }
}

int main(int argc, char *argv[])
{
    if (!has_sudo())
    {
        std::cout << "root permissions required to run!" << std::endl;
        return -1;
    }
    bool ask_all = argc > 1 && std::string(argv[1]) == "all";
    if (ask_all)
        std::cout << "WARNING: this mode should only be configured to match the HAPI-E board!" << std::endl
                  << std::endl;

    std::ostringstream sout;
    prompt("output", "output directory (default: /home/pi/hapi)", sout, "/home/pi/hapi");
    prompt("delay", "delay in microseconds: 0-15 (default 8)", sout, "8", delay_formatter);
    prompt("exp", "exposure in microseconds: 0-15 (default 2)", sout, "2", exp_formatter);
    prompt("pulse", "pulse width in nanoseconds: 0-310 (default 310)", sout, "310", pulse_formatter);
    prompt("image_type", "Image type: png, ppm, pgm, tiff, jpeg, jpg, bmp (default png)", sout, "png");

    if (ask_all)
    {
        prompt("trigger_type", "trigger type: 0 = Software, 1 = Hardware (default 1)", sout, "1");
        prompt("arm_pin", "arm pin (default: 26)", sout, "26");
        prompt("delay_pin0", "delay pin 0 (default: 7)", sout, "7");
        prompt("delay_pin1", "delay pin 1 (default: 0)", sout, "0");
        prompt("delay_pin2", "delay pin 2 (default: 1)", sout, "1");
        prompt("delay_pin3", "delay pin 3 (default: 2)", sout, "2");
        prompt("exp_pin0", "exposure pin 0 (default: 13)", sout, "13");
        prompt("exp_pin1", "exposure pin 1 (default: 6)", sout, "6");
        prompt("exp_pin2", "exposure pin 2 (default: 14)", sout, "14");
        prompt("exp_pin3", "exposure pin 3 (default: 10)", sout, "10");
        prompt("pulse_pin0", "pulse pin 0 (default: 24)", sout, "24");
        prompt("pulse_pin1", "pulse pin 1 (default: 27)", sout, "27");
        prompt("pulse_pin2", "pulse pin 2 (default: 25)", sout, "25");
        prompt("pulse_pin3", "pulse pin 3 (default: 28)", sout, "28");
        prompt("pulse_pin4", "pulse pin 4 (default: 29)", sout, "29");
    }

    std::string ostr = sout.str();

    if (!fs::exists("/opt/hapi"))
        fs::create_directory("/opt/hapi");

    std::ofstream out("/opt/hapi/hapi.conf", std::ios::binary);

    if (out)
    {
        out << ostr;
        out.close();
    }
    else
    {
        std::cout << "Unable to write config file to: /opt/hapi/hapi.conf" << std::endl;
    }

    return 0;
}