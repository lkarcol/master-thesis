#include "Utils.h";

double vector3size(const VECTOR3 &vec) {
	return sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}


void stringSplit(const string str, string delimiter, vector<string> &vec) {


	regex reg("\\" + delimiter);

	sregex_token_iterator iter(str.begin(), str.end(), reg, -1);
	sregex_token_iterator end;

	vector<string> _vec(iter, end);

	std::copy(_vec.begin(), _vec.end(), std::back_inserter(vec));
}
