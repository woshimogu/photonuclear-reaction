//
// Created by mogu on 7/6/21.
//

#ifndef B1_B1RUN_HH
#define B1_B1RUN_HH

#include "G4Run.hh"
#include "G4VProcess.hh"
#include "globals.hh"
#include <map>

class B1DetectorConstruction;
class G4ParticleDefinition;

class B1Run: public G4Run{
public:
    B1Run(B1DetectorConstruction*);
    ~B1Run();

public:
    void CountProcesses(G4VProcess* process);

    void CountNuclearChannel(G4String, G4double);
    void ParticleCount(G4String, G4double);
    void CountGF();

    virtual void Merge(const G4Run*);

    void EndOfRun(G4bool);

private:
    struct ParticleData {
        ParticleData()
                : fCount(0), fEmean(0.), fEmin(0.), fEmax(0.) {}
        ParticleData(G4int count, G4double ekin, G4double emin, G4double emax)
                : fCount(count), fEmean(ekin), fEmin(emin), fEmax(emax) {}
        G4int     fCount;
        G4double  fEmean;
        G4double  fEmin;
        G4double  fEmax;
    };

    struct NuclChannel {
        NuclChannel()
                : fCount(0), fQ(0.) {}
        NuclChannel(G4int count, G4double Q)
                : fCount(count), fQ(Q) {}
        G4int     fCount;
        G4double  fQ;
    };

private:
    B1DetectorConstruction* fDetector;
    G4ParticleDefinition* fParticle;
    G4double              fEkin;

    std::map<G4String,G4int> fProcCounter;

    G4int fGFcount;      // U238 (gamma,fission) count

    std::map<G4String,NuclChannel>  fNuclChannelMap;
    std::map<G4String,ParticleData> fParticleDataMap;

};

#endif //B1_B1RUN_HH
