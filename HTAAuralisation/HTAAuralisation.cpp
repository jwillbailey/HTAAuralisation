// HTAAuralisation.cpp : Defines the entry point for the console application.
//
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

#include "../../AudioFile/AudioFile.h"
#include <armadillo>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <iostream>

#include <direct.h>
#include <string>
#include "DSPtools.h"
#include <future>
#include <fstream>
#include <math.h>
#include "SpkConfig.h"
#include "HRTF.h"

//#include "../../vst-intelligibility-meter/s3a_SiMeter_Plugin/Source/S3A_SiMeter/DSPtools.h"



arma::vec proc(arma::vec a, arma::vec b, double c) {
	arma::vec y = arma::conv(a, b);
	y = y / c;
	return y;
}

vector<double> ambDec(vector<double> p, vector<double> &w, vector<double>& x, vector<double>& y, vector<double>& z) {
	vector<double> s;
	double norm = arma::max(arma::vec(w));
	for (int i = 0; i < w.size(); i++) {
		s.push_back(((w[i] + p[0] * x[i] + p[1] * y[i] + p[1] * z[i])/norm)*0.5);
	}
	return s;
}

int main()
{
	
	
	cout << "Welcome to the auralisation app.\n";
	SpkConfig spk;
	int spkN = spk.GetLabels().size();
	cout << "\n" << spk.GetLayoutName() << " selected\n" << std::endl;
	vector<vector<double>> pos = spk.GetPolarPositions();
	//HRTF h(pos);
	cout << "Loading B-format IR from ./Impulse Response\n";
	// Load in 4 channel .WAV Impulse response
	int _j = 0;
	cout << "IR Loading:\n";
	vector<fs::path> ir_paths;
	
	for (const auto& entry : fs::directory_iterator("../IR/")) {
		ir_paths.push_back(entry.path());
		AudioFile<double> a;
		std::cout << _j << ": " << entry.path().string() << std::endl;
		_j++;
	}
	int irs;
	cout << "Select Impulse Response: ";
	cin >> irs;
	cout << "\n";
	vector<fs::path> ir_path;
	vector <AudioFile<double>> ir;
	_j = 0;
	for (const auto& entry : fs::directory_iterator(ir_paths[irs].string())) {
		ir_path.push_back(entry.path());
		AudioFile<double> a;
		a.load(entry.path().string());
		ir.push_back(a);
		std::cout << _j << ": " << entry.path().string() << std::endl;
		_j++;
	}
	cout << "\nLoading audio file from ./Audio \n";
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

	
	
	// TODO: implement fstream for changing hardcoded speaker layout
	/* At this point you should have the following:
	- An IR
	- An audio file
	- both should be the same type
	- A list of speaker rendering positions
	- A list of headphone rendering positions with corresponding HRTF pairs
	*/
	cout << "\nConvolving room";
	vector<double> a(af.samples[0].begin(), af.samples[0].end());
	arma::vec A(a);
	cout << ".";
	vector<double> w(ir[0].samples[0].begin(), ir[0].samples[0].end());
	cout << "."; 
	vector<double> x(ir[1].samples[0].begin(), ir[1].samples[0].end());
	cout << "."; 
	vector<double> y(ir[2].samples[0].begin(), ir[2].samples[0].end());
	cout << "."; 
	vector<double> z(ir[3].samples[0].begin(), ir[3].samples[0].end());
	cout << ".";
	arma::vec W(w);
	cout << ".";
	arma::vec X(x); 
	cout << "."; 
	arma::vec Y(y);
	cout << ".";
	arma::vec Z(z);
	cout << ".";
	double norm = arma::max(W);
	auto yw = async(proc, A, W, norm);
	auto yx = async(proc, A, X, norm);
	auto yy = async(proc, A, Y, norm);
	auto yz = async(proc, A, Z, norm);
	
	arma::vec wdone = yw.get();
	cout << "."; 
	arma::vec xdone = yx.get();
	cout << ".";
	arma::vec ydone = yy.get();
	cout << "."; 
	arma::vec zdone = yz.get();
	cout << ".";
	vector<double> y_w = arma::conv_to<vector<double>>::from(wdone);
	cout << "."; 
	vector<double> y_x = arma::conv_to<vector<double>>::from(xdone);
	cout << "."; 
	vector<double> y_y = arma::conv_to<vector<double>>::from(ydone);
	cout << "."; 
	vector<double> y_z = arma::conv_to<vector<double>>::from(zdone);
	cout << ".";
	//TODO: Decode to N speakers, output N channel file
	cout << "\n\nRendering to " << spk.GetLayoutName() << "\n";
	vector<AudioFile<double>> output;
	struct tm ltm;
	time_t now = time(0);
	localtime_s(&ltm, &now);
	string yr = to_string(ltm.tm_year);
	string mth = to_string(ltm.tm_mon);
	string dy = to_string(ltm.tm_mday);
	string hr = to_string(ltm.tm_hour);
	string mn = to_string(ltm.tm_min);
	string pth = "../Output/" + yr + mth + dy;
	string spth = "../Output/" + yr + mth + dy + "/" + hr + mn;
	//got to this point 16-1-2020
	const char* op = pth.c_str();
	fs::create_directory(pth.c_str());
	fs::create_directory(spth.c_str());
	ofstream details(spth + "/OutputInfo.txt");
	if (details.is_open()) {
		details << "Original Audio: " << af_path[af_n].string() << "\n";
		details << "Impulse Response: " << ir_paths[irs] << "\n";
		details << "Speaker layout: " << spk.GetLayoutName() << "\n";
	}
	for (int i = 0; i < spkN; i++) {
		vector<vector<double>> pos = spk.GetCartesianPositions();
		vector<double> s = ambDec(pos[i], y_w, y_x, y_y, y_z);
		AudioFile<double> a_n;
		a_n.samples.clear();
		a_n.samples.push_back(s);
		a_n.setSampleRate(af.getSampleRate());
		a_n.save(spth +"/"+ to_string(i) + "_audio.wav", AudioFileFormat::Wave);
		output.push_back(a_n);
		cout << ".";
	}

	

	//TODO: Render to binaural and output stereofile
	
	system("pause");

    return 0;
}

