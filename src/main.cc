#include "Plotter.h"
#include "Normalizer.h"
#include "Logfile.h"
#include "Style.h"
#include "tokenizer.hpp"


using namespace std;

void read_info(string, map<string, Normer*>&);


string output = "output.root";
string stylename = "default";

int main(int argc, char* argv[]) {
  if(argc < 2) {
    cerr << "No config file given: Exiting" << endl;
    exit(1);
  }

  bool ssqrtsb = true;

  map<string, Normer*> plots;
  Plotter fullPlot;
  for(int i = 1; i < argc; ++i) {
    if(argv[i][0] == '-') {
      if(strcmp(argv[i],"-help") == 0) {
	cout << "Usage: ./Plotter [OPTION] [CONFIG FILE]" << endl;
	cout << "Plotter allows for making stack plots with ratio or significance plots as a" << endl;
	cout << "secondary graph. The Default graph is the Ratio plot.  You can change this with" << endl;
	cout << "the different options" << endl << endl;
	cout << "    -sigleft      Significance plot cumulative from the left. Entry in bin i" << endl;
	cout << "                  represents siginifcance if events with values in bins greater" << endl;
	cout << "                  than i are cut" << endl;
	cout << "    -sigright     Significance plot cumulative from the right. Entry in bin i" << endl;
	cout << "                  represents significance if events with values in bins lower" << endl;
	cout << "                  than i are cut" << endl;
	cout << "    -sigbin       Significance plot with significance of each bin. Not" << endl;
	cout << "                  cumulative, just significance of the respective bin" << endl;
	cout << "    -ssqrtb       Default for Significance is using the formula:" << endl;
        cout << "                  s/sqrt(s + b)" << endl;
	cout << "                  This option changes the significance calculation to be:" << endl;
	cout << "                  s/sqrt(b)" << endl;
	exit(0);
      } else if( strcmp(argv[i], "-sigleft") == 0) fullPlot.setBottomType(SigLeft);
      else if( strcmp(argv[i], "-sigright") == 0) fullPlot.setBottomType(SigRight);
      else if( strcmp(argv[i],"-sigbin") == 0) fullPlot.setBottomType(SigBin);
      else if( strcmp(argv[i],"-ssqrtb") == 0) ssqrtsb = false;
      else {
	cout << "wrong option, exiting" << endl;
	exit(0);
      }
    } else read_info(argv[i], plots);
  }




  int totalfiles = 0;
  ///  vector<string> datan, bgn, sign;
  for(map<string, Normer*>::iterator it = plots.begin(); it != plots.end(); ++it) {
    fullPlot.addFile(*it->second);
  }

  cout << "Finished Normalization" << endl;
  // if(datan.size() ==  0 || fullPlot.FileList[0]->GetSize() == 0) {
  //   cout << "No Data given: exiting" << endl;
  //   exit(1);
  // }

  TFile* final = new TFile(output.c_str(), "RECREATE");
  Logfile logfile;
  logfile.setHeader(fullPlot.getFilenames("all"));

  Style stylez("style/" + stylename);
  fullPlot.setStyle(stylez);

  fullPlot.CreateStack(final, logfile);

  cout << "Finished making Stack Plot" << endl;

  logfile.setTrailer();
  final->Close();
}


void read_info(string filename, map<string, Normer*>& plots) {
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  ifstream info_file(filename);
  boost::char_separator<char> sep(", \t");

  if(!info_file) {
    std::cout << "could not open file " << filename <<std::endl;
    exit(1);
  }

  vector<string> stemp;
  string line;
  double lumi;

  while(getline(info_file, line)) {
    tokenizer tokens(line, sep);
    stemp.clear();
    for(tokenizer::iterator iter = tokens.begin();iter != tokens.end(); iter++) {
      if( ((*iter)[0] == '/' && (*iter)[0] == '/') || ((*iter)[0] == '#') ) break;
      stemp.push_back(*iter);

    }

    if(stemp.size() >= 2) {
      if(stemp[0].find("lumi") != string::npos) lumi = stod(stemp[1]);
      else if(stemp[0].find("output") != string::npos) output = stemp[1];
      else if(stemp[0].find("style") != string::npos) stylename = stemp[1];
      else if(plots.find(stemp[1]) == plots.end()) plots[stemp[1]] = new Normer(stemp);
      else plots[stemp[1]]->setValues(stemp);
    } 
  }
  info_file.close();

  for(map<string, Normer*>::iterator it = plots.begin(); it != plots.end(); it++) {
    it->second->setLumi(lumi);
  }
}
