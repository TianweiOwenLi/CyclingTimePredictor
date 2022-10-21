#include <iostream>
#include <fstream>

#include <sstream>

#include <assert.h>
#include <list>
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>
#include <cmath>
#include <string>

#include <chrono>
#include <thread>

#include <getopt.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::function;
using std::min;
using std::max;
#define dbg(x) std::cout<<#x<<" = "<<x<<'\n'
#define mp(x,y) std::make_pair(x,y)

typedef struct {
    double x;
    double y;
} datapoint;

typedef vector<datapoint> path; // duplicate x not allowed. x must be sorted.

double watt_kmh(double kmh, double slope, double mass) {
    double resistance = 3.1 * (mass / 90.0) * kmh + 0.0065 * kmh * kmh * kmh;
    double climbpower = (kmh / 3.6) * slope * mass * 9.81;
    return resistance + climbpower;
}


double current_slope(path::iterator pathit) { // end() - 1 not allowed
    double y_diff = (pathit+1)->y - pathit->y;
    double x_diff = (pathit+1)->x - pathit->x;
    return y_diff / x_diff;
}


inline string numfmt(double x) {
    const int precision{6};
    string s = std::to_string(x);
    if (x >= 0.0) s = "+" + s;
    if (s.length() < precision) {
        s += string(8 - s.length(), '0');
    } else {
        s = s.substr(0, precision);
    }

    return s;
}


void pretty_print(double t, double x, double y, double v, double slope, \
        double d_watt) {

    cout << "time: " << numfmt(t) << "s   dist: " << numfmt(x) << \
            "m   altitude: " << numfmt(y) << "m   speed: " << numfmt(v * 3.6) \
            << "km/h   slope: " << numfmt(slope * 100)\
            << "%   redundant_pow: " << numfmt(d_watt) << "w" << endl;
}


double simulate(path p, double power, double mass, double dt, bool verbose, \
        bool realtime) {

    double x = 0, y = 0, v = 0, a = 0, t = 0;
    auto chkpoint = p.begin();
    double slope = current_slope(chkpoint);

    int last_printed_int_time = -1;

    for (;chkpoint + 1 != p.end(); t += dt) {

        if (realtime) {
            std::this_thread::sleep_for(\
                std::chrono::milliseconds((int)(dt / 0.001)));
        }

        x += v * dt;
        y = (slope * (x - chkpoint->x)) + chkpoint->y;
        v += a * dt;

        double watt_needed = watt_kmh(v * 3.6, slope, mass);
        a = (v >= 1.5)? \
            (power - watt_needed) / (v * mass) : 1; // m/s^2

        // note that dt / 100 is for eliminating rounding error.
        if (verbose && (int) (t + (dt / 100)) > last_printed_int_time) {
            pretty_print(t, x, y, v, slope, 150 - watt_needed);
            last_printed_int_time = (int) (t + (dt / 100));
        }


        // update checkpoint and slope if necessary.
        if (x > (chkpoint+1)->x) {
            chkpoint++;
            slope = current_slope(chkpoint);
        }

        // halts simulation if v becomes negative
        if (v < 0.0) {
            cout << "Your bicycle started going backwards! \n";
            break;
        }
    }

    int integral_time = (int) (t+0.0001);
    cout << "\n\nTime to finish: ";

    if (integral_time >= 3600) {
        cout << integral_time / 3600 << " hr ";
    } 

    if (integral_time >= 3600 || integral_time % 3600 > 60) {
        cout << (integral_time % 3600) / 60 << " min "; 
    }

    cout << integral_time % 60 << " sec \n\n" << endl;

    return t;
}


/**
 * Prints help message.
*/
void print_help_msg() {
    cout << "Usage: ./bikesim DATA_FILE POWER MASS PRECISION [OPTION]\n\n";
    cout << "Reads pairs of (location, height) pairs from the input file, as \n"
            "well as the specified average cycling wattage, and precision of \n"
            "simulation in terms of elapsed seconds per step, and returns \n"
            "the estimated time in seconds required to finish such a trip. \n";
    cout << "\nOptions:\n";
    cout << "  -v \t enable verbose mode, prints data during simulation.\n";
    cout << "  -r \t realtime mode, inserts a delay between each simulation \n"
            "     \t   steps, so that the simulation becomes real-time. This \n"
            "     \t   flag automatically enables verbose mode. Warning: c++ \n"
            "     \t   builtin function for timed thread sleeps have limited \n"
            "     \t   ccuracy, therefore the simulation may not be quite \n"
            "     \t   real-time when precision is high.\n";
    cout << "  -h \t prints this help message. " << endl;
}


int main(int argc, char *argv[]) {
    
    // parse required arguments
    if (argc < 5) {
        for (int i = 1; i <= argc; i++) {
            if (string(argv[i]) == "-h") {
                print_help_msg();
                return 0;
            }
        }
        cout << "Too few arguments! \n";
        return 1;
    }

    std::ifstream pathfile(argv[1]);
    vector<datapoint> p;
    if (pathfile.is_open()) {
        string line;
        while (getline(pathfile, line)) {
            int idx = line.find(',');
            
            double x = strtod(line.substr(0, idx).c_str(), NULL); 
            double y = strtod(line.substr(idx+1).c_str(), NULL);

            if (!p.empty() && x <= p.back().x) {
                cout << "Error: path cannot go backwards! \n";
                return 1;
            }

            p.push_back({x, y});
        }
    } else {
        cout << "Error: unable to open file \"" << argv[1] << "\"! \n";
        return 1; 
    }

    double power{strtod(argv[2], NULL)};
    if (power <= 25.0) {
        cout << "Error: average pedaling wattage too small! \n";
        return 1;
    }

    double mass{strtod(argv[3], NULL)};
    if (mass <= 30.0) {
        cout << "Error: mass too small! \n";
        return 1;
    }

    double precision{strtod(argv[4], NULL)};
    if (precision <= 1.0e-6) {
        cout << "Error: precision too small or negative! \n";
        return 1;
    }


    // parses options
    char opt;
    bool verbose = false;
    bool realtime = false;
    while ((opt = getopt(argc, argv, "vrh")) != -1) {
        switch (opt) {
            
            case 'v':
                verbose = true;
                break;

            case 'r':
                verbose = true;
                realtime = true;
                break;

            case 'h':
                print_help_msg();
                return 0;

            default: 
                printf("bikesim: invalid option -%c. \nTry "
                       "\'bikesim -h\' for usage. \n", opt);
                return 1;
        }
    }


    simulate(p, power, mass, precision, verbose, realtime);
}
