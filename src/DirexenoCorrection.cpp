
#include "DirexenoCorrection.h"
#include <fstream>
#include "TCanvas.h"
#include "TGraph.h"
#include "TFile.h"

// Check if caen fix is preformed.  look at WF from the processed file
using namespace std;

DirexenoCorrection::DirexenoCorrection(const std::string setup_file_name, int TimeSamples_){
  cout<<"hi"<<endl;

  cnpy::npz_t my_npz = cnpy::npz_load("./101122_BL.npz");
  cnpy::NpyArray arr_BLcap = my_npz["BLcap"];
  float *BLcapRaw = arr_BLcap.data<float>();

  cnpy::NpyArray arr_BLtrig = my_npz["BLtrig"];
  float *BLtrigRaw = arr_BLtrig.data<float>();

  cnpy::NpyArray trig_init_data = my_npz["trig_init"];
  trig_init = trig_init_data.data<int>();

  xxs=new double[1024];
  int TimeSamples =TimeSamples_;
  cout<<"Nsamples="<<TimeSamples<<endl;
  cout<<" trig = "<<trig_init[0]<<endl;
  trig_init_value = trig_init[0];
  cout<<trig_init_value<<endl;
 cout<<"hi"<<endl;

  for (int i=0; i<1024; i++) xxs[i]=i;
  cout<<"hi"<<endl;
  float **BLtrigs=new float*[length_BLtrig];
  float **BLcaps=new float*[length_BLcap];

  int j=0;
  for (int c=0; c<nchannels;c++){
    //cout<<"1/  tgtrapg "<<c<<endl;
    BLtrigs[c]=&BLtrigRaw[c * length_BLtrig];
    BLcaps[c]=&BLcapRaw[c * length_BLcap];
    //    cout<<"2/ tgtrapg "<<c<<endl;
    gcaps[c]=new TGraph(length_BLcap, xxs,(double*) &BLcapRaw[c * length_BLtrig]);
    gtrig[c]=new TGraph(length_BLtrig, xxs,(double*) &BLtrigRaw[c * length_BLtrig]);
    gcaps_shifted[c] = new TGraph();

    for (int i=0; i<length_BLcap ; i++ ){
      int index_template = i - trig_init_value;
      if (index_template<0) {index_template = index_template + 1024;}
      if (index_template>1023) {index_template = index_template - 1024;}
      if (c>0) gcaps_shifted[c]->SetPoint(i,i,gcaps[c-1]->GetY()[index_template]);
      if (c==0) gcaps_shifted[c]->SetPoint(i,i,gcaps[c]->GetY()[index_template]);

    }
  }

  //  draw_BLcap() ;
  // draw_BLtrig() ;
  cout<<"done \n";
}


void DirexenoCorrection::draw_BLcap() {
  TCanvas *c1 = new TCanvas("C1","c1",2000,2000);
  c1->Divide(5,5);
  for (int c = 0; c<nchannels; c++) {
    gcaps[c]->SetTitle(Form("Capacitor arrays baseline, channel %d, start bin %d",c+1,trig_init[0]));
    c1->cd(c+1);
    gPad->SetGrid();
  }
  c1->SaveAs("BL_cap.png");
}


double DirexenoCorrection::remove_baseline(float *wf, int N) {
  double base_line=0;
  for (int i=0; i<N; i++) {
    base_line+=wf[i];
  }
  base_line = base_line / N;
  //cout<<TimeSamples<<endl;
  for (int i=0; i<1024; i++) {
    // cout<<wf[i]<<"-"<<base_line<<"=";
    wf[i]=wf[i]-base_line;
    //    wf[i]= - wf[i];
    // cout<<wf[i]<<endl;
  }
  return base_line;
}



