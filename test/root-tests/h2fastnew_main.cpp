#include <stdlib.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TDavixFile.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TRandom.h"
#include "TTree.h"
#include "TTreeCache.h"
#include "TTreePerfStats.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
#include "TEnv.h"

#include <iostream>
#include <fstream>
#include <sstream>




void h2fast(const char *url , Bool_t draw=kFALSE, Long64_t cachesize=10000000, Int_t learn=1) {
// gEnv->SetValue("TFile.DavixLog", 10);
//  gDebug= 0x02;
   TStopwatch sw;
   TTree* T = NULL;
   sw.Start();
   Long64_t oldb = TFile::GetFileBytesRead();
   TFile *f = TFile::Open(url);

   if (!f || f->IsZombie()) {
      printf("File h1big.root does not exist\n");
      exit (-1);
   }


//   TTreeCacheUnzip::SetParallelUnzip(TTreeCacheUnzip::kEnable);

   T= (TTree*)f->Get("h42");
   Long64_t nentries = T->GetEntries();
   T->SetCacheSize(cachesize);
   TTreeCache::SetLearnEntries(learn);
   TFileCacheRead *tpf = f->GetCacheRead();
   //tpf->SetEntryRange(0,nentries);

   if (draw) T->Draw("rawtr","E33>20");
   else {
      TBranch *brawtr = T->GetBranch("rawtr");
      TBranch *bE33   = T->GetBranch("E33");
      Float_t E33;
      bE33->SetAddress(&E33);
      for (Long64_t i=0;i<nentries;i++) {
         T->LoadTree(i);
         bE33->GetEntry(i);
         if (E33 > 0) brawtr->GetEntry(i);
      }
   }
   if (tpf) tpf->Print();
   printf("Bytes read = %lld\n",TFile::GetFileBytesRead()-oldb);
   printf("Real Time = %7.3f s, CPUtime = %7.3f s\n",sw.RealTime(),sw.CpuTime());
   delete T;
   delete f;
}


int main(int argc, char** argv){
	if(argc <2 ){
		std::cout << "Usage: " << (char*) argv[0] << " [remote_url] " << std::endl;
		return -1;
	}

	h2fast(argv[1]);
	return 0;
}
