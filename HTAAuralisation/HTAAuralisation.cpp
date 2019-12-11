// HTAAuralisation.cpp : Defines the entry point for the console application.
//

#include "../../AudioFile/AudioFile.h"
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

//#include "../../vst-intelligibility-meter/s3a_SiMeter_Plugin/Source/S3A_SiMeter/DSPtools.h"
using namespace std;
namespace fs = std::filesystem;

class SpkConf {
public:

	SpkConf();
	vector<vector<double>>	GetPolarPositions();
	vector<vector<double>> GetCartesianPositions();
	vector<string> GetLabels();
	//string GetLayoutName();
private:
	vector<string> labels;
	vector<vector<double>> polarPositions;
	vector<vector<double>> cartesianPositions;
	string layoutName;
	bool HeaderCheck(string l);
	string LineParse(string line);
};

SpkConf::SpkConf() {
	ifstream speakerLayout("../Config/renderpos.txt");
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
}

vector<vector<double>> SpkConf::GetPolarPositions() {
	return this->polarPositions;
}

vector<vector<double>> SpkConf::GetCartesianPositions() {
	return this->cartesianPositions;
}

vector<string> SpkConf::GetLabels() {
	return this->labels;
}

bool SpkConf::HeaderCheck(string l) {
	bool op = true;
	char h = '*';
	if (l[0] == h)
		op = false;
	return op;
}

string SpkConf::LineParse(string line) {
	
	if (line.substr(0, 12) == "Config Name:") {
		this->layoutName = line.substr(13);
	}
	if (line.substr(0, 2) == "//") {
		this->labels.push_back(line.substr(2));
	}
	if (line.substr(0, 1) == "{") {
		//todo: parse speaker positions - use a loop, checking for comma then subdivide at comma.
	}
	return "done parsing\n";
}

arma::fvec proc(arma::fvec a, arma::fvec b, double c) {
	arma::fvec y = arma::conv(a, b);
	y = y / c;
	return y;
}

int main()
{

	cout << "Welcome to the auralisation app.\n";
	SpkConf spk;
	
	cout << "Loading B-format IR from ./Impulse Response\n";
	// Load in 4 channel .WAV Impulse response
	int _j = 0;
	cout << "IR Loading:\n";
	vector<fs::path> ir_path;
	vector <AudioFile<double>> ir;
	for (const auto& entry : fs::directory_iterator("../IR/")) {
		ir_path.push_back(entry.path());
		AudioFile<double> a;
		a.load(entry.path().string());
		ir.push_back(a);
		std::cout << _j << ": " << entry.path().string() << std::endl;
		_j++;
	}
	cout << "Loading audio file from ./Audio \n";
	cout << "Available audio files:\n";
	vector<fs::path> af_path;
	_j = 0;
	for (const auto& entry : fs::directory_iterator("../Audio/")) {
		af_path.push_back(entry.path());
		std::cout << _j <<": "<< entry.path().string() << std::endl;
		_j++;
	}
	cout << "\nWhich audio file?\n";

	// Load in mono audio in .Wav format
	int af_n;
	cin >> af_n;
	AudioFile<double> af;
	af.load(af_path[af_n].string());
	
	
	//cout << "\nLoading HRTFs from ./HRTFs\n";
	//make a vector of HRTFs and vector of rendering points

	cout << "IR info: Fs=" << ir[0].getSampleRate() << " 2^n=" << ir[0].getBitDepth() << " bits"; // output Fs and 2^n of IR
	cout << "\n Audio info: Fs=" << af.getSampleRate() << " 2^n=" << af.getBitDepth() << " bits";
		; //output Fs and 2^n of audio
	cout << "\nConforming audio "; //get the highest quality file and updoot the inferior to parity
	DSPtools::InterpFilt interp;
	
	if (ir[0].getSampleRate() != af.getSampleRate()) {
		if (ir[0].getSampleRate() < af.getSampleRate()) {
			vector<double> n_a;
			n_a = interp.Resample(af.samples[0], ir[0].getSampleRate(), af.getSampleRate());
			af.samples.clear();
			af.samples.push_back(n_a);
			af.setSampleRate(ir[0].getSampleRate());
		}
		else {
			for(int i = 0; i < ir.size(); i++)
				{
				vector<double> n_a;
				n_a = interp.Resample(ir[i].samples[0], af.getSampleRate(), ir[i].getSampleRate());

				ir[i].samples.clear();
				ir[i].samples.push_back(n_a);
				ir[i].setSampleRate(af.getSampleRate());
				}
		}
	}

	cout << "\nReading in config file (Speaker configs)";
	// TODO: implement fstream for changing hardcoded speaker layout
	float rp[14][2] = {
	//	#include "../Config/renderpos.txt"
	};
	

	

	/* At this point you should have the following:
	- An IR
	- An audio file
	- both should be the same type
	- A list of speaker rendering positions
	- A list of headphone rendering positions with corresponding HRTF pairs
	*/
	cout << "\nConvolving room";
	vector<float> a(af.samples[0].begin(), af.samples[0].end());
	arma::fvec A(a);
		
	vector<float> w(ir[0].samples[0].begin(), ir[0].samples[0].end());
	vector<float> x(ir[1].samples[0].begin(), ir[1].samples[0].end());
	vector<float> y(ir[2].samples[0].begin(), ir[2].samples[0].end());
	vector<float> z(ir[3].samples[0].begin(), ir[3].samples[0].end());

	arma::fvec W(w);
	arma::fvec X(x); 
	arma::fvec Y(y); 
	arma::fvec Z(z);
	
	double norm = arma::max(W);
	auto yw = async(proc, A, W, norm);
	auto yx = async(proc, A, X, norm);
	auto yy = async(proc, A, Y, norm);
	auto yz = async(proc, A, Z, norm);
	
	arma::fvec wdone = yw.get();
	arma::fvec xdone = yx.get();
	arma::fvec ydone = yy.get();
	arma::fvec zdone = yz.get();

	vector<float> y_w = arma::conv_to<vector<float>>::from(wdone);
	vector<float> y_x = arma::conv_to<vector<float>>::from(xdone);
	vector<float> y_y = arma::conv_to<vector<float>>::from(ydone);
	vector<float> y_z = arma::conv_to<vector<float>>::from(zdone);

	//TODO: Decode to N speakers, output N channel file

	//TODO: Render to binaural and output stereofile
	
	system("pause");

    return 0;
}

