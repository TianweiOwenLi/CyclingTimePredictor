#include <iostream>
#include <fstream>

#include <assert.h>
#include <list>
#include <vector>
#include <bitset>
#include <random>
#include <complex>
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
    double resistance = 3.1 * kmh + 0.0065 * kmh * kmh * kmh;
    double climbpower = (kmh / 3.6) * slope * mass * 9.81;
    return resistance + climbpower;
}


double current_slope(path::iterator pathit) { // end() - 1 not allowed
    double y_diff = (pathit+1)->y - pathit->y;
    double x_diff = (pathit+1)->x - pathit->x;
    return y_diff / x_diff;
}


int simulate(path p, double power, double mass, double dt, bool verbose) {
        double x = 0, y = 0, v = 0, a = 0, t = 0;

        auto chkpoint = p.begin();
        double slope = current_slope(chkpoint);

        int print_counter = 0;

        for (;chkpoint + 1 != p.end(); t += dt) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            x += v * dt;
            y = (slope * (x - chkpoint->x)) + chkpoint->y;
            v += a * dt;

            double watt_needed = watt_kmh(v * 3.6, slope, mass);
            a = (v >= 1.5)? \
                (power - watt_needed) / (v * mass) : 1; // m/s^2

            if (verbose && (print_counter++) % 10 == 0) {
                cout << "x: " << x << "  v: " << v << "  a: " << a << \
                "  slope: " << slope/100.0 << "%  t: " \
                << t << "  watt needed: " << watt_needed << endl;
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

        return t;
}


/**
 * Prints help message.
*/
void print_help_msg() {
    cout << "Usage: ./bikesim DATA_FILE WATT TIME_PRECISION [OPTION]" << endl;
    cout << "Reads pairs of (location, height) pairs from the input file, as \n"
            "well as the specified average cycling wattage, and precision of \n"
            "simulation in terms of elapsed seconds per step, and returns \n"
            "the estimated time in seconds required to finish such a trip. \n";
    cout << "\nOptions:\n";
    cout << "  -v \t enable verbose mode, prints data during simulation.\n";
    cout << "  -r \t realtime mode, inserts a delay between each simulation "
            "steps, so that the simulation becomes real-time. This flag will "
            "automatically enable verbose mode. Warning: c++ built-in function "
            "for timed thread sleeps have limited accuracy, therefore the "
            "simulation may not be quite real-time when precision is high.\n";
    cout << "  -h \t prints this help message. ";
}


int main(int argc, char *argv[]) {
    
    
    // parse required arguments
    if (argc < 4) {
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
    if (power <= 0.0) {
        cout << "Error: average pedaling wattage must be positive! \n";
        return 1;
    }

    double precision{strtod(argv[3], NULL)};
    if (precision <= 0.0) {
        cout << "Error: time precision has to be positive! \n";
        return 1;
    }


    // parses options
    char opt;
    bool verbose = false;
    bool realtime = false;
    while ((opt = getopt(argc, argv, "v:r:h")) != -1) {
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

    simulate(p, power, 90, precision, verbose);
}
