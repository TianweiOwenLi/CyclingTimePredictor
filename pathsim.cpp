#include <iostream>
#include <fstream>
#include <assert.h>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
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

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::stack;
using std::queue;
using std::pair;
using std::unordered_set;
using std::unordered_map;
using std::multimap;
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


int simulate(path p, double power, double mass, double dt) { // dt > 0
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

            if ((print_counter++) % 10 == 0)
                cout << "x: " << x << "  v: " << v << "  a: " << a << \
                "  slope: " << slope/100.0 << "%  t: " \
                << t << "  watt needed: " << watt_needed << endl;


            // update checkpoint and slope if necessary.
            if (x > (chkpoint+1)->x) {
                chkpoint++;
                slope = current_slope(chkpoint);
            }
        }

        return t;
}


int main() {
    vector<datapoint> p;
    p.push_back({0, 240});
    p.push_back({200, 243});
    p.push_back({500, 235});
    p.push_back({600, 240});

    simulate(p, 150, 90, 0.1);
}