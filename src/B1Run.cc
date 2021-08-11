//
// Created by mogu on 7/6/21.
//

#include "B1Run.hh"
#include "B1DetectorConstruction.hh"
#include "B1PrimaryGeneratorAction.hh"

#include "G4ProcessTable.hh"
#include "G4HadronicProcessStore.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

B1Run::B1Run(B1DetectorConstruction* det)
        : G4Run(),
          fDetector(det), fParticle(0), fEkin(0.),
          fGFcount(0)
{

}

B1Run::~B1Run()
{ }


/*统计过程数*/
void B1Run::CountProcesses(G4VProcess* process)
{
    G4String procName = process->GetProcessName();
    std::map<G4String,G4int>::iterator it = fProcCounter.find(procName);
    if ( it == fProcCounter.end()) {
        fProcCounter[procName] = 1;
    }
    else {
        fProcCounter[procName]++;
    }
}

void B1Run::CountGF()
{
    fGFcount++;
}

/*统计反应道数*/
void B1Run::CountNuclearChannel(G4String name, G4double Q)
{
    std::map<G4String, NuclChannel>::iterator it = fNuclChannelMap.find(name);
    if ( it == fNuclChannelMap.end()) {
        fNuclChannelMap[name] = NuclChannel(1, Q);
    }
    else {
        NuclChannel& data = it->second;
        data.fCount++;
        data.fQ += Q;
    }
}

void B1Run::ParticleCount(G4String name, G4double Ekin)
{
    std::map<G4String, ParticleData>::iterator it = fParticleDataMap.find(name);
    if ( it == fParticleDataMap.end()) {
        fParticleDataMap[name] = ParticleData(1, Ekin, Ekin, Ekin);
    }
    else {
        ParticleData& data = it->second;
        data.fCount++;
        data.fEmean += Ekin;
        //update min max
        G4double emin = data.fEmin;
        if (Ekin < emin) data.fEmin = Ekin;
        G4double emax = data.fEmax;
        if (Ekin > emax) data.fEmax = Ekin;
    }
}

void B1Run::Merge(const G4Run* run)
{
    const B1Run* localRun = static_cast<const B1Run*>(run);

    fGFcount   += localRun->fGFcount;

    //map: processes count
    std::map<G4String,G4int>::const_iterator itp;
    for ( itp = localRun->fProcCounter.begin();
          itp != localRun->fProcCounter.end(); ++itp ) {

        G4String procName = itp->first;
        G4int localCount = itp->second;
        if ( fProcCounter.find(procName) == fProcCounter.end()) {
            fProcCounter[procName] = localCount;
        }
        else {
            fProcCounter[procName] += localCount;
        }
    }


    //map: nuclear channels
    std::map<G4String,NuclChannel>::const_iterator itc;
    for (itc = localRun->fNuclChannelMap.begin();
         itc != localRun->fNuclChannelMap.end(); ++itc) {

        G4String name = itc->first;
        const NuclChannel& localData = itc->second;
        if ( fNuclChannelMap.find(name) == fNuclChannelMap.end()) {
            fNuclChannelMap[name]
                    = NuclChannel(localData.fCount, localData.fQ);
        }
        else {
            NuclChannel& data = fNuclChannelMap[name];
            data.fCount += localData.fCount;
            data.fQ     += localData.fQ;
        }
    }

    //map: particles count
    std::map<G4String,ParticleData>::const_iterator itn;
    for (itn = localRun->fParticleDataMap.begin();
         itn != localRun->fParticleDataMap.end(); ++itn) {

        G4String name = itn->first;
        const ParticleData& localData = itn->second;
        if ( fParticleDataMap.find(name) == fParticleDataMap.end()) {
            fParticleDataMap[name]
                    = ParticleData(localData.fCount,
                                   localData.fEmean,
                                   localData.fEmin,
                                   localData.fEmax);
        }
        else {
            ParticleData& data = fParticleDataMap[name];
            data.fCount += localData.fCount;
            data.fEmean += localData.fEmean;
            G4double emin = localData.fEmin;
            if (emin < data.fEmin) data.fEmin = emin;
            G4double emax = localData.fEmax;
            if (emax > data.fEmax) data.fEmax = emax;
        }
    }

    G4Run::Merge(run);
}

void B1Run::EndOfRun(G4bool print)
{
    G4int prec = 5, wid = prec + 2;
    G4int dfprec = G4cout.precision(prec);

    if (numberOfEvent == 0) { G4cout.precision(dfprec);   return;}

    //frequency of processes
    //
    G4cout << "\n Process calls frequency:" << G4endl;
    G4int survive = 0;
    std::map<G4String,G4int>::iterator it;
    for (it = fProcCounter.begin(); it != fProcCounter.end(); it++) {
        G4String procName = it->first;
        G4int    count    = it->second;
        G4cout << "\t" << procName << "= " << count;
        if (procName == "Transportation") survive = count;
    }
    G4cout << G4endl;



    //nuclear channel count
    //
    G4cout << "\n List of nuclear reactions: \n" << G4endl;
    std::map<G4String,NuclChannel>::iterator ic;
    for (ic = fNuclChannelMap.begin(); ic != fNuclChannelMap.end(); ic++) {
        G4String name    = ic->first;
        NuclChannel data = ic->second;
        G4int count = data.fCount;
        G4double Q  = data.fQ/count;
        if (print)
            G4cout << "  " << std::setw(60) << name << ": " << std::setw(7) << count
                   << "   Q = " << std::setw(wid) << G4BestUnit(Q, "Energy")
                   << G4endl;
    }


    //particles count
    //
    G4cout << "\n List of generated particles:" << G4endl;

    std::map<G4String,ParticleData>::iterator itn;
    for (itn = fParticleDataMap.begin(); itn != fParticleDataMap.end(); itn++) {
        G4String name = itn->first;
        ParticleData data = itn->second;
        G4int count = data.fCount;
        G4double eMean = data.fEmean/count;
        G4double eMin = data.fEmin;
        G4double eMax = data.fEmax;
        if (print)
            G4cout << "  " << std::setw(13) << name << ": " << std::setw(7) << count
                   << "  Emean = " << std::setw(wid) << G4BestUnit(eMean, "Energy")
                   << "\t( "  << G4BestUnit(eMin, "Energy")
                   << " --> " << G4BestUnit(eMax, "Energy")
                   << ")" << G4endl;
    }


    // (gamma, fission) with U238 count
    G4cout << "\n Number of U238 (gamma,fission) reaction: \n" << G4endl;
    G4cout << "  "  << std::setw(60) << "U238 (g,f) reaction event counts" << ": "
           << std::setw(7) << fGFcount << G4endl;

    //remove all contents in fProcCounter, fCount
    fProcCounter.clear();
    fNuclChannelMap.clear();
    fParticleDataMap.clear();

    //restore default format
    G4cout.precision(dfprec);
}
