#include "Utils.h";

double vector3size(const VECTOR3 &vec) {
	return sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

vector<string> stringSplit(const string str) {

	regex reg("\\|");
	sregex_token_iterator iter(str.begin(), str.end(), reg, -1);
	sregex_token_iterator end;

	vector<string> vec(iter, end);

	return vec;
}