void DirexenoCorrection::fix_SCA_baseline(float *wf, int start_index,int channel) {
  if (channel==0)
    return;    // Channel 0 which is the trigger channel doesn't have calibration information in Gera's file

  if (channel>0) {
      TGraph g1;
      TGraph g2;
      TGraph g3;
      TGraph g4;
      g1.SetTitle("wf");
      g2.SetTitle("template");
      g3.SetTitle(Form("index_wf %d",start_index));
      g4.SetTitle(Form("index_template %d",trig_init_value));
      float wf2[1024];

      // Take the capcitor array baseline (SCA) and realign it with respect to the WF first capacitor. put it in gcaps_shifted.
      for (int i=0; i<length_BLcap ; i++ ){
	int index_template_wf = i +trig_init_value-start_index;
	if (index_template_wf<0) {index_template_wf = index_template_wf + 1024;}
	if (index_template_wf>1023) {index_template_wf = index_template_wf - 1024;}
	if (channel>0) gcaps_shifted[channel]->SetPoint(i,i,gcaps[channel-1]->GetY()[index_template_wf]);
	if (channel==0) gcaps_shifted[channel]->SetPoint(i,i,gcaps[channel]->GetY()[index_template_wf]);

      }
      g1.SetTitle(Form("WF start=%d",start_index));
      g2.SetTitle(Form("shifted template start=%d",trig_init_value));
      g4.SetTitle(Form("org template start=%d",trig_init_value));
     	for (int i=0; i<1000; i++) {
	  int offset =0;
	  g1.SetPoint(i,i,wf[i]);
	  g2.SetPoint(i,i, gcaps_shifted[channel]->GetY()[i]);
	  g4.SetPoint(i,i, gcaps[channel-1]->GetY()[i]);
	  wf[i] = wf[i] + gcaps_shifted[channel]->GetY()[i];
	  g3.SetPoint(i,i, wf[i]);

	}
	/*	TCanvas *c1=new TCanvas("c1","c1",600,600);
	g1.Draw("A*");
	g3.SetMarkerColor(4);
	//g3.Draw("*");
	gPad->Update();
	c1->SaveAs(Form("wf%d_b.png",channel));
	TCanvas *c2=new TCanvas("c2","c2",600,600);
	g2.SetMarkerColor(2);
	g2.Draw("A*");
	gPad->Update();
	c2->SaveAs(Form("wf%d_a.png",channel));
	TCanvas *c22=new TCanvas("c22","c22",600,600);
	g4.SetMarkerColor(3);
	g4.Draw("A*");
	gPad->Update();
	c22->SaveAs(Form("wf%d_aa.png",channel));
	TCanvas *c3=new TCanvas("c3","c3",600,600);
	g3.SetMarkerColor(4);
	g3.Draw("A*");
	gPad->Update();
	c3->SaveAs(Form("wf%d_c.png",channel));
	*/
  }

}

	/*	  int index_template = i - trig_init_value+offset;
	  int index_wf = i - start_index; // use from wf bins 2:1026  (based on Gera's code)

	  if (index_template<0) {index_template = index_template + 1024;}
	  if (index_template>1023) {index_template = index_template - 1024;}
	  if (index_wf<0) {index_wf = index_wf+ 1024;}
	  if (index_wf>1023) {index_wf = index_wf - 1024;}
	  //if (i==20) cout<<wf[index_wf+2]<<"= - "<< gcaps[channel-1]->GetY()[index_template]<<endl;
	  if (channel>0) {
	    wf[index_wf+2] = wf[index_wf+2] - gcaps[channel-1]->GetY()[index_template];

	    // This help return the SCA aligned waveform, another relevant block is below
	    //	    if (wf[index_wf+2]>1000 || fabs(wf[index_wf+2])>40) wf2[i+2]=0;
	    // else     wf2[i+2]= wf[index_wf+2] ;
	  }
	  if (index_wf>2) {
	    //	if (wf[index_wf]>1000) {cout<<"large bin="<<wf[index_wf]<<" "<<index_wf<<endl;}
	    g1.SetPoint(i,i,wf[index_wf]);
	    g2.SetPoint(i,i,gcaps[channel-1]->GetY()[index_template]);
	    g3.SetPoint(i,i,index_wf);
	    g4.SetPoint(i,i,index_template);
	    }*/
	  /* if (wf[index_wf]>1000)
	     cout<<" value bad"<<wf[index_wf]<<"gcaps="<<gcaps[channel-1].GetY()[index_template]<<"index="<<index_wf<<","<<index_template<<endl;
	     else
	     cout<<" value good"<<wf[index_wf]<<"gcaps="<<gcaps[channel-1].GetY()[index_template]<<"index="<<index_wf<<","<<index_template<<endl;

	  */    //cout<<wf[index_wf]<<endl;
//	}

	// This help return the SCA aligned waveform. another relevant block is above
	/*	if (channel>0) {
	  for (int i=0; i<1024; i++) {
	    wf[i+2]=wf2[i+2];
	  }
	  }*/
    /*TCanvas *c1=new TCanvas("c1","c1",1000,1000);
      TGraph *gg=new TGraph(1024,xxs,(double*)wf);
      c1->Divide(2,2);
      c1->cd(1);
      g1.Draw("A*");

      gPad->SetGrid();
      c1->cd(2);
      gcaps[channel-1]->Draw("A*");
      gPad->SetGrid();
      g2.Draw("A*");
      c1->cd(3);
      g3.Draw("A*");
      c1->cd(4);
      g4.Draw("A*");
      TFile* myFile = TFile::Open(Form("c%d.root",channel),"recreate");
	g1.Write("g1");
	g2.Write("g2");
	g3.Write("g3");
	g4.Write("g4");
	myFile->Close();
	c1->SaveAs(Form("c%d.png",channel));
    */

//  }

//}

void DirexenoCorrection::draw_BLtrig() {
  TCanvas *c1 = new TCanvas("C1","c1",2000,2000);
  c1->Divide(6,4);
  TGraph *g[nchannels];
  /*  for (int c = 0; c<nchannels; c++) {
    g[c]=new TGraph(1024, xxs, BLtrigs[c]);
    g[c]->SetTitle(Form("Trigger baseline, channel %d ",c+1));
    c1->cd(c+1);
    g[c]->Draw("Ap");
    gPad->SetGrid();
    }*/
  c1->SaveAs("BL_trigs.png");
}
void DirexenoCorrection::draw_wf(float *wf, int n) {
  TCanvas *c1 = new TCanvas("C1","c1",2000,2000);
  //  TGraph *g=new TGraph(1024, xxs, &wf[2]);
  //g->Draw("Al");
  gPad->SetGrid();

  c1->SaveAs(Form("wf%d.png",n));
}
