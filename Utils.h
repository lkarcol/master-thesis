#pragma once

#include "sdk/Orbitersdk.h";
#include <vector>;
#include <regex>;
#include <sstream>

using namespace::std;

// Vratenie velkosti vektora
double vector3size(const VECTOR3 &vec);

// Rozdelenie string na |
// TODO: parametrizovat delimiter
void stringSplit(const string str, string delimiter, vector<string> &vec);