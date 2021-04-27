#include <stdio.h>
#include <stdlib.h>

#include "JevpBuilder.h"
#include "DAQ_READER/daqReader.h"
#include <DAQ_READER/daq_dta.h>
#include <DAQ_STGC/daq_stgc.h>
#include "Jevp/StJevpPlot/RunStatus.h"

#include <TH1D.h>
#include <TH2F.h>

#include <math.h>
#include "fttBuilder.h"
#include <RTS/include/rtsLog.h>
#include <rtsSystems.h>

// This is the one PlotSet that is guarenteed to always exist
// It's main purpose is to provide the run information 
// To the server...
//
// It has no plots (currently)
//


ClassImp(fttBuilder);
	
void fttBuilder::initialize(int argc, char *argv[]) {
	
    // setup histograms in content
    char tmp[256];
	char tmp1[256];
	
	// VPD lo

	sprintf(tmp,"ADC");
	sprintf(tmp1,"ADC");
	contents.ADC = new TH1D(tmp,tmp1,400,-0.5,4095.5);
	contents.ADC->SetXTitle("ADC");
	contents.ADC->SetYTitle("counts");

    sprintf(tmp,"ADCZoom");
    sprintf(tmp1,"ADCZoom");
    contents.ADCZoom = new TH1D(tmp,tmp1,200,-0.5,200.5);
    contents.ADCZoom->SetXTitle("ADC");
    contents.ADCZoom->SetYTitle("counts");

	
	// Add root histograms to Plots
	int np = sizeof(contents) / sizeof(TH1 *);
	JevpPlot *plots[np];

	int n=0;
	plots[n] = new JevpPlot(contents.ADC);
    plots[++n] = new JevpPlot(contents.ADCZoom);




	// Add Plots to plot set...
	for(int i=0;i<=n;i++) {
		LOG(DBG, "Adding plot %d",i);
		contents.array[i]->GetXaxis()->SetLabelSize(0.055);
		contents.array[i]->GetYaxis()->SetLabelSize(0.045);
		addPlot(plots[i]);
	}
}
	
void fttBuilder::startrun(daqReader *rdr) {
	resetAllPlots();

	ReadConfig();
}

void fttBuilder::stoprun(daqReader *rdr) {
}

void fttBuilder::event(daqReader *rdr) {
	LOG(DBG, "event #%d",rdr->seq);

    bool do_print = true;
    daq_dta *dd ;
    dd = rdr->det("stgc")->get("altro") ;   

    bool altro_found = false;
    while(dd && dd->iterate()) {    
        altro_found = 1 ;

        if(do_print) {
            // there is NO RDO in the bank
            printf("STGC ALTRO: sec %d, ALTRO %2d(FEE%02d):%02d\n",dd->sec,dd->row,dd->row/2,dd->pad) ;

            for(u_int i=0;i<dd->ncontent;i++) {
                printf("    %3d %3d\n",dd->adc[i].tb,dd->adc[i].adc) ;
            }
        }
    }


    dd = rdr->det("stgc")->get("vmm") ;

    bool vmm_found = false;
    while(dd && dd->iterate()) {    
        vmm_found = 1 ;

        if(do_print) {
            // there is NO RDO in the bank
            printf("STGC VMM: sec %d, RDO %d\n",dd->sec,dd->rdo) ;

            struct stgc_vmm_t *vmm = (stgc_vmm_t *)dd->Void ;
            for(u_int i=0;i<dd->ncontent;i++) {
                u_char feb = vmm[i].feb_vmm >> 2 ;  // feb [0..5]
                u_char vm = vmm[i].feb_vmm & 3 ;    // VMM [0..3]

                printf("  FEB %d:%d, ch %02d: ADC %d, BCID %d\n",feb,vm,vmm[i].ch,
                       vmm[i].adc,vmm[i].bcid) ;

                contents.ADC->Fill( vmm[i].adc );
                contents.ADCZoom->Fill( vmm[i].adc );
            }
        }
    }


}

void fttBuilder::main(int argc, char *argv[])
{
	fttBuilder me;  
	me.Main(argc, argv);
}


vector<string> fttBuilder::readTokenVector( TString &str ){
	str.ToLower();
	TObjArray* Strings = str.Tokenize(" ");

	TIter iString( Strings );
	TObjString* os=0;

	vector<string> strs;
	while ( (os=(TObjString*)iString()) ){
		strs.push_back( os->GetString().Data() );
	}
	return strs;
}

vector<int> fttBuilder::readIntVector( TString &str, int start ){
	vector<string> tokens = readTokenVector( str );
	vector<int> itokens;
	if ( start >= tokens.size() )
		return itokens;
	for ( int i = start; i < tokens.size(); i++ ){
		itokens.push_back( atoi( tokens[i].c_str() ) );
	}
	return itokens;
}

void fttBuilder::ReadConfig(){
	TString buffer;
	char mConfigFile[256];
	sprintf(mConfigFile, "%s/ftt/%s",confdatadir,"FTT_Config.txt");
	ifstream filein(mConfigFile); 

	int count=0;
	vector<TString> cValues;
	if(filein){
		while(!filein.eof()){
			buffer.ReadLine(filein);
			if(buffer.BeginsWith("/")) continue;
			if(buffer.BeginsWith("#")) continue;
			cValues.push_back( buffer );
			float number=atof(buffer.Data());
			count++;
		}
	} else {
		LOG("====FTT====", "Can not open file: %s", mConfigFile);
	}


	// process the tokens
	for ( int i = 0; i < cValues.size(); i++ ){
		vector<string> tk = readTokenVector( cValues[ i ] );		
		if ( tk.size() <= 0  ) continue;
        //  EXAMPLES based on VPD below
		// if ( "maskedeast" == tk[0] )
		// 	maskedChannelsEast = readIntVector( cValues[ i ], 1 );
		// else if ( "maskedwest" == tk[0] )
		// 	maskedChannelsWest = readIntVector( cValues[ i ], 1 );
		// else if ( "pulseronoff" == tk[0] && tk.size() >= 2 ){
		// 	pulserSwitch = (bool) atoi( tk[1].c_str() );
		// }
		// else if ( "noisecorr" == tk[0] && tk.size() >= 2 ){
		// 	noiseCorr = (bool) atoi( tk[1].c_str() );
		// }
		// else if ( "pulsermeanseast" == tk[0] )
		// 	expected_pulser_means_east = readIntVector( cValues[ i ], 1 );
		// else if ( "pulsermeanswest" == tk[0] )
		// 	expected_pulser_means_west = readIntVector( cValues[ i ], 1 );
		// else if ( "refeast" == tk[0] && tk.size() >= 2 ) 
		// 	refChannelEast = atoi( tk[1].c_str() );
		// else if ( "refwest" == tk[0] && tk.size() >= 2 ) 
		// 	refChannelWest = atoi( tk[1].c_str() );
	}

	filein.close();
}