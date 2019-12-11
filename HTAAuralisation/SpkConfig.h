#pragma once
#include <armadillo>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <string>
#include "DSPtools.h"
#include <future>
#include <fstream>
#include <math.h>

class SpkConfig {
public:
	struct Config {
	public:
		int dummyProperty = 0;
	};
	Config config;
	SpkConfig();
	vector<vector<double>>	GetPolarPositions();
	vector<vector<double>> GetCartesianPositions();
	vector<string> GetLabels();
	string GetLayoutName();
private:
	vector<string> labels;
	vector<vector<double>> polarPositions;
	vector<vector<double>> cartesianPositions;
	string layoutName;
	bool HeaderCheck(string l);
	string LineParse(string line);
	void Polar2Cart(vector<vector<double>>& x, vector<vector<double>>& z);
};
