#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>

namespace fs = std::experimental::filesystem;

bool has_sudo()
{
    return getuid() == 0 && geteuid() == 0;
}

void prompt(std::string k, std::string s, std::ostringstream &sout)
{
    std::string line;
    std::cout << "Enter " << s << ": ";
    std::getline(std::cin, line);
    sout << k << "=" << line << std::endl;
}

int main(int argc, char *argv[])
{
    if(!has_sudo())
    {
        std::cout << "root permissions required to run!" << std::endl;
        return -1;
    }
    bool ask_all = argc > 1 && std::string(argv[1]) == "all";
    std::ostringstream sout;
    prompt("output", "output directory (ex: /home/pi/hapi)", sout);
    prompt("delay", "delay in microseconds", sout);
    prompt("exp", "exposure in microseconds", sout);
    prompt("pulse", "pulse width in nanoseconds", sout);
    prompt("image_type", "Image type (png, ppm, pgm, tiff, jpeg, jpg, bmp)", sout);

    if (ask_all)
    {
        prompt("trigger_type", "trigger type (0 = Software, 1 = Hardware)", sout);
        prompt("arm_pin", "arm pin (ex: 26)", sout);
        prompt("delay_pin0", "delay pin 0 (ex: 7)", sout);
        prompt("delay_pin1", "delay pin 1 (ex: 0)", sout);
        prompt("delay_pin2", "delay pin 2 (ex: 1)", sout);
        prompt("delay_pin3", "delay pin 3 (ex: 2)", sout);
        prompt("exp_pin0", "exposure pin 0 (ex: 13)", sout);
        prompt("exp_pin1", "exposure pin 1 (ex: 6)", sout);
        prompt("exp_pin2", "exposure pin 2 (ex: 14)", sout);
        prompt("exp_pin3", "exposure pin 3 (ex: 10)", sout);
        prompt("pulse_pin0", "pulse pin 0 (ex: 24)", sout);
        prompt("pulse_pin1", "pulse pin 1 (ex: 27)", sout);
        prompt("pulse_pin2", "pulse pin 2 (ex: 25)", sout);
        prompt("pulse_pin3", "pulse pin 3 (ex: 28)", sout);
        prompt("pulse_pin4", "pulse pin 4 (ex: 29)", sout);
    }

    std::string ostr = sout.str();

    if(!fs::exists("/opt/hapi"))
        fs::create_directory("/opt/hapi");

    std::ofstream out("/opt/hapi/hapi.conf", std::ios::binary);

    if (out)
    {
        out << ostr;
        out.close();
    }

    return 0;
}