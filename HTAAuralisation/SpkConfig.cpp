#include "SpkConfig.h"
using namespace std;
namespace fs = std::filesystem;

SpkConfig::SpkConfig() {
	
	vector<fs::path> sc_path;
	fs::path a;
	sc_path.push_back(a);
	int x = 9999;
	while (x > (sc_path.size()-1)) {
		int  _j = 0;
		sc_path.clear();
		std::cout << "Please select a speaker layout" << std::endl;
		for (const auto& entry : fs::directory_iterator("../Config/")) {
			sc_path.push_back(entry.path());

			std::cout << _j << ": " << entry.path().string() << std::endl;
			_j++;
		}
		cin >> x;
	}
	
	ifstream speakerLayout(sc_path[x].string());
	string line;

	if (speakerLayout.is_open()) {
		while (getline(speakerLayout, line))
		{
			if (HeaderCheck(line)) {
				LineParse(line);
			}
		}
		speakerLayout.close();
	}
	else cout << "Unable to open file";
	if (polarPositions.size() > 0)
		Polar2Cart(this->polarPositions, cartesianPositions);
}

vector<vector<double>> SpkConfig::GetPolarPositions() {
	return this->polarPositions;
}

vector<vector<double>> SpkConfig::GetCartesianPositions() {
	return this->cartesianPositions;
}

vector<string> SpkConfig::GetLabels() {
	return this->labels;
}

string SpkConfig::GetLayoutName() {
	return this->layoutName;
}

bool SpkConfig::HeaderCheck(string l) {
	bool op = true;
	char h = '*';
	if (l[0] == h)
		op = false;
	return op;
}

string SpkConfig::LineParse(string line) {

	if (line.substr(0, 12) == "Config Name:") {
		this->layoutName = line.substr(13);
	}
	else if (line.substr(0, 2) == "//") {
		this->labels.push_back(line.substr(2));
	}
	else {
		//todo: parse speaker positions - use a loop, checking for comma then subdivide at comma.
		int split = 0;
		for (int i = 0; i < line.size(); i++) {
			split++;
			if (line[i] == ',')
				break;
		}
		double az = stod(line.substr(0, split - 1));
		double el = stod(line.substr(split));
		vector<double> p;
		p.push_back(az);
		p.push_back(el);
		this->polarPositions.push_back(p);
	}


	return "done parsing\n";
}

void SpkConfig::Polar2Cart(vector<vector<double>>& x, vector<vector<double>>& z) {
	z.clear();
	for (int i = 0; i < x.size(); i++) {
		vector<double> y;
		y.push_back(sin(x[i][0]) * cos(x[i][1]));
		y.push_back(cos(x[i][0]) * cos(x[i][1]));
		y.push_back(sin(x[i][1]));
		z.push_back(y);
	}
}