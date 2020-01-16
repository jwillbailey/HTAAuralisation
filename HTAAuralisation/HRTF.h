#pragma once
class HRTF {
public:
	HRTF(vector<vector<double>>& pos);
private:
	vector<vector<arma::vec>> kernels;
	void Process(arma::vec& x, double theta, double epsilon);
	void Load();
};

HRTF::HRTF(vector<vector<double>>& pos) {

	int _j = 0;
	vector<double> els;
	vector<fs::path> paths;

	for (const auto& entry : fs::directory_iterator("../HRTF/KEMAR/")) {
		//ir_path.push_back(entry.path());
		//AudioFile<double> a;
		//a.load(entry.path().string());
		//ir.push_back(a);
		els.push_back(stod(entry.path().string().substr(18)));
		paths.push_back(entry.path());
	}
}
void HRTF::Process(arma::vec& x, double theta, double epsilon) {
}
void HRTF::Load() {

}